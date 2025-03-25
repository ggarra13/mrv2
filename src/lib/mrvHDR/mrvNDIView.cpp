//
// Tiny Vulkan demo program for the Fast Light Tool Kit (FLTK).
//
// Copyright 1998-2010 by Bill Spitzak and others.
//
// This library is free software. Distribution and use rights are outlined in
// the file "COPYING" which should have been included with this file.  If this
// file is missing or damaged, see the license at:
//
//     https://www.fltk.org/COPYING.php
//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
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

#if defined(TLRENDER_LIBPLACEBO)
extern "C"
{
#include <libplacebo/dummy.h>
#include <libplacebo/shaders/colorspace.h>
#include <libplacebo/shaders.h>
}
#endif


namespace
{
    const char* kModule = "ndi_viewer";
    const double kTimeout = 0.005;
} // namespace

namespace
{
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
#if defined(TLRENDER_LIBPLACEBO)
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
    
        gpu_dummy.glsl.version = 410;
        gpu_dummy.glsl.gles = false;
        gpu_dummy.glsl.vulkan = false;
        gpu_dummy.glsl.compute = false;

        gpu_dummy.limits.callbacks = false;
        gpu_dummy.limits.thread_safe = true;
        /* pl_buf */  
        gpu_dummy.limits.max_buf_size  = SIZE_MAX;  
        gpu_dummy.limits.max_ubo_size  = SIZE_MAX;   
        gpu_dummy.limits.max_ssbo_size = SIZE_MAX;  
        gpu_dummy.limits.max_vbo_size  = SIZE_MAX;
        gpu_dummy.limits.max_mapped_size    = SIZE_MAX;   
        gpu_dummy.limits.max_buffer_texels  = UINT64_MAX;  
        /* pl_tex */  
        gpu_dummy.limits.max_tex_1d_dim= UINT32_MAX;
        gpu_dummy.limits.max_tex_2d_dim= UINT32_MAX;
        gpu_dummy.limits.max_tex_3d_dim= UINT32_MAX;    
        gpu_dummy.limits.buf_transfer  = true;  
        gpu_dummy.limits.align_tex_xfer_pitch = 1; 
        gpu_dummy.limits.align_tex_xfer_offset = 1;
    
        /* pl_pass */ 
        gpu_dummy.limits.max_variable_comps = SIZE_MAX; 
        gpu_dummy.limits.max_constants = SIZE_MAX;
        gpu_dummy.limits.max_pushc_size= SIZE_MAX;   
        gpu_dummy.limits.max_dispatch[0]    = UINT32_MAX;   
        gpu_dummy.limits.max_dispatch[1]    = UINT32_MAX;   
        gpu_dummy.limits.max_dispatch[2]    = UINT32_MAX;
        gpu_dummy.limits.fragment_queues    = 0;   
        gpu_dummy.limits.compute_queues= 0;
    
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
#endif // TLRENDER_LIBPLACEBO

    
    struct NDIView::Private
    {
        NDIlib_find_instance_t NDI_find = nullptr;
        NDIlib_recv_instance_t NDI_recv = nullptr;

        std::vector<std::string> NDIsources;
        std::string currentNDISource;

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
        std::string    primariesName;
        std::string    transferName;
        std::string    matrixName;

        // Full mrv2 image data (we try to use this)
        bool hasHDR = false;
        image::HDRData hdrData;

        NDIlib_FourCC_video_type_e fourCC =	NDIlib_FourCC_type_UYVY;

        // tlRender variables
        image::Info   info;
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
        Fl_Vk_Window::init_vk_swapchain();

        VkResult result;
        uint32_t formatCount;
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpu,
                                                      m_surface,
                                                      &formatCount, NULL);
        VK_CHECK_RESULT(result);
  
        std::vector<VkSurfaceFormatKHR> formats(formatCount);
        result = vkGetPhysicalDeviceSurfaceFormatsKHR(m_gpu,
                                                      m_surface,
                                                      &formatCount,
                                                      formats.data());
        VK_CHECK_RESULT(result);

