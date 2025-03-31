//
//
//

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <limits>
#include <mutex>
#include <regex>
#include <thread>

#include "half.h"

#include <tlCore/Image.h>
#include <tlCore/HDR.h>
#include <tlCore/StringFormat.h>

#include <FL/Fl_Vk_Window.H>
#include <FL/Fl_Vk_Utils.H>

#include <rapidxml/rapidxml.hpp>

#include <tlDevice/NDI/NDI.h>

#include "mrvHDR/mrvNDIView.h"

// Must come last due to X11 macros
#include <FL/platform.H>
#include <FL/Fl.H>

extern "C"
{
#include <libplacebo/dummy.h>
#include <libplacebo/shaders/colorspace.h>
#include <libplacebo/shaders.h>
}

namespace
{
    const char* kModule = "ndi_viewer";
} // namespace

namespace
{
    float apply_inverse_pq(float x)
    {
        float m1 = 0.8359375f;
        float m2 = 18.8515625f;
        float c1 = 0.8359375f;
        float c2 = 18.6875f;
        float c3 = 2.402f;

        if (x <= c1)
        {
            return x / m1;
        }
        else
        {
            return pow((x + c2) / c3, 1.0f / m2);
        }
    }

    // Function to unescape &quot; back to normal quotes (")
    std::string unescape_quotes_from_xml(const std::string& xml_escaped_str)
    {
        std::string json_str;
        size_t pos = 0;
        while (pos < xml_escaped_str.size())
        {
            if (xml_escaped_str.compare(pos, 6, "&quot;") == 0)
            {
                json_str += '"'; // Replace &quot; with "
                pos += 6;        // Skip over &quot;
            }
            else
            {
                json_str += xml_escaped_str[pos];
                pos++;
            }
        }
        return json_str;
    }
} // namespace

namespace mrv
{
    VkImage NDIView::createImage(
        VkImageType imageType, uint32_t width, uint32_t height, uint32_t depth,
        VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage)
    {

        VkImageCreateInfo imageInfo = {};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = imageType;
        imageInfo.format = format;
        imageInfo.extent.width = width;
        imageInfo.extent.height = height;
        imageInfo.extent.depth = depth;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.tiling = tiling;
        imageInfo.usage = usage;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VkImage image;
        if (vkCreateImage(m_device, &imageInfo, nullptr, &image) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create image");
        }
        return image;
    }