        bool hdrFound = false;
        // Look for HDR10 or HLG if present
        for (const auto& format : formats)
        {
            switch(format.colorSpace)
            {
            case VK_COLOR_SPACE_HDR10_ST2084_EXT:
            case VK_COLOR_SPACE_HDR10_HLG_EXT:
            case VK_COLOR_SPACE_DOLBYVISION_EXT:
                m_format = format.format;
                m_color_space = format.colorSpace;
                hdrFound = true;
                break;
            default:
                break;
            }

            if (hdrFound)
                break;
        }

        if (!hdrFound)
        {
            std::cerr << "No HDR supported!" << std::endl;
        }
        
        // If the format list includes just one entry of VK_FORMAT_UNDEFINED,
        // the surface has no prefresulted format.  Otherwise, at least one
        // supported format will be returned.
        if (formatCount == 1 && formats[0].format == VK_FORMAT_UNDEFINED) {
            m_format = VK_FORMAT_B8G8R8A8_UNORM;
        } else {
            m_format = formats[0].format;
        }
        m_color_space = formats[0].colorSpace;

    }

    void NDIView::start()
    {
        TLRENDER_P();
        
        p.image = image::Image::create(p.info);

        destroy_resources();

        vkDestroyShaderModule(m_device, m_frag_shader_module, NULL);
        m_frag_shader_module = VK_NULL_HANDLE;

        if (p.hasHDR)
        {
            _create_HDR_shader();
        }
        else
        {
            p.hdrColors.clear();
            p.hdrColorsDef.clear();
        }
        
        prepare();
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
            half* rgba = (half*) data;
                    
            // Determine BT.601 or BT.709 based on resolution
            bool useBT709 = (w >= 1280 && h >= 720);
                    
            // Coefficients
            float Kr = useBT709 ? 0.2126f : 0.299f;
            float Kb = useBT709 ? 0.0722f : 0.114f;
            float Kg = 1.0f - Kr - Kb;

            for (int y = 0; y < h; ++y) {
                for (int x = 0; x < w; ++x) {
                    const int yw = y * w;
                    int index_y = yw + x;
                    int index_uv = yw + (x / 2) * 2;
                    int index_alpha = index_y;

                    // Extract Y, U, V, and Alpha
                    float Yf = p_y[index_y] / 65535.0f;
                    float Uf = (p_uv[index_uv] - 32768) / 32768.0f;
                    float Vf = (p_uv[index_uv + 1] - 32768) / 32768.0f;
                    float A = p_alpha[index_alpha] / 65535.0f;

                    // YUV to RGB conversion
                    float R = Yf + 1.402f * Vf;
                    float G = Yf - 0.344f * Uf - 0.714f * Vf;
                    float B = Yf + 1.772f * Uf;

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

    void NDIView::prepare_texture_image(
        const float* tex_colors, Fl_Vk_Texture* tex_obj,
        VkImageTiling tiling, VkImageUsageFlags usage, VkFlags required_props)
    {
        TLRENDER_P();
        
        const VkFormat tex_format = VK_FORMAT_R16G16B16A16_SFLOAT;
        uint32_t tex_width = 1, tex_height = 1;
        if (p.image)
        {
            const auto& info = p.image->getInfo();
            tex_width = info.size.w;
            tex_height = info.size.h;
        }
        
        VkResult result;
        bool pass;

        tex_obj->tex_width = tex_width;
        tex_obj->tex_height = tex_height;

        VkImageCreateInfo image_create_info = {};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.pNext = NULL;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.format = tex_format;
        image_create_info.extent = {tex_width, tex_height, 1};
        image_create_info.mipLevels = 1;
        image_create_info.arrayLayers = 1;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.tiling = tiling;
        image_create_info.usage = usage;
        image_create_info.flags = 0;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        VkMemoryAllocateInfo mem_alloc = {};
        mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mem_alloc.pNext = NULL;
        mem_alloc.allocationSize = 0;
        mem_alloc.memoryTypeIndex = 0;

        result =
            vkCreateImage(m_device, &image_create_info, NULL, &tex_obj->image);
        VK_CHECK_RESULT(result);

        vkGetImageMemoryRequirements(m_device, tex_obj->image, &m_mem_reqs);

        mem_alloc.allocationSize = m_mem_reqs.size;
        pass = memory_type_from_properties(
            m_mem_reqs.memoryTypeBits, required_props,
            &mem_alloc.memoryTypeIndex);

        /* allocate memory */
        result = vkAllocateMemory(m_device, &mem_alloc, NULL, &tex_obj->mem);
        VK_CHECK_RESULT(result);

        /* bind memory */
        result = vkBindImageMemory(m_device, tex_obj->image, tex_obj->mem, 0);
        VK_CHECK_RESULT(result);

        if (required_props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
        {
            VkImageSubresource subres = {};
            subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            subres.mipLevel = 0;
            subres.arrayLayer = 0;
            VkSubresourceLayout layout;
            void* data;
            int32_t x, y;

            vkGetImageSubresourceLayout(
                m_device, tex_obj->image, &subres, &layout);

            result = vkMapMemory(
                m_device, tex_obj->mem, 0, mem_alloc.allocationSize, 0, &data);
            VK_CHECK_RESULT(result);

            vkUnmapMemory(m_device, tex_obj->mem);
        }

        VkCommandBuffer cmd;
        VkCommandBufferAllocateInfo cmdAllocInfo = {};
        cmdAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmdAllocInfo.commandPool = m_cmd_pool;
        cmdAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        cmdAllocInfo.commandBufferCount = 1;
        vkAllocateCommandBuffers(m_device, &cmdAllocInfo, &cmd);

        VkCommandBufferBeginInfo cmdBeginInfo = {};
        cmdBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        vkBeginCommandBuffer(cmd, &cmdBeginInfo);

        // Initial transition to shader-readable layout
        set_image_layout(
            cmd, tex_obj->image, VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_UNDEFINED, // Initial layout
            VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
            0,                          // No previous access
            VK_PIPELINE_STAGE_HOST_BIT, // Host stage
            VK_ACCESS_SHADER_READ_BIT,  // Shader read
            VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);

        // Submit the command buffer to apply the transition
        vkEndCommandBuffer(cmd);
        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &cmd;
        vkQueueSubmit(m_queue, 1, &submit_info, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_queue); // Wait for completion

        vkFreeCommandBuffers(m_device, m_cmd_pool, 1, &cmd);
    }

    void NDIView::prepare_textures()
    {
        VkResult result;
        const VkFormat tex_format = VK_FORMAT_R16G16B16A16_SFLOAT;
        const float tex_colors[DEMO_TEXTURE_COUNT][2 * 4 * sizeof(half)] = {
            {0.4F, 0.4F, 0.4F, 1.F, 0.6F, 0.6F, 0.6F, 0.1F}
        };

        // Query if image supports texture format
        VkFormatProperties props;
        vkGetPhysicalDeviceFormatProperties(m_gpu, tex_format, &props);

        for (int i = 0; i < DEMO_TEXTURE_COUNT; i++)
        {
            if ((props.linearTilingFeatures &
                 VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) &&
                !m_use_staging_buffer)
            {
                /* Device can texture using linear textures */
                prepare_texture_image(
                    tex_colors[i], &m_textures[i], VK_IMAGE_TILING_LINEAR,
                    VK_IMAGE_USAGE_SAMPLED_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
            }
            else if (
                props.optimalTilingFeatures &
                VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT)
            {
                Fl::fatal("m_use_staging_buffer is unimplemented");
            }
            else
            {
                /* Can't support VK_FORMAT_B8G8R8A8_UNORM !? */
                Fl::fatal(
                    "No support for B8G8R8A8_UNORM as texture image format");
            }

            VkSamplerCreateInfo sampler = {};
            sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
            sampler.pNext = NULL;
            sampler.magFilter = VK_FILTER_NEAREST;
            sampler.minFilter = VK_FILTER_NEAREST;
            sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
            sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
            sampler.mipLodBias = 0.0f;
            sampler.anisotropyEnable = VK_FALSE;
            sampler.maxAnisotropy = 1;
            sampler.compareOp = VK_COMPARE_OP_NEVER;
            sampler.minLod = 0.0f;
            sampler.maxLod = 0.0f;
            sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
            sampler.unnormalizedCoordinates = VK_FALSE;

            VkImageViewCreateInfo view = {};
            view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            view.pNext = NULL;
            view.image = VK_NULL_HANDLE;
            view.viewType = VK_IMAGE_VIEW_TYPE_2D;
            view.format = tex_format;
            view.components = {
                VK_COMPONENT_SWIZZLE_R,
                VK_COMPONENT_SWIZZLE_G,
                VK_COMPONENT_SWIZZLE_B,
                VK_COMPONENT_SWIZZLE_A,
            };
            view.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
            view.flags = 0;

            /* create sampler */
            result = vkCreateSampler(
                m_device, &sampler, NULL, &m_textures[i].sampler);
            VK_CHECK_RESULT(result);

            /* create image view */
            view.image = m_textures[i].image;
            result =
                vkCreateImageView(m_device, &view, NULL, &m_textures[i].view);
            VK_CHECK_RESULT(result);
        }
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

        m_vertices.vi_bindings[0].binding = VERTEX_BUFFER_BIND_ID;
        m_vertices.vi_bindings[0].stride = sizeof(vertices[0]);
        m_vertices.vi_bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        m_vertices.vi_attrs[0].binding = VERTEX_BUFFER_BIND_ID;
        m_vertices.vi_attrs[0].location = 0;
        m_vertices.vi_attrs[0].format = VK_FORMAT_R32G32_SFLOAT;
        m_vertices.vi_attrs[0].offset = 0;

        m_vertices.vi_attrs[1].binding = VERTEX_BUFFER_BIND_ID;
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

        // Input from vertex shader
        layout(location = 0) in vec2 inTexCoord;

        // Output color
        layout(location = 0) out vec4 outColor;

        // Texture sampler (bound via descriptor set)
        layout(binding = 0) uniform sampler2D textureSampler;

        {0}


        void main() {
            outColor = texture(textureSampler, inTexCoord);
            {1}
        }
    )").arg(p.hdrColorsDef).arg(p.hdrColors);
        // Compile to SPIR-V
        try
        {

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

    void NDIView::prepare_descriptor_pool()
    {
        VkDescriptorPoolSize type_count = {};
        type_count.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        type_count.descriptorCount = DEMO_TEXTURE_COUNT;

        VkDescriptorPoolCreateInfo descriptor_pool = {};
        descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool.pNext = NULL;
        descriptor_pool.maxSets = 1;
        descriptor_pool.poolSizeCount = 1;
        descriptor_pool.pPoolSizes = &type_count;

        VkResult result;

        result = vkCreateDescriptorPool(
            m_device, &descriptor_pool, NULL, &m_desc_pool);
        VK_CHECK_RESULT(result);
    }

    void NDIView::prepare_descriptor_set()
    {
        VkDescriptorImageInfo tex_descs[DEMO_TEXTURE_COUNT];
        VkResult result;
        uint32_t i;

        VkDescriptorSetAllocateInfo alloc_info = {};
        alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        alloc_info.pNext = NULL;
        alloc_info.descriptorPool = m_desc_pool;
        alloc_info.descriptorSetCount = 1;
        alloc_info.pSetLayouts = &m_desc_layout;

        result = vkAllocateDescriptorSets(m_device, &alloc_info, &m_desc_set);
        VK_CHECK_RESULT(result);

        memset(&tex_descs, 0, sizeof(tex_descs));
        for (i = 0; i < DEMO_TEXTURE_COUNT; i++)
        {
            tex_descs[i].sampler = m_textures[i].sampler;
            tex_descs[i].imageView = m_textures[i].view;
            tex_descs[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        }

        VkWriteDescriptorSet write = {};
        write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        write.dstSet = m_desc_set;
        write.descriptorCount = DEMO_TEXTURE_COUNT;
        write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        write.pImageInfo = tex_descs;

        vkUpdateDescriptorSets(m_device, 1, &write, 0, NULL);
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
        m_validate = true;
        m_use_staging_buffer = false;
        m_clearColor = { 0.F, 0.F, 0.F, 0.F };

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

        prepare_textures();
        prepare_vertices();
        prepare_descriptor_layout();
        prepare_render_pass();
        prepare_pipeline();
        prepare_descriptor_pool();
        prepare_descriptor_set();
    }

    void NDIView::update_texture()
    {
        TLRENDER_P();

        if (p.hasHDR)
        {
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
            m_hdr_metadata.maxLuminance = data.displayMasteringLuminance.getMax(); 
            m_hdr_metadata.minLuminance = data.displayMasteringLuminance.getMin(); 
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

        void* data;
        vkMapMemory(m_device, m_textures[0].mem, 0, m_mem_reqs.size, 0, &data);

        if (p.image)
        {
            std::memcpy(data, p.image->getData(), p.image->getDataByteCount());
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

    void NDIView::draw()
    {
        TLRENDER_P();

        // Background color
        
        draw_begin();

        update_texture();

        // Draw the triangle
        VkDeviceSize offsets[1] = {0};
        vkCmdBindVertexBuffers(
            m_draw_cmd, VERTEX_BUFFER_BIND_ID, 1, &m_vertices.buf, offsets);

        vkCmdDraw(m_draw_cmd, 4, 1, 0, 0);

        Fl_Window::draw();

        draw_end();
    }

    void NDIView::_videoThread()
    {
        TLRENDER_P();
        while (p.videoThread.running)
        {
            {
                std::unique_lock<std::mutex> lock(p.findMutex.mutex);
                if (p.currentNDISource.empty())
                    continue;
            }
            // We now have at least one source, so we create a receiver to
            // look at it.

            NDIlib_recv_create_t NDI_recv_create_desc;
            NDI_recv_create_desc.p_ndi_recv_name = "mrv2 HDR Receiver";
            NDI_recv_create_desc.source_to_connect_to =
                p.currentNDISource.c_str();
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
                        pixelAspectRatio = 1.F /
                                           ( video_frame.xres *
                                             video_frame.yres );
                
                    if (p.info.size.w != video_frame.xres ||
                        p.info.size.h != video_frame.yres ||
                        p.info.size.pixelAspectRatio != pixelAspectRatio ||
                        p.fourCC != video_frame.FourCC)
                    {
                        init = true;
                    }

                    p.info.size.w = video_frame.xres;
                    p.info.size.h = video_frame.yres;
                    p.info.size.pixelAspectRatio = pixelAspectRatio;
                    p.info.layout.mirror.y = true;
                    p.info.pixelType = image::PixelType::RGBA_F16;
                    p.info.videoLevels = image::VideoLevels::FullRange;
                    p.fourCC = video_frame.FourCC;

                    if (video_frame.p_metadata)
                    {   
                        // Parsing XML metadata
                        rapidxml::xml_document<> doc;
                        doc.parse<0>((char*)video_frame.p_metadata);

                        // Get root node
                        rapidxml::xml_node<>* root =
                            doc.first_node("ndi_color_info");

                        // Get attributes
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

                        if (attr_mrv2)
                        {
                            p.hasHDR = true;
                            const std::string& jsonString =
                                unescape_quotes_from_xml(attr_mrv2->value());
                            
                            const nlohmann::json& j =
                                nlohmann::json::parse(jsonString);
                            
                            const image::HDRData& hdrData =
                                j.get<image::HDRData>();
                            if (p.hdrData != hdrData)
                            {
                                p.hdrData = hdrData;
                                init = true;
                            }
                        }
                        else
                        {
                            p.hasHDR= false;
                        }
                    
                        // Display color information
                        // fprintf(
                        //     stderr,
                        //     "Video metadata color info (transfer: %s, matrix: "
                        //     "%s, primaries: %s)\n",
                        //     attr_transfer->value(), attr_matrix->value(),
                        //     attr_primaries->value());

                        // std::cerr << "primaries--------" << std::endl;
                        // std::cerr << hdrData.primaries[0] << " "
                        //           << hdrData.primaries[1] << " "
                        //           << hdrData.primaries[2] << " "
                        //           << hdrData.primaries[3] << std::endl;
                    }
                        
                    if (init)
                    {
                        start();
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

    void NDIView::destroy_resources()
    {
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

        for (uint32_t i = 0; i < DEMO_TEXTURE_COUNT; i++)
        {
            if (m_textures[i].view != VK_NULL_HANDLE)
            {
                vkDestroyImageView(m_device, m_textures[i].view, NULL);
                m_textures[i].view = VK_NULL_HANDLE;
            }
            if (m_textures[i].image != VK_NULL_HANDLE)
            {
                vkDestroyImage(m_device, m_textures[i].image, NULL);
                m_textures[i].image = VK_NULL_HANDLE;
            }
            if (m_textures[i].mem != VK_NULL_HANDLE)
            {
                vkFreeMemory(m_device, m_textures[i].mem, NULL);
                m_textures[i].mem = VK_NULL_HANDLE;
            }
            if (m_textures[i].sampler != VK_NULL_HANDLE)
            {
                vkDestroySampler(m_device, m_textures[i].sampler, NULL);
                m_textures[i].sampler = VK_NULL_HANDLE;
            }
        }
    }

    void NDIView::prepare_descriptor_layout()
    {
        VkDescriptorSetLayoutBinding layout_binding = {};
        layout_binding.binding = 0;
        layout_binding.descriptorType =
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        layout_binding.descriptorCount = DEMO_TEXTURE_COUNT;
        layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        layout_binding.pImmutableSamplers = NULL;

        VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
        descriptor_layout.sType =
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_layout.pNext = NULL;
        descriptor_layout.bindingCount = 1;
        descriptor_layout.pBindings = &layout_binding;

        VkResult result;

        result = vkCreateDescriptorSetLayout(
            m_device, &descriptor_layout, NULL, &m_desc_layout);
        VK_CHECK_RESULT(result);

        VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
        pPipelineLayoutCreateInfo.sType =
            VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pPipelineLayoutCreateInfo.pNext = NULL;
        pPipelineLayoutCreateInfo.setLayoutCount = 1;
        pPipelineLayoutCreateInfo.pSetLayouts = &m_desc_layout;

        result = vkCreatePipelineLayout(
            m_device, &pPipelineLayoutCreateInfo, NULL, &m_pipeline_layout);
        VK_CHECK_RESULT(result);
    }

    void NDIView::_create_HDR_shader()
    {
        TLRENDER_P();
        
#if defined(TLRENDER_LIBPLACEBO)
        pl_shader_params shader_params;
        memset(&shader_params, 0, sizeof(pl_shader_params));
    
        shader_params.id = 1;
        shader_params.gpu = p.placeboData->gpu;
        shader_params.dynamic_constants = false;
    
        pl_shader shader = pl_shader_alloc(p.placeboData->log,
                                            &shader_params);
        if (!shader)
        {
            throw std::runtime_error("pl_shader_alloc failed!");
        }


        pl_color_map_params cmap;
        memset(&cmap, 0, sizeof(pl_color_map_params));

        // defaults, generates LUTs if state is set.
        cmap.gamut_mapping = nullptr; //&pl_gamut_map_perceptual;
        cmap.tone_mapping_function = nullptr; // pl_tone_mapping clip
                    
        // PL_GAMUT_MAP_CONSTANTS is defined in wrong order for C++
        cmap.gamut_constants = { 0 };
        cmap.gamut_constants.perceptual_deadzone = 0.3F;
        cmap.gamut_constants.perceptual_strength = 0.8F;
        cmap.gamut_constants.colorimetric_gamma  = 1.80f; 
        cmap.gamut_constants.softclip_knee  = 0.70f;
        cmap.gamut_constants.softclip_desat = 0.35f;
                    
        cmap.metadata   = PL_HDR_METADATA_ANY;
        cmap.lut3d_size[0] = 48;
        cmap.lut3d_size[1] = 32;
        cmap.lut3d_size[2] = 256;
        cmap.lut_size = 256;
        cmap.visualize_rect.x0 = 0;
        cmap.visualize_rect.y0 = 0;
        cmap.visualize_rect.x1 = 1;
        cmap.visualize_rect.y1 = 1;
        cmap.contrast_smoothness = 3.5f;

        const image::HDRData& data = p.hdrData;
                
        pl_color_space src_colorspace;
        memset(&src_colorspace, 0, sizeof(pl_color_space));
        src_colorspace.primaries = PL_COLOR_PRIM_BT_2020;
        src_colorspace.transfer  = PL_COLOR_TRC_PQ;

                
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
        pl_color_space_infer(&src_colorspace);
                
                    
    
        pl_color_space dst_colorspace;
        memset(&dst_colorspace, 0, sizeof(pl_color_space));
        dst_colorspace.primaries = PL_COLOR_PRIM_BT_2020;
        dst_colorspace.transfer  = PL_COLOR_TRC_PQ;
        if (m_color_space == VK_COLOR_SPACE_HDR10_HLG_EXT)
        {
            dst_colorspace.transfer = PL_COLOR_TRC_HLG;
        }
        else if (m_color_space == VK_COLOR_SPACE_DOLBYVISION_EXT)
        {
            // \@todo:  How to handle this?
            // dst_colorspace.transfer = PL_COLOR_TRC_DOLBYVISION;
        }
        
        pl_color_space_infer(&dst_colorspace);
    
        pl_color_map_args color_map_args;
        memset(&color_map_args, 0, sizeof(pl_color_map_args));
    
        color_map_args.src = src_colorspace;
        color_map_args.dst = dst_colorspace;
        color_map_args.prelinearized = false;

        pl_shader_obj state = NULL;
        color_map_args.state = &state;  // with NULL and tonemap_clip works
    
        pl_shader_color_map_ex(shader, &cmap, &color_map_args);
    
        const pl_shader_res* res = pl_shader_finalize(shader);
        if (!res)
        {
            pl_shader_free(&shader);
            throw std::runtime_error("pl_shader_finalize failed!");
        }

        std::string hdrColors;
        std::string hdrColorsDef;
        
        std::stringstream s;
        
        for (int i = 0; i < res->num_descriptors; i++)
        {
            const pl_shader_desc* sd = &res->descriptors[i];
            const pl_desc* desc = &sd->desc;
            switch (desc->type)
            {
            case PL_DESC_SAMPLED_TEX:
            case PL_DESC_STORAGE_IMG:
            {
                static const char *types[] = {
                    "sampler1D",
                    "sampler2D",
                    "sampler3D",
                };

                pl_desc_binding binding = sd->binding;
                pl_tex tex = (pl_tex)binding.object;
                int dims = pl_tex_params_dimension(tex->params);
                const char* type = types[dims-1];

                char prefix = ' ';
                switch(tex->params.format->type)
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
                                
                s << "layout(binding=" << (i + 1) << ") uniform "
                  << prefix
                  << type
                  << " "
                  << desc->name
                  << ";"
                  << std::endl;
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
          << "// Variables"
          << "//" << std::endl << std::endl;
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
                switch(var.type)
                {
                case PL_VAR_SINT:
                {
                    int* m = (int*) shader_var.data;
                    s << " = " << m[0] << ";" << std::endl;
                    break;
                }
                case PL_VAR_UINT:
                {
                    unsigned* m = (unsigned*) shader_var.data;
                    s << " = " << m[0] << ";" << std::endl;
                    break;
                }
                case PL_VAR_FLOAT:
                {
                    float* m = (float*) shader_var.data;
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
                                if (!(r == dim_m - 1 &&
                                      c == dim_v - 1))
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
          << "//" << std::endl << std::endl;
        for (int i = 0; i < res->num_constants; ++i)
        {
            const struct pl_shader_const constant = res->constants[i];
            switch(constant.type)
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
        

        {
            std::stringstream s;
            s << "outColor = " << res->name << "(outColor);"
              << std::endl;
            p.hdrColors = s.str();
        }

        pl_shader_free(&shader);
#endif
        return;
    }
    
} // namespace mrv