    VkDeviceMemory NDIView::allocateAndBindImageMemory(VkImage image)
    {
        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_device, image, &memRequirements);

        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_gpu, &memProperties);

        uint32_t memoryTypeIndex = 0;
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
        {
            if ((memRequirements.memoryTypeBits & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags &
                 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT))
            {
                memoryTypeIndex = i;
                break;
            }
        }

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = memoryTypeIndex;

        VkDeviceMemory imageMemory;
        if (vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMemory) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate image memory");
        }

        vkBindImageMemory(m_device, image, imageMemory, 0);
        return imageMemory;
    }

    VkCommandBuffer NDIView::beginSingleTimeCommands()
    {
        VkCommandBufferAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_cmd_pool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        return commandBuffer;
    }

    void NDIView::endSingleTimeCommands(VkCommandBuffer commandBuffer)
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_queue);

        vkFreeCommandBuffers(m_device, m_cmd_pool, 1, &commandBuffer);
    }

    //
    // \@todo: change for set_image_layout
    //
    void NDIView::transitionImageLayout(
        VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
    {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED &&
            newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (
            oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
            newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else
        {
            throw std::runtime_error("Unsupported layout transition");
        }

        vkCmdPipelineBarrier(
            commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0,
            nullptr, 1, &barrier);

        endSingleTimeCommands(commandBuffer);
    }

    void NDIView::createBuffer(
        VkDeviceSize size, VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties, VkBuffer& buffer,
        VkDeviceMemory& bufferMemory)
    {
        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create buffer");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);

        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_gpu, &memProperties);

        uint32_t memoryTypeIndex = 0;
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; ++i)
        {
            if ((memRequirements.memoryTypeBits & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) ==
                    properties)
            {
                memoryTypeIndex = i;
                break;
            }
        }

        VkMemoryAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = memoryTypeIndex;

        if (vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate buffer memory");
        }

        vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
    }

    void NDIView::uploadTextureData(
        VkImage image, uint32_t width, uint32_t height, uint32_t depth,
        VkFormat format, const void* data)
    {
        VkResult result;
        VkDeviceSize imageSize = width * height * depth * 4 *
                                 sizeof(float); // Assuming RGBA float data

        // Create staging buffer
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;

        createBuffer(
            imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            stagingBuffer, stagingBufferMemory);

        // Copy data to staging buffer
        void* mappedData;
        result = vkMapMemory(
            m_device, stagingBufferMemory, 0, imageSize, 0, &mappedData);
        VK_CHECK_RESULT(result);

        std::memcpy(mappedData, data, static_cast<size_t>(imageSize));
        vkUnmapMemory(m_device, stagingBufferMemory);

        // Copy staging buffer to image
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferImageCopy region = {};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;
        region.imageOffset = {0, 0, 0};
        region.imageExtent = {width, height, depth};

        vkCmdCopyBufferToImage(
            commandBuffer, stagingBuffer, image,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        endSingleTimeCommands(commandBuffer);

        // Clean up staging buffer
        vkDestroyBuffer(m_device, stagingBuffer, nullptr);
        vkFreeMemory(m_device, stagingBufferMemory, nullptr);
    }

    VkImageView NDIView::createImageView(
        VkImage image, VkFormat format, VkImageType imageType)
    {
        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType =
            (imageType == VK_IMAGE_TYPE_1D)   ? VK_IMAGE_VIEW_TYPE_1D
            : (imageType == VK_IMAGE_TYPE_2D) ? VK_IMAGE_VIEW_TYPE_2D
                                              : VK_IMAGE_VIEW_TYPE_3D;
        viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        if (vkCreateImageView(m_device, &viewInfo, nullptr, &imageView) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create image view");
        }
        return imageView;
    }

    VkSampler NDIView::createSampler()
    {
        VkSamplerCreateInfo samplerInfo = {};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = 1.0f;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 0.0f;
        samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        VkSampler sampler;
        if (vkCreateSampler(m_device, &samplerInfo, nullptr, &sampler) !=
            VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create sampler");
        }
        return sampler;
    }

    void NDIView::addGPUTextures(const pl_shader_res* res)
    {
        // vkDeviceWaitIdle(m_device);  // waits for all queue on the device

        // // Clean up existing resources
        // if (m_desc_layout != VK_NULL_HANDLE) {
        //     vkDestroyDescriptorSetLayout(m_device, m_desc_layout, nullptr);
        //     m_desc_layout = VK_NULL_HANDLE;
        // }
        // if (m_pipeline_layout != VK_NULL_HANDLE) {
        //     vkDestroyPipelineLayout(m_device, m_pipeline_layout, nullptr);
        //     m_pipeline_layout = VK_NULL_HANDLE;
        // }
        // if (m_pipeline != VK_NULL_HANDLE) {
        //     vkDestroyPipeline(m_device, m_pipeline, nullptr);
        //     m_pipeline = VK_NULL_HANDLE;
        // }
        // if (m_desc_pool != VK_NULL_HANDLE) {
        //     vkDestroyDescriptorPool(m_device, m_desc_pool, nullptr);
        //     m_desc_pool = VK_NULL_HANDLE;
        // }
        // if (m_desc_set != VK_NULL_HANDLE) {
        //     // Descriptor sets are freed with the pool, no need to
        //     // destroy individually
        //     m_desc_set = VK_NULL_HANDLE;
        // }
        // if (m_frag_shader_module != VK_NULL_HANDLE) {
        //     vkDestroyShaderModule(m_device, m_frag_shader_module, nullptr);
        //     m_frag_shader_module = VK_NULL_HANDLE;
        // }

        // // Remove any existing libplacebo textures, keep m_textures[0]
        // if (m_textures.size() > 1) {
        //     // vkQueueWaitIdle(m_queue); // Wait for completion

        //     for (size_t i = 1; i < m_textures.size(); ++i) {
        //         destroy_texture_image(m_textures[i]);
        //     }
        //     m_textures.resize(1); // Keep only main texture
        // }

        for (unsigned i = 0; i < res->num_descriptors; ++i)
        {
            const pl_shader_desc* sd = &res->descriptors[i];
            switch (sd->desc.type)
            {
            case PL_DESC_STORAGE_IMG:
                throw std::runtime_error("Unimplemented storage image");
            case PL_DESC_SAMPLED_TEX:
            {
                pl_tex tex = reinterpret_cast<pl_tex>(sd->binding.object);
                pl_fmt fmt = tex->params.format;

                int dims = pl_tex_params_dimension(tex->params);
                assert(dims >= 1 && dims <= 3);

                const char* samplerName = sd->desc.name;

                // Map libplacebo format to Vulkan format
                VkFormat imageFormat =
                    VK_FORMAT_R32G32B32A32_SFLOAT; // Default for 3D
                if (dims == 2)
                    imageFormat = VK_FORMAT_R32G32B32_SFLOAT;
                if (dims == 1)
                    imageFormat = VK_FORMAT_R32_SFLOAT;

                // Texture dimensions
                uint32_t width = tex->params.w;
                uint32_t height = (dims >= 2) ? tex->params.h : 1;
                uint32_t depth = (dims == 3) ? tex->params.d : 1;

                assert(width > 0);
                if (dims >= 2)
                    assert(height > 0);
                if (dims == 3)
                    assert(depth > 0);

                // Get texture data from libplacebo
                const void* values = pl_tex_dummy_data(tex);
                if (!values)
                {
                    throw std::runtime_error(
                        "Could not read pl_tex_dummy_data");
                }

                // Determine image type
                VkImageType imageType = (dims == 1)   ? VK_IMAGE_TYPE_1D
                                        : (dims == 2) ? VK_IMAGE_TYPE_2D
                                                      : VK_IMAGE_TYPE_3D;

                // Create Vulkan image
                VkImage image =
                    createImage(imageType, width, height, depth, imageFormat);

                // Allocate and bind memory for the image
                VkDeviceMemory imageMemory = allocateAndBindImageMemory(image);

                // Transition image layout to TRANSFER_DST_OPTIMAL
                transitionImageLayout(
                    image, VK_IMAGE_LAYOUT_UNDEFINED,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

                // Upload texture data
                uploadTextureData(
                    image, width, height, depth, imageFormat, values);

                // Transition image layout to SHADER_READ_ONLY_OPTIMAL
                transitionImageLayout(
                    image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

                // Create image view
                VkImageView imageView =
                    createImageView(image, imageFormat, imageType);

                // Create sampler (equivalent to GL_LINEAR)
                VkSampler sampler = createSampler();

                Fl_Vk_Texture texture(
                    imageType, imageFormat, image, imageView, sampler,
                    imageMemory, samplerName, width, height, depth);
                m_textures.push_back(texture);
                break;
            }
            default:
                throw std::runtime_error("Unknown texture type");
            }
        }
    }

    struct LibPlaceboData
    {
        LibPlaceboData();
        ~LibPlaceboData();

        pl_log log;
        pl_gpu gpu;
    };

    LibPlaceboData::LibPlaceboData()
    {
        log = pl_log_create(PL_API_VER, NULL);
        if (!log)
        {
            throw std::runtime_error("log creation failed");
        }

        struct pl_gpu_dummy_params gpu_dummy;
        memset(&gpu_dummy, 0, sizeof(pl_gpu_dummy_params));

        gpu_dummy.glsl.version = 450;
        gpu_dummy.glsl.gles = false;
        gpu_dummy.glsl.vulkan = false;
        gpu_dummy.glsl.compute = false;

        gpu_dummy.limits.callbacks = false;
        gpu_dummy.limits.thread_safe = true;
        /* pl_buf */
        gpu_dummy.limits.max_buf_size = SIZE_MAX;
        gpu_dummy.limits.max_ubo_size = SIZE_MAX;
        gpu_dummy.limits.max_ssbo_size = SIZE_MAX;
        gpu_dummy.limits.max_vbo_size = SIZE_MAX;
        gpu_dummy.limits.max_mapped_size = SIZE_MAX;
        gpu_dummy.limits.max_buffer_texels = UINT64_MAX;
        /* pl_tex */
        gpu_dummy.limits.max_tex_1d_dim = UINT32_MAX;
        gpu_dummy.limits.max_tex_2d_dim = UINT32_MAX;
        gpu_dummy.limits.max_tex_3d_dim = UINT32_MAX;
        gpu_dummy.limits.buf_transfer = true;
        gpu_dummy.limits.align_tex_xfer_pitch = 1;
        gpu_dummy.limits.align_tex_xfer_offset = 1;

        /* pl_pass */
        gpu_dummy.limits.max_variable_comps = SIZE_MAX;
        gpu_dummy.limits.max_constants = SIZE_MAX;
        gpu_dummy.limits.max_pushc_size = SIZE_MAX;
        gpu_dummy.limits.max_dispatch[0] = UINT32_MAX;
        gpu_dummy.limits.max_dispatch[1] = UINT32_MAX;
        gpu_dummy.limits.max_dispatch[2] = UINT32_MAX;
        gpu_dummy.limits.fragment_queues = 0;
        gpu_dummy.limits.compute_queues = 0;

        gpu = pl_gpu_dummy_create(log, &gpu_dummy);
        if (!gpu)
        {
            throw std::runtime_error("pl_gpu_dummy_create failed!");
        }
    }

    LibPlaceboData::~LibPlaceboData()
    {
        pl_gpu_dummy_destroy(&gpu);
        pl_log_destroy(&log);
    }

    struct NDIView::Private
    {
        NDIlib_find_instance_t NDI_find = nullptr;
        NDIlib_recv_instance_t NDI_recv = nullptr;

        std::vector<std::string> NDIsources;
        std::string currentNDISource;

        bool init = false;

        struct FindMutex
        {
            std::mutex mutex;
        };
        FindMutex findMutex;
        struct FindThread
        {
            std::condition_variable cv;
            std::thread thread;
            std::atomic<bool> running;
        };
        FindThread findThread;

        struct VideoMutex
        {
            std::mutex mutex;
        };
        VideoMutex videoMutex;
        struct VideoThread
        {
            std::chrono::steady_clock::time_point logTimer;
            std::condition_variable cv;
            std::thread thread;
            std::atomic<bool> running;
        };
        VideoThread videoThread;

        struct AudioMutex
        {
            std::mutex mutex;
        };
        AudioMutex audioMutex;
        struct AudioThread
        {
            std::chrono::steady_clock::time_point logTimer;
            std::condition_variable cv;
            std::thread thread;
            std::atomic<bool> running;
        };
        AudioThread audioThread;

        // Standard NDI attributes (we don't use this)
        std::string primariesName;
        std::string transferName;
        std::string matrixName;

        // Full mrv2 image data (we try to use this)
        bool hdrMonitorFound = false;
        bool hasHDR = false;
        image::HDRData hdrData;

        NDIlib_FourCC_video_type_e fourCC = NDIlib_FourCC_type_UYVY;

        // tlRender variables
        image::Info info;
        std::shared_ptr<image::Image> image;

        // LibPlacebo variables
        std::shared_ptr<LibPlaceboData> placeboData;
        std::string hdrColors;
        std::string hdrColorsDef;
    };

    NDIView::~NDIView()
    {
        TLRENDER_P();

        _exitThreads();

        p.findThread.running = false;
        if (p.findThread.thread.joinable())
            p.findThread.thread.join();

        vkDestroyShaderModule(m_device, m_frag_shader_module, NULL);
        vkDestroyShaderModule(m_device, m_vert_shader_module, NULL);
    }

    NDIView::NDIView(int x, int y, int w, int h, const char* l) :
        Fl_Vk_Window(x, y, w, h, l),
        _p(new Private)
    {
        _init();
    }

    NDIView::NDIView(int w, int h, const char* l) :
        Fl_Vk_Window(w, h, l),
        _p(new Private)
    {
        _init();
    }

    void NDIView::init_vk_swapchain()
    {
        TLRENDER_P();

        Fl_Vk_Window::init_vk_swapchain();

        VkResult result;
        uint32_t formatCount;
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(
            m_gpu, m_surface, &formatCount, NULL);
        VK_CHECK_RESULT(result);

        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(
            m_gpu, m_surface, &formatCount, formats.data());
        VK_CHECK_RESULT(result);

        // Look for HDR10 or HLG if present
        for (const auto& format : formats)
        {
            switch (format.colorSpace)
            {
            case VK_COLOR_SPACE_HDR10_ST2084_EXT:
            case VK_COLOR_SPACE_HDR10_HLG_EXT:
            case VK_COLOR_SPACE_DOLBYVISION_EXT:
                m_format = format.format;
                m_color_space = format.colorSpace;
                p.hdrMonitorFound = true;
                break;
            default:
                break;
            }

            if (p.hdrMonitorFound)
                break;
        }

        if (!p.hdrMonitorFound)
        {
            std::cerr << "No HDR monitor found or configured for SDR!"
                      << std::endl;
            bool foundLinear = false;
            for (const auto& format : formats)
            {
                // Prefer UNORM with SRGB_NONLINEAR (linear output intent)
                if (format.format == VK_FORMAT_B8G8R8A8_UNORM &&
                    format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
                {
                    m_format = VK_FORMAT_B8G8R8A8_UNORM;
                    m_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
                    foundLinear = true;
                    break;
                }
            }
            if (!foundLinear)
            {
                // Fallback to first supported format (usually works)
                m_format = formats[0].format;
                m_color_space = formats[0].colorSpace;
                std::cerr << "No ideal linear format found, using fallback: "
                          << m_format << ", " << m_color_space << std::endl;
            }
        }
        else
        {
            std::cerr << "HDR monitor found" << std::endl;
            return;
        }

        // Handle undefined format case
        if (formatCount == 1 && formats[0].format == VK_FORMAT_UNDEFINED)
        {
            m_format = VK_FORMAT_B8G8R8A8_UNORM;
            m_color_space = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        }
    }

    void NDIView::_copy(const uint8_t* video_frame)
    {
        TLRENDER_P();

        const auto& info = p.image->getInfo();
        const std::size_t w = p.info.size.w;
        const std::size_t h = p.info.size.h;
        uint8_t* const data = p.image->getData();

        if (p.fourCC == NDIlib_FourCC_type_PA16)
        {
            uint16_t* p_y = (uint16_t*)video_frame;
            const uint16_t* p_uv = p_y + w * h;
            const uint16_t* p_alpha = p_uv + w * h;
            half* rgba = (half*)data;

            // Determine BT.601 or BT.709 based on resolution
            bool useBT709 = (w >= 1280 && h >= 720);

            // Coefficients
            float Kr = useBT709 ? 0.2126f : 0.299f;
            float Kb = useBT709 ? 0.0722f : 0.114f;
            float Kg = 1.0f - Kr - Kb;

            for (int y = 0; y < h; ++y)
            {
                for (int x = 0; x < w; ++x)
                {
                    const int yw = y * w;
                    int index_y = yw + x;
                    int index_uv = yw + (x / 2) * 2;
                    int index_alpha = index_y;

                    // Extract Y, U, V, and Alpha
                    float Yf = p_y[index_y] / 65535.0f;
                    float Uf = (p_uv[index_uv] - 32768) / 65535.0f;
                    float Vf = (p_uv[index_uv + 1] - 32768) / 65535.0f;
                    float A = p_alpha[index_alpha] / 65535.0f;

                    float R, G, B;

                    float Y_linear;
                    if (p.hasHDR)
                        Y_linear = apply_inverse_pq(Yf);
                    else
                        Y_linear = Yf;

                    if (useBT709)
                    {
                        // BT.709 typical multipliers
                        // (Exact values differ slightly in various references)
                        R = Y_linear + 1.5748f * Vf;
                        G = Y_linear - 0.1873f * Uf - 0.4681f * Vf;
                        B = Y_linear + 1.8556f * Uf;
                    }
                    else
                    {
                        // BT.601
                        R = Y_linear + 1.402f * Vf;
                        G = Y_linear - 0.344f * Uf - 0.714f * Vf;
                        B = Y_linear + 1.772f * Uf;
                    }

                    // Clamp to [0, 1] to prevent oversaturation (not needed)
                    // R = std::max(0.0f, std::min(1.0f, R));
                    // G = std::max(0.0f, std::min(1.0f, G));
                    // B = std::max(0.0f, std::min(1.0f, B));

                    // Store as RGBA float
                    int rgba_index = index_y * 4;
                    rgba[rgba_index] = R;
                    rgba[rgba_index + 1] = G;
                    rgba[rgba_index + 2] = B;
                    rgba[rgba_index + 3] = A;
                }
            }
        }
    }

    void NDIView::prepare_main_texture()
    {
        TLRENDER_P();

        VkResult result;
        const VkFormat tex_format = VK_FORMAT_R16G16B16A16_SFLOAT;

        // Query if image supports texture format
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_gpu, tex_format, &props);

        m_textures.resize(1);

        if ((props.linearTilingFeatures &
             VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) &&
            !m_use_staging_buffer)
        {
            uint32_t tex_width = 1, tex_height = 1;
            if (p.image)
            {
                const auto& info = p.image->getInfo();
                tex_width = p.info.size.w;
                tex_height = p.info.size.h;
            }

            m_textures[0].width = tex_width;
            m_textures[0].height = tex_height;
            m_textures[0].image = createImage(
                VK_IMAGE_TYPE_2D, tex_width, tex_height, 1, tex_format,
                VK_IMAGE_TILING_LINEAR, VK_IMAGE_USAGE_SAMPLED_BIT);

            VkMemoryRequirements memRequirements;
            vkGetImageMemoryRequirements(
                m_device, m_textures[0].image, &memRequirements);

            VkMemoryAllocateInfo mem_alloc = {};
            mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
            mem_alloc.pNext = NULL;
            mem_alloc.allocationSize = 0;
            mem_alloc.memoryTypeIndex = 0;
            mem_alloc.allocationSize = memRequirements.size;

            memory_type_from_properties(
                memRequirements.memoryTypeBits,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                    VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                &mem_alloc.memoryTypeIndex);

            /* allocate memory */
            result = vkAllocateMemory(
                m_device, &mem_alloc, NULL, &m_textures[0].mem);
            VK_CHECK_RESULT(result);

            /* bind memory */
            result = vkBindImageMemory(
                m_device, m_textures[0].image, m_textures[0].mem, 0);
            VK_CHECK_RESULT(result);

            VkCommandBuffer cmd = beginSingleTimeCommands();

            // Initial transition to shader-readable layout
            set_image_layout(
                cmd, m_textures[0].image, VK_IMAGE_ASPECT_COLOR_BIT,
                VK_IMAGE_LAYOUT_UNDEFINED, // Initial layout
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                0,                          // No previous access
                VK_PIPELINE_STAGE_HOST_BIT, // Host stage
                VK_ACCESS_SHADER_READ_BIT,  // Shader read
                VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

            endSingleTimeCommands(cmd);
        }
        else if (
            props.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)
        {
            Fl::fatal("m_use_staging_buffer is unimplemented");
        }
        else
        {
            /* Can't support VK_FORMAT_B8G8R8A8_UNORM !? */
            Fl::fatal("No support for B8G8R8A8_UNORM as texture image format");
        }

        /* create sampler */
        m_textures[0].sampler = createSampler();

        /* create image view */

        VkImageViewCreateInfo viewInfo = {};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.pNext = NULL;
        viewInfo.image = VK_NULL_HANDLE;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = tex_format;
        viewInfo.components = {
            VK_COMPONENT_SWIZZLE_R,
            VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B,
            VK_COMPONENT_SWIZZLE_A,
        };
        viewInfo.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
        viewInfo.flags = 0;
        viewInfo.image = m_textures[0].image;

        result =
            vkCreateImageView(m_device, &viewInfo, NULL, &m_textures[0].view);
        VK_CHECK_RESULT(result);
    }

    void NDIView::prepare_vertices()
    {
        TLRENDER_P();

        // clang-format off
        struct Vertex
        {
            float x, y;
            float u, v;      // UV coordinates
        };

        std::vector<Vertex> vertices;
        const math::Size2i viewportSize = { pixel_w(), pixel_h() };
        image::Size renderSize = { pixel_w(), pixel_h() };
        
        if (p.image)
        {
            renderSize = p.image->getSize();
            renderSize.w *= renderSize.pixelAspectRatio;
        }

        float aspectRender = renderSize.w / static_cast<float>(renderSize.h);
        float aspectViewport = viewportSize.w / static_cast<float>(viewportSize.h);

        float scaleX = 1.0F;
        float scaleY = 1.0F;

        if (aspectRender < aspectViewport) {
            // Image is too wide, shrink X
            scaleX = aspectRender / aspectViewport;
        }
        else
        {
            // Image is too tall, shrink Y
            scaleY = aspectViewport / aspectRender;
        }
        
        vertices.push_back({-scaleX, -scaleY, 0.F, 0.F});
        vertices.push_back({ scaleX, -scaleY, 1.F, 0.F});
        vertices.push_back({ scaleX,  scaleY, 1.F, 1.F});
        vertices.push_back({-scaleX,  scaleY, 0.F, 1.F});
            
        VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();

        // clang-format on
        VkBufferCreateInfo buf_info = {};
        buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        buf_info.pNext = NULL;
        buf_info.size = buffer_size;
        buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        buf_info.flags = 0;

        VkMemoryAllocateInfo mem_alloc = {};
        mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mem_alloc.pNext = NULL;
        mem_alloc.allocationSize = 0;
        mem_alloc.memoryTypeIndex = 0;

        VkMemoryRequirements mem_reqs;
        VkResult result;
        bool pass;
        void* data;

        memset(&m_vertices, 0, sizeof(m_vertices));

        result = vkCreateBuffer(m_device, &buf_info, NULL, &m_vertices.buf);
        VK_CHECK_RESULT(result);

        vkGetBufferMemoryRequirements(m_device, m_vertices.buf, &mem_reqs);
        VK_CHECK_RESULT(result);

        mem_alloc.allocationSize = mem_reqs.size;
        pass = memory_type_from_properties(
            mem_reqs.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            &mem_alloc.memoryTypeIndex);

        result = vkAllocateMemory(m_device, &mem_alloc, NULL, &m_vertices.mem);
        VK_CHECK_RESULT(result);

        result = vkMapMemory(
            m_device, m_vertices.mem, 0, mem_alloc.allocationSize, 0, &data);
        VK_CHECK_RESULT(result);

        std::memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));

        vkUnmapMemory(m_device, m_vertices.mem);

        result =
            vkBindBufferMemory(m_device, m_vertices.buf, m_vertices.mem, 0);
        VK_CHECK_RESULT(result);

        m_vertices.vi.sType =
            VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        m_vertices.vi.pNext = NULL;
        m_vertices.vi.vertexBindingDescriptionCount = 1;
        m_vertices.vi.pVertexBindingDescriptions = m_vertices.vi_bindings;
        m_vertices.vi.vertexAttributeDescriptionCount = 2;
        m_vertices.vi.pVertexAttributeDescriptions = m_vertices.vi_attrs;

        m_vertices.vi_bindings[0].binding = 0;
        m_vertices.vi_bindings[0].stride = sizeof(vertices[0]);
        m_vertices.vi_bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        m_vertices.vi_attrs[0].binding = 0;
        m_vertices.vi_attrs[0].location = 0;
        m_vertices.vi_attrs[0].format = VK_FORMAT_R32G32_SFLOAT;
        m_vertices.vi_attrs[0].offset = 0;

        m_vertices.vi_attrs[1].binding = 0;
        m_vertices.vi_attrs[1].location = 1;
        m_vertices.vi_attrs[1].format = VK_FORMAT_R32G32_SFLOAT;
        m_vertices.vi_attrs[1].offset = sizeof(float) * 2;
    }

    // m_format, m_depth (optionally) -> creates m_renderPass
    void NDIView::prepare_render_pass()
    {
        bool has_depth = mode() & FL_DEPTH;
        bool has_stencil = mode() & FL_STENCIL;

        VkAttachmentDescription attachments[2];
        attachments[0] = VkAttachmentDescription();
        attachments[0].format = m_format;
        attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        attachments[1] = VkAttachmentDescription();

        VkAttachmentReference color_reference = {};
        color_reference.attachment = 0;
        color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkAttachmentReference depth_reference = {};
        depth_reference.attachment = 1;
        depth_reference.layout =
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass = {};
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.flags = 0;
        subpass.inputAttachmentCount = 0;
        subpass.pInputAttachments = NULL;
        subpass.colorAttachmentCount = 1;
        subpass.pColorAttachments = &color_reference;
        subpass.pResolveAttachments = NULL;

        if (has_depth || has_stencil)
        {
            attachments[1].format = m_depth.format;
            attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            if (has_stencil)
            {
                attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            }
            else
            {
                attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
            }
            attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[1].initialLayout =
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
            attachments[1].finalLayout =
                VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            subpass.pDepthStencilAttachment = &depth_reference;
            subpass.preserveAttachmentCount = 0;
            subpass.pPreserveAttachments = NULL;
        }

        VkRenderPassCreateInfo rp_info = {};
        rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        rp_info.pNext = NULL;
        rp_info.attachmentCount = (has_depth || has_stencil) ? 2 : 1;
        rp_info.pAttachments = attachments;
        rp_info.subpassCount = 1;
        rp_info.pSubpasses = &subpass;
        rp_info.dependencyCount = 0;
        rp_info.pDependencies = NULL;

        VkResult result;
        result = vkCreateRenderPass(m_device, &rp_info, NULL, &m_renderPass);
        VK_CHECK_RESULT(result);
    }

    VkShaderModule NDIView::prepare_vs()
    {
        if (m_vert_shader_module != VK_NULL_HANDLE)
            return m_vert_shader_module;

        std::string vertex_shader_glsl = R"(
 #version 450
 layout(location = 0) in vec2 inPos;
 layout(location = 1) in vec2 inTexCoord;
 layout(location = 0) out vec2 outTexCoord;

 void main() {
      gl_Position = vec4(inPos, 0.0, 1.0);
      outTexCoord = inTexCoord;
 }
 )";

        try
        {
            std::vector<uint32_t> spirv = compile_glsl_to_spirv(
                vertex_shader_glsl,
                shaderc_vertex_shader, // Shader type
                "vertex_shader.glsl"   // Filename for error reporting
            );

            m_vert_shader_module = create_shader_module(m_device, spirv);
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
            m_vert_shader_module = VK_NULL_HANDLE;
        }
        return m_vert_shader_module;
    }

    VkShaderModule NDIView::prepare_fs()
    {
        if (m_frag_shader_module != VK_NULL_HANDLE)
            return m_frag_shader_module;

        TLRENDER_P();

        // Example GLSL vertex shader
        std::string frag_shader_glsl = tl::string::Format(R"(
#version 450
#extension GL_KHR_shader_subgroup_basic : enable
#extension GL_KHR_shader_subgroup_vote : enable
#extension GL_KHR_shader_subgroup_arithmetic : enable
#extension GL_KHR_shader_subgroup_ballot : enable
#extension GL_KHR_shader_subgroup_shuffle : enable
#extension GL_KHR_shader_subgroup_clustered : enable
#extension GL_KHR_shader_subgroup_quad : enable
#extension GL_ARB_texture_gather : enable

// Input from vertex shader
layout(location = 0) in vec2 inTexCoord;
layout(location = 0) out vec4 outColor;
{0}

// Main image to display
layout(binding = 0) uniform sampler2D inTexture;

void main() {
     vec4 tmp = texture(inTexture, inTexCoord);
     {1}
}
)")
                                           .arg(p.hdrColorsDef)
                                           .arg(p.hdrColors);
        // Compile to SPIR-V
        try
        {
            std::cerr << frag_shader_glsl << std::endl;
            std::vector<uint32_t> spirv = compile_glsl_to_spirv(
                frag_shader_glsl,
                shaderc_fragment_shader, // Shader type
                "frag_shader.glsl"       // Filename for error reporting
            );
            m_frag_shader_module = create_shader_module(m_device, spirv);
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << std::endl;
            m_frag_shader_module = VK_NULL_HANDLE;
        }
        return m_frag_shader_module;
    }

    void NDIView::prepare_pipeline()
    {
        VkGraphicsPipelineCreateInfo pipeline;
        VkPipelineCacheCreateInfo pipelineCache;

        VkPipelineVertexInputStateCreateInfo vi;
        VkPipelineInputAssemblyStateCreateInfo ia;
        VkPipelineRasterizationStateCreateInfo rs;
        VkPipelineColorBlendStateCreateInfo cb;
        VkPipelineDepthStencilStateCreateInfo ds;
        VkPipelineViewportStateCreateInfo vp;
        VkPipelineMultisampleStateCreateInfo ms;
        VkDynamicState dynamicStateEnables[(
            VK_DYNAMIC_STATE_STENCIL_REFERENCE - VK_DYNAMIC_STATE_VIEWPORT +
            1)];
        VkPipelineDynamicStateCreateInfo dynamicState;

        VkResult result;

        memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
        memset(&dynamicState, 0, sizeof dynamicState);
        dynamicState.sType =
            VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamicState.pDynamicStates = dynamicStateEnables;

        memset(&pipeline, 0, sizeof(pipeline));
        pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipeline.layout = m_pipeline_layout;

        vi = m_vertices.vi;

        memset(&ia, 0, sizeof(ia));
        ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;

        memset(&rs, 0, sizeof(rs));
        rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rs.polygonMode = VK_POLYGON_MODE_FILL;
        rs.cullMode = VK_CULL_MODE_NONE;
        rs.frontFace = VK_FRONT_FACE_CLOCKWISE;
        rs.depthClampEnable = VK_FALSE;
        rs.rasterizerDiscardEnable = VK_FALSE;
        rs.depthBiasEnable = VK_FALSE;
        rs.lineWidth = 1.0f;

        memset(&cb, 0, sizeof(cb));
        cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        VkPipelineColorBlendAttachmentState att_state[1];
        memset(att_state, 0, sizeof(att_state));
        att_state[0].colorWriteMask = 0xf;
        att_state[0].blendEnable = VK_FALSE;
        cb.attachmentCount = 1;
        cb.pAttachments = att_state;

        memset(&vp, 0, sizeof(vp));
        vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        vp.viewportCount = 1;
        dynamicStateEnables[dynamicState.dynamicStateCount++] =
            VK_DYNAMIC_STATE_VIEWPORT;
        vp.scissorCount = 1;
        dynamicStateEnables[dynamicState.dynamicStateCount++] =
            VK_DYNAMIC_STATE_SCISSOR;

        bool has_depth = mode() & FL_DEPTH;
        bool has_stencil = mode() & FL_STENCIL;

        memset(&ds, 0, sizeof(ds));
        ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        ds.depthTestEnable = has_depth ? VK_TRUE : VK_FALSE;
        ds.depthWriteEnable = has_depth ? VK_TRUE : VK_FALSE;
        ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
        ds.depthBoundsTestEnable = VK_FALSE;
        ds.stencilTestEnable = has_stencil ? VK_TRUE : VK_FALSE;
        ds.back.failOp = VK_STENCIL_OP_KEEP;
        ds.back.passOp = VK_STENCIL_OP_KEEP;
        ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
        ds.front = ds.back;

        memset(&ms, 0, sizeof(ms));
        ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        ms.pSampleMask = NULL;
        ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        // Two stages: vs and fs
        pipeline.stageCount = 2;
        VkPipelineShaderStageCreateInfo shaderStages[2];
        memset(&shaderStages, 0, 2 * sizeof(VkPipelineShaderStageCreateInfo));

        shaderStages[0].sType =
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = prepare_vs();
        shaderStages[0].pName = "main";

        shaderStages[1].sType =
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = prepare_fs();
        shaderStages[1].pName = "main";

        pipeline.pVertexInputState = &vi;
        pipeline.pInputAssemblyState = &ia;
        pipeline.pRasterizationState = &rs;
        pipeline.pColorBlendState = &cb;
        pipeline.pMultisampleState = &ms;
        pipeline.pViewportState = &vp;
        pipeline.pDepthStencilState = &ds;
        pipeline.pStages = shaderStages;
        pipeline.renderPass = m_renderPass;
        pipeline.pDynamicState = &dynamicState;

        memset(&pipelineCache, 0, sizeof(pipelineCache));
        pipelineCache.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

        result = vkCreatePipelineCache(
            m_device, &pipelineCache, NULL, &m_pipelineCache);
        VK_CHECK_RESULT(result);
        result = vkCreateGraphicsPipelines(
            m_device, m_pipelineCache, 1, &pipeline, NULL, &m_pipeline);
        VK_CHECK_RESULT(result);

        vkDestroyPipelineCache(m_device, m_pipelineCache, NULL);
    }

    void NDIView::prepare_descriptor_layout()
    {
        TLRENDER_P();

        std::vector<VkDescriptorSetLayoutBinding> bindings;

        // Main texture at binding 0
        VkDescriptorSetLayoutBinding mainBinding = {};
        mainBinding.binding = 0;
        mainBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        mainBinding.descriptorCount = 1;
        mainBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        mainBinding.pImmutableSamplers = nullptr;
        bindings.push_back(mainBinding);

        // Additional libplacebo textures starting at binding 1
        if (p.hasHDR)
        {
            for (uint32_t i = 1; i < m_textures.size(); ++i)
            {
                VkDescriptorSetLayoutBinding binding = {};
                binding.binding =
                    i; // Matches libplacebo’s binding = i + 1 in GLSL
                binding.descriptorType =
                    VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
                binding.descriptorCount = 1;
                binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
                binding.pImmutableSamplers = nullptr;
                bindings.push_back(binding);
            }
        }

        VkDescriptorSetLayoutCreateInfo layoutInfo = {};
        layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
        layoutInfo.pBindings = bindings.data();

        VkResult result = vkCreateDescriptorSetLayout(
            m_device, &layoutInfo, nullptr, &m_desc_layout);
        VK_CHECK_RESULT(result);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
        pipelineLayoutInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &m_desc_layout;

        result = vkCreatePipelineLayout(
            m_device, &pipelineLayoutInfo, nullptr, &m_pipeline_layout);
        VK_CHECK_RESULT(result);
    }

    void NDIView::prepare_descriptor_pool()
    {
        VkDescriptorPoolSize poolSize = {};
        poolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        poolSize.descriptorCount =
            static_cast<uint32_t>(m_textures.size()); // One per texture

        VkDescriptorPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.maxSets = 1;
        poolInfo.poolSizeCount = 1;
        poolInfo.pPoolSizes = &poolSize;

        VkResult result =
            vkCreateDescriptorPool(m_device, &poolInfo, nullptr, &m_desc_pool);
        VK_CHECK_RESULT(result);
    }

    void NDIView::prepare_descriptor_set()
    {
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_desc_pool;
        allocInfo.descriptorSetCount = 1;
        allocInfo.pSetLayouts = &m_desc_layout;

        VkResult result =
            vkAllocateDescriptorSets(m_device, &allocInfo, &m_desc_set);
        VK_CHECK_RESULT(result);

        std::vector<VkDescriptorImageInfo> imageInfos(m_textures.size());
        std::vector<VkWriteDescriptorSet> writes(m_textures.size());
        for (uint32_t i = 0; i < m_textures.size(); ++i)
        {
            imageInfos[i].sampler = m_textures[i].sampler;
            imageInfos[i].imageView = m_textures[i].view;
            imageInfos[i].imageLayout =
                VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

            writes[i].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writes[i].dstSet = m_desc_set;
            writes[i].dstBinding = i; // 0 for main, 1+ for libplacebo
            writes[i].descriptorCount = 1;
            writes[i].descriptorType =
                VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            writes[i].pImageInfo = &imageInfos[i];
        }

        vkUpdateDescriptorSets(
            m_device, static_cast<uint32_t>(writes.size()), writes.data(), 0,
            nullptr);
    }

    void NDIView::_exitThreads()
    {
        TLRENDER_P();

        p.videoThread.running = false;
        if (p.videoThread.thread.joinable())
            p.videoThread.thread.join();

        p.audioThread.running = false;
        if (p.audioThread.thread.joinable())
            p.audioThread.thread.join();
    }

    void NDIView::_init()
    {
        TLRENDER_P();

        // Some Vulkan settings
#ifdef NDEBUG
        m_validate = false;
#else
        m_validate = true;
#endif
        m_use_staging_buffer = false;
        m_clearColor = {0.F, 0.F, 0.F, 0.F};

        mode(FL_RGB | FL_DOUBLE | FL_ALPHA);
        m_vert_shader_module = VK_NULL_HANDLE;
        m_frag_shader_module = VK_NULL_HANDLE;

        if (!NDIlib_initialize())
            throw std::runtime_error("Could not initialize NDI library");

        p.placeboData.reset(new LibPlaceboData);

        p.findThread.running = true;
        p.findThread.thread = std::thread(
            [this]
            {
                try
                {
                    _findThread();
                }
                catch (std::exception& e)
                {
                    std::cerr << e.what() << std::endl;
                }
            });
    }

    void NDIView::_startThreads()
    {
        TLRENDER_P();

        p.videoThread.running = true;
        p.videoThread.thread = std::thread([this] { _videoThread(); });
    }

    void NDIView::prepare()
    {
        TLRENDER_P();

        vkDeviceWaitIdle(m_device); // waits for all queue on the device

        {
            std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
            prepare_main_texture();
            prepare_shader();
        }
        prepare_vertices();
        prepare_descriptor_layout();
        prepare_render_pass();
        prepare_pipeline();
        prepare_descriptor_pool();
        prepare_descriptor_set();

        m_swapchain_needs_recreation = false;
    }

    void NDIView::update_texture()
    {
        TLRENDER_P();

        VkResult result;

        if (p.hdrMonitorFound && p.hasHDR)
        {
            // This will make the FLTK swapchain call vk->SetHDRMetadataEXT();
            const image::HDRData& data = p.hdrData;
            m_hdr_metadata.sType = VK_STRUCTURE_TYPE_HDR_METADATA_EXT;
            m_hdr_metadata.displayPrimaryRed = {
                data.primaries[image::HDRPrimaries::Red][0],
                data.primaries[image::HDRPrimaries::Red][1],
            };
            m_hdr_metadata.displayPrimaryGreen = {
                data.primaries[image::HDRPrimaries::Green][0],
                data.primaries[image::HDRPrimaries::Green][1],
            };
            m_hdr_metadata.displayPrimaryBlue = {
                data.primaries[image::HDRPrimaries::Blue][0],
                data.primaries[image::HDRPrimaries::Blue][1],
            };
            m_hdr_metadata.whitePoint = {
                data.primaries[image::HDRPrimaries::White][0],
                data.primaries[image::HDRPrimaries::White][1],
            };
            // Max display capability
            m_hdr_metadata.maxLuminance =
                data.displayMasteringLuminance.getMax();
            m_hdr_metadata.minLuminance =
                data.displayMasteringLuminance.getMin();
            m_hdr_metadata.maxContentLightLevel = data.maxCLL;
            m_hdr_metadata.maxFrameAverageLightLevel = data.maxFALL;
        }

        VkCommandBuffer update_cmd;
        VkCommandBufferAllocateInfo cmdAllocInfo = {};
        cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdAllocInfo.commandPool = m_cmd_pool;
        cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdAllocInfo.commandBufferCount = 1;
        vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &update_cmd);

        VkCommandBufferBeginInfo cmdBeginInfo = {};
        cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(update_cmd, &cmdBeginInfo);

        // Transition to GENERAL for CPU writes
        set_image_layout(
            update_cmd, m_textures[0].image, VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL,
            VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            VK_ACCESS_HOST_WRITE_BIT, VK_PIPELINE_STAGE_HOST_BIT);

        vkEndCommandBuffer(update_cmd);
        VkSubmitInfo submitInfo = {};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &update_cmd;
        vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_queue); // Synchronize before CPU write

        void* mappedData;
        result = vkMapMemory(
            m_device, m_textures[0].mem, 0, VK_WHOLE_SIZE, 0, &mappedData);
        VK_CHECK_RESULT(result);

        if (p.image)
        {
            const size_t dataSize =
                m_textures[0].width * m_textures[0].height * 4 * sizeof(half);
            const size_t byteCount = p.image->getDataByteCount();

            // Make sure the texture and the image have the same data size.
            // If not, recalculate the swapchain (call destroy_resources and
            // prepare).
            if (dataSize == byteCount)
            {
                std::memcpy(mappedData, p.image->getData(), dataSize);
            }
            else
            {
                m_swapchain_needs_recreation = true;
            }
        }

        vkUnmapMemory(m_device, m_textures[0].mem);

        // Reallocate command buffer for second transition
        vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &update_cmd);
        vkBeginCommandBuffer(update_cmd, &cmdBeginInfo);

        // Transition back to SHADER_READ_ONLY_OPTIMAL
        set_image_layout(
            update_cmd, m_textures[0].image, VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_GENERAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            VK_ACCESS_HOST_WRITE_BIT, VK_PIPELINE_STAGE_HOST_BIT,
            VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        vkEndCommandBuffer(update_cmd);
        submitInfo.pCommandBuffers = &update_cmd;
        vkQueueSubmit(m_queue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_queue); // Synchronize before rendering

        vkFreeCommandBuffers(m_device, m_cmd_pool, 1, &update_cmd);
    }

    void NDIView::vk_draw_begin()
    {
        // Change background color here
        m_clearColor = {0.0, 0.0, 0.0, 0.0};
        Fl_Vk_Window::vk_draw_begin();
    }

    void NDIView::draw()
    {
        TLRENDER_P();

        update_texture();

        // Draw the triangle
        VkDeviceSize offsets[1] = {0};
        vkCmdBindVertexBuffers(m_draw_cmd, 0, 1, &m_vertices.buf, offsets);

        vkCmdDraw(m_draw_cmd, 4, 1, 0, 0);

        Fl_Window::draw();
    }

    void NDIView::_videoThread()
    {
        TLRENDER_P();
        while (p.videoThread.running)
        {
            std::unique_lock<std::mutex> lock(p.findMutex.mutex);
            if (p.currentNDISource.empty())
                continue;
            // We now have at least one source, so we create a receiver to
            // look at it.

            NDIlib_recv_create_t NDI_recv_create_desc;
            NDI_recv_create_desc.p_ndi_recv_name = "mrv2 HDR Receiver";
            NDI_recv_create_desc.source_to_connect_to =
                p.currentNDISource.c_str();
            lock.unlock(); // Explicitly unlock before creating receiver
            NDI_recv_create_desc.color_format = NDIlib_recv_color_format_best;

            // Create the receiver
            p.NDI_recv = NDIlib_recv_create(&NDI_recv_create_desc);
            if (!p.NDI_recv)
                continue;

            // The descriptors
            NDIlib_video_frame_t video_frame;
            NDIlib_audio_frame_t audio_frame; // not used yet

            while (p.videoThread.running)
            {

                switch (NDIlib_recv_capture(
                    p.NDI_recv, &video_frame, nullptr, nullptr, 1000))
                {
                    // Video data
                case NDIlib_frame_type_video:
                {
                    bool init = false;

                    float pixelAspectRatio = 1.F;
                    if (video_frame.picture_aspect_ratio == 0.F)
                        pixelAspectRatio =
                            1.F / (video_frame.xres * video_frame.yres);

                    if (p.info.size.w != video_frame.xres ||
                        p.info.size.h != video_frame.yres ||
                        p.info.size.pixelAspectRatio != pixelAspectRatio)
                    {
                        init = true;
                    }

                    if (video_frame.p_metadata)
                    {
                        // Parsing XML metadata
                        rapidxml::xml_document<> doc;
                        doc.parse<0>((char*)video_frame.p_metadata);

                        // Get root node
                        rapidxml::xml_node<>* root =
                            doc.first_node("ndi_color_info");

                        // Get attributes
                        if (root)
                        {
                            rapidxml::xml_attribute<>* attr_transfer =
                                root->first_attribute("transfer");
                            rapidxml::xml_attribute<>* attr_matrix =
                                root->first_attribute("matrix");
                            rapidxml::xml_attribute<>* attr_primaries =
                                root->first_attribute("primaries");
                            rapidxml::xml_attribute<>* attr_mrv2 =
                                root->first_attribute("mrv2");

                            if (attr_transfer)
                                p.transferName = attr_transfer->value();
                            if (attr_matrix)
                                p.matrixName = attr_matrix->value();
                            if (attr_primaries)
                                p.primariesName = attr_primaries->value();

                            if (!p.hasHDR)
                                init = true;
                            p.hasHDR = true;

                            if (attr_mrv2)
                            {
                                const std::string& jsonString =
                                    unescape_quotes_from_xml(
                                        attr_mrv2->value());

                                const nlohmann::json& j =
                                    nlohmann::json::parse(jsonString);

                                const image::HDRData& hdrData =
                                    j.get<image::HDRData>();
                                if (p.hdrData != hdrData)
                                {
                                    // \@ todo:
                                    p.hdrData = hdrData;
                                    init = true;
                                }
                            }
                        }
                        else
                        {
                            if (p.hasHDR)
                                init = true;
                            p.hasHDR = false;
                        }

                        // Display color information
                        // fprintf(
                        //     stderr,
                        //     "Video metadata color info (transfer: %s, matrix:
                        //     "
                        //     "%s, primaries: %s)\n",
                        //     attr_transfer->value(), attr_matrix->value(),
                        //     attr_primaries->value());

                        // std::cerr << "primaries--------" << std::endl;
                        // std::cerr << hdrData.primaries[0] << " "
                        //           << hdrData.primaries[1] << " "
                        //           << hdrData.primaries[2] << " "
                        //           << hdrData.primaries[3] << std::endl;
                    }
                    else
                    {
                        if (p.hasHDR)
                            init = true;
                        p.hasHDR = false;
                    }

                    if (init)
                    {
                        std::unique_lock<std::mutex> lock(p.videoMutex.mutex);
                        p.info.size.w = video_frame.xres;
                        p.info.size.h = video_frame.yres;
                        p.info.size.pixelAspectRatio = pixelAspectRatio;
                        p.info.layout.mirror.y = true;
                        p.info.pixelType = image::PixelType::RGBA_F16;
                        p.info.videoLevels = image::VideoLevels::FullRange;
                        p.fourCC = video_frame.FourCC;
                        p.image = image::Image::create(p.info);
                        m_swapchain_needs_recreation = true;
                    }

                    if (video_frame.p_data)
                    {
                        _copy(video_frame.p_data);
                        redraw();
                    }

                    NDIlib_recv_free_video(p.NDI_recv, &video_frame);
                    break;

                    //     // Audio data
                    // case NDIlib_frame_type_audio:
                    //     printf("Audio data received (%d samples).\n",
                    //     audio_frame.no_samples);
                    //     NDIlib_recv_free_audio(p.NDI_recv, &audio_frame);
                    //     break;

                    // Everything else
                }
                default:
                    break;
                }
            }
        }

        // Destroy the receiver
        NDIlib_recv_destroy(p.NDI_recv);
    }

    void NDIView::_findThread()
    {
        TLRENDER_P();

        const NDIlib_source_t* sources = nullptr;

        p.NDI_find = NDIlib_find_create_v2();
        if (!p.NDI_find)
            throw std::runtime_error("could not create ndi find");

        while (p.findThread.running)
        {
            using namespace std::chrono;
            for (const auto startTime = high_resolution_clock::now();
                 high_resolution_clock::now() - startTime < seconds(3);)
            {
                // Wait up till 1 second to check for new sources to be added or
                // removed
                if (!NDIlib_find_wait_for_sources(
                        p.NDI_find, 1000 /* milliseconds */))
                {
                    break;
                }
            }

            uint32_t no_sources = 0;

            std::unique_lock lock(p.findMutex.mutex);
            p.NDIsources.clear();

            // Get the updated list of sources
            while (!no_sources)
            {
                sources =
                    NDIlib_find_get_current_sources(p.NDI_find, &no_sources);
            }

            for (int i = 0; i < no_sources; ++i)
            {
                p.NDIsources.push_back(sources[i].p_ndi_name);
            }

            if (!p.NDIsources.empty())
            {
                std::string oldSource = p.currentNDISource;
                p.currentNDISource = p.NDIsources[0];

                if (p.currentNDISource != oldSource)
                {
                    _exitThreads();
                    _startThreads();
                }
            }
        }
    }

    void NDIView::destroy_textures()
    {
        vkQueueWaitIdle(m_queue); // Wait for completion

        for (uint32_t i = 0; i < m_textures.size(); i++)
        {
            destroy_texture_image(m_textures[i]);
        }
        m_textures.clear();
    }

    void NDIView::destroy_resources()
    {
        vkDestroyShaderModule(m_device, m_frag_shader_module, NULL);
        m_frag_shader_module = VK_NULL_HANDLE;

        if (m_vertices.buf != VK_NULL_HANDLE)
        {
            vkDestroyBuffer(m_device, m_vertices.buf, NULL);
            m_vertices.buf = VK_NULL_HANDLE;
        }

        if (m_vertices.mem != VK_NULL_HANDLE)
        {
            vkFreeMemory(m_device, m_vertices.mem, NULL);
            m_vertices.mem = VK_NULL_HANDLE;
        }

        destroy_textures();

        Fl_Vk_Window::destroy_resources();
    }

    void NDIView::prepare_shader()
    {
        TLRENDER_P();

        pl_shader_params shader_params;
        memset(&shader_params, 0, sizeof(pl_shader_params));

        shader_params.id = 1;
        shader_params.gpu = p.placeboData->gpu;
        shader_params.dynamic_constants = false;

        pl_shader shader = pl_shader_alloc(p.placeboData->log, &shader_params);
        if (!shader)
        {
            throw std::runtime_error("pl_shader_alloc failed!");
        }

        pl_color_map_params cmap;
        memset(&cmap, 0, sizeof(pl_color_map_params));

        // defaults, generates LUTs if state is set.
        cmap.gamut_mapping = &pl_gamut_map_perceptual;

        // PL_GAMUT_MAP_CONSTANTS is defined in wrong order for C++
        cmap.gamut_constants = {0};
        cmap.gamut_constants.perceptual_deadzone = 0.3F;
        cmap.gamut_constants.perceptual_strength = 0.8F;
        cmap.gamut_constants.colorimetric_gamma = 1.80f;
        cmap.gamut_constants.softclip_knee = 0.70f;
        cmap.gamut_constants.softclip_desat = 0.35f;

        // Hable and ACES are best for HDR
        //   &pl_tone_map_hable;
        cmap.tone_mapping_function = nullptr;

        cmap.tone_constants = {0};
        cmap.tone_constants.knee_adaptation = 0.4f;
        cmap.tone_constants.knee_minimum = 0.1f;
        cmap.tone_constants.knee_maximum = 0.8f;
        cmap.tone_constants.knee_default = 0.4f;
        cmap.tone_constants.knee_offset = 1.0f;

        cmap.tone_constants.slope_tuning = 1.5f;
        cmap.tone_constants.slope_offset = 0.2f;

        cmap.tone_constants.spline_contrast = 0.5f;

        cmap.tone_constants.reinhard_contrast = 0.5f;
        cmap.tone_constants.linear_knee = 0.3f;

        cmap.tone_constants.exposure = 1.0f;

        cmap.contrast_smoothness = 3.5f;

        const image::HDRData& data = p.hdrData;

        pl_color_space src_colorspace;
        memset(&src_colorspace, 0, sizeof(pl_color_space));

        if (p.hasHDR)
        {
            src_colorspace.primaries = PL_COLOR_PRIM_BT_2020;
            src_colorspace.transfer = PL_COLOR_TRC_PQ;
            cmap.metadata = PL_HDR_METADATA_ANY;
            pl_hdr_metadata& hdr = src_colorspace.hdr;
            hdr.min_luma = data.displayMasteringLuminance.getMin();
            hdr.max_luma = data.displayMasteringLuminance.getMax();
            hdr.prim.red.x = data.primaries[image::HDRPrimaries::Red][0];
            hdr.prim.red.y = data.primaries[image::HDRPrimaries::Red][1];
            hdr.prim.green.x = data.primaries[image::HDRPrimaries::Green][0];
            hdr.prim.green.y = data.primaries[image::HDRPrimaries::Green][1];
            hdr.prim.blue.x = data.primaries[image::HDRPrimaries::Blue][0];
            hdr.prim.blue.y = data.primaries[image::HDRPrimaries::Blue][1];
            hdr.prim.white.x = data.primaries[image::HDRPrimaries::White][0];
            hdr.prim.white.y = data.primaries[image::HDRPrimaries::White][1];
            hdr.max_cll = data.maxCLL;
            hdr.max_fall = data.maxFALL;
            hdr.scene_max[0] = data.sceneMax[0];
            hdr.scene_max[1] = data.sceneMax[1];
            hdr.scene_max[2] = data.sceneMax[2];
            hdr.scene_avg = data.sceneAvg;
            hdr.ootf.target_luma = data.ootf.targetLuma;
            hdr.ootf.knee_x = data.ootf.kneeX;
            hdr.ootf.knee_y = data.ootf.kneeY;
            hdr.ootf.num_anchors = data.ootf.numAnchors;
            for (int i = 0; i < hdr.ootf.num_anchors; i++)
                hdr.ootf.anchors[i] = data.ootf.anchors[i];
        }
        else
        {
            src_colorspace.primaries = PL_COLOR_PRIM_BT_709;
            src_colorspace.transfer = PL_COLOR_TRC_SRGB;
        }

        pl_color_space_infer(&src_colorspace);

        pl_color_space dst_colorspace;
        memset(&dst_colorspace, 0, sizeof(pl_color_space));

        if (p.hdrMonitorFound)
        {
            cmap.lut3d_size[0] = 0;
            cmap.lut3d_size[1] = 0;
            cmap.lut3d_size[2] = 0;
            cmap.lut_size = 0;
            cmap.visualize_rect.x0 = 0;
            cmap.visualize_rect.y0 = 0;
            cmap.visualize_rect.x1 = 1;
            cmap.visualize_rect.y1 = 1;

            dst_colorspace.primaries = PL_COLOR_PRIM_BT_2020;
            dst_colorspace.transfer = PL_COLOR_TRC_PQ;
            if (m_color_space == VK_COLOR_SPACE_HDR10_HLG_EXT)
            {
                dst_colorspace.transfer = PL_COLOR_TRC_HLG;
            }
            else if (m_color_space == VK_COLOR_SPACE_DOLBYVISION_EXT)
            {
                // \@todo:  How to handle this? PL_COLOR_TRC_DOLBYVISION does
                //          not exist.
                // dst_colorspace.transfer = ???
                dst_colorspace.transfer = PL_COLOR_TRC_PQ;
            }

            if (p.hasHDR)
                cmap.tone_mapping_function = nullptr;
            else
            {
                // cmap.tone_mapping_function = &pl_tone_map_clip;  // works
                cmap.tone_mapping_function = &pl_tone_map_hable;
                // cmap.tone_mapping_function = &pl_tone_map_st2094_40;

                // \@bug: MoltenVK compile error
                // cmap.tone_mapping_function = &pl_tone_map_reinhard;

                cmap.lut3d_size[0] = 16;
                cmap.lut3d_size[1] = 16;
                cmap.lut3d_size[2] = 16;
                cmap.lut_size = 256;
                cmap.visualize_rect.x0 = 0;
                cmap.visualize_rect.y0 = 0;
                cmap.visualize_rect.x1 = 1;
                cmap.visualize_rect.y1 = 1;
                cmap.contrast_smoothness = 3.5f;
            }
        }
        else
        {
            cmap.lut3d_size[0] = 48;
            cmap.lut3d_size[1] = 32;
            cmap.lut3d_size[2] = 256;
            cmap.lut_size = 256;
            cmap.visualize_rect.x0 = 0;
            cmap.visualize_rect.y0 = 0;
            cmap.visualize_rect.x1 = 1;
            cmap.visualize_rect.y1 = 1;
            cmap.contrast_smoothness = 3.5f;

            dst_colorspace.primaries = PL_COLOR_PRIM_BT_709;
            dst_colorspace.transfer = PL_COLOR_TRC_SRGB;

            if (p.hasHDR)
                cmap.tone_mapping_function = &pl_tone_map_st2094_40;
            else
                cmap.tone_mapping_function = nullptr;
        }

        pl_color_space_infer(&dst_colorspace);

        pl_color_map_args color_map_args;
        memset(&color_map_args, 0, sizeof(pl_color_map_args));

        color_map_args.src = src_colorspace;
        color_map_args.dst = dst_colorspace;

        color_map_args.prelinearized = false;
        pl_shader_obj state = NULL;
        color_map_args.state = &state; // with NULL and tonemap_clip works

        pl_shader_color_map_ex(shader, &cmap, &color_map_args);

        const pl_shader_res* res = pl_shader_finalize(shader);
        if (!res)
        {
            pl_shader_free(&shader);
            throw std::runtime_error("pl_shader_finalize failed!");
        }

        std::stringstream s;

        // s << "#define textureLod(t, p, b) texture(t, p)" << std::endl
        //   << std::endl;

        // std::cerr << "num_vertex_attribs=" << res->num_vertex_attribs
        //           << std::endl
        //           << "num_descriptors=" << res->num_descriptors << std::endl
        //           << "num_variables=" << res->num_variables << std::endl
        //           << "num_constants=" << res->num_constants << std::endl;

        for (int i = 0; i < res->num_descriptors; i++)
        {
            const pl_shader_desc* sd = &res->descriptors[i];
            const pl_desc* desc = &sd->desc;
            switch (desc->type)
            {
            case PL_DESC_SAMPLED_TEX:
            case PL_DESC_STORAGE_IMG:
            {
                static const char* types[] = {
                    "sampler1D",
                    "sampler2D",
                    "sampler3D",
                };

                pl_desc_binding binding = sd->binding;
                pl_tex tex = (pl_tex)binding.object;
                int dims = pl_tex_params_dimension(tex->params);
                const char* type = types[dims - 1];

                char prefix = ' ';
                switch (tex->params.format->type)
                {
                case PL_FMT_UINT:
                    prefix = 'u';
                    break;
                case PL_FMT_SINT:
                    prefix = 'i';
                    break;
                case PL_FMT_FLOAT:
                case PL_FMT_UNORM:
                case PL_FMT_SNORM:
                default:
                    break;
                }

                s << "layout(binding=" << (i + 1) << ") uniform " << prefix
                  << type << " " << desc->name << ";" << std::endl;
                break;
            }
            case PL_DESC_BUF_UNIFORM:
                throw "buf uniform";
                break;
            case PL_DESC_BUF_STORAGE:
                throw "buf storage";
            case PL_DESC_BUF_TEXEL_UNIFORM:
                throw "buf texel uniform";
            case PL_DESC_BUF_TEXEL_STORAGE:
                throw "buf texel storage";
            case PL_DESC_INVALID:
            case PL_DESC_TYPE_COUNT:
                throw "invalid or count";
                break;
            }
        }

        s << "//" << std::endl
          << "// Variables" << std::endl
          << "//" << std::endl
          << std::endl;
        // \@todo: add uniform bindings
        // s << "layout(binding = 2) uniform UBO {\n";
        for (int i = 0; i < res->num_variables; ++i)
        {
            const struct pl_shader_var shader_var = res->variables[i];
            const struct pl_var var = shader_var.var;
            std::string glsl_type = pl_var_glsl_type_name(var);
            s << "const " << glsl_type << " " << var.name;
            if (!shader_var.data)
            {
                s << ";" << std::endl;
            }
            else
            {
                int dim_v = var.dim_v;
                int dim_m = var.dim_m;
                switch (var.type)
                {
                case PL_VAR_SINT:
                {
                    int* m = (int*)shader_var.data;
                    s << " = " << m[0] << ";" << std::endl;
                    break;
                }
                case PL_VAR_UINT:
                {
                    unsigned* m = (unsigned*)shader_var.data;
                    s << " = " << m[0] << ";" << std::endl;
                    break;
                }
                case PL_VAR_FLOAT:
                {
                    float* m = (float*)shader_var.data;
                    if (dim_m > 1 && dim_v > 1)
                    {
                        s << " = " << glsl_type << "(";
                        for (int c = 0; c < dim_v; ++c)
                        {
                            for (int r = 0; r < dim_m; ++r)
                            {
                                int index = c * dim_m + r;
                                s << m[index];

                                // Check if it's the last element
                                if (!(r == dim_m - 1 && c == dim_v - 1))
                                {
                                    s << ", ";
                                }
                            }
                        }
                        s << ");" << std::endl;
                    }
                    else if (dim_v > 1)
                    {
                        s << " = " << glsl_type << "(";
                        for (int c = 0; c < dim_v; ++c)
                        {
                            s << m[c];

                            // Check if it's the last element
                            if (!(c == dim_v - 1))
                            {
                                s << ", ";
                            }
                        }
                        s << ");" << std::endl;
                    }
                    else
                    {
                        s << " = " << m[0] << ";" << std::endl;
                    }
                    break;
                }
                default:
                    break;
                }
            }
        }

        // \@todo: add uniform bindings, instead of using constants
        // s << "};" << std::endl;
        s << std::endl
          << "//" << std::endl
          << "// Constants" << std::endl
          << "//" << std::endl
          << std::endl;
        for (int i = 0; i < res->num_constants; ++i)
        {
            s << "layout(constant_id=" << i << ") ";
            const struct pl_shader_const constant = res->constants[i];
            switch (constant.type)
            {
            case PL_VAR_SINT:
                s << "const int " << constant.name << " = "
                  << *(reinterpret_cast<const int*>(constant.data));
                break;
            case PL_VAR_UINT:
                s << "const uint " << constant.name << " = "
                  << *(reinterpret_cast<const unsigned*>(constant.data));
                break;
            case PL_VAR_FLOAT:
                s << "const float " << constant.name << " = "
                  << *(reinterpret_cast<const float*>(constant.data));
                break;
            default:
                break;
            }
            s << ";" << std::endl;
        }

        s << res->glsl << std::endl;
        p.hdrColorsDef = s.str();

        try
        {
            addGPUTextures(res);
        }
        catch (const std::exception& e)
        {
            throw e;
        }

        {
            p.hdrColors = "outColor = ";
            p.hdrColors += res->name;
            p.hdrColors += "(tmp);\n";
        }

        pl_shader_free(&shader);
    }

} // namespace mrv
