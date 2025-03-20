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

#include <limits>

#include <FL/platform.H>
#include <FL/Fl.H>

#include <FL/Fl_Vk_Window.H>
#include <FL/Fl_Vk_Utils.H>

#include "mrvHDR/mrvNDIView.h"

namespace mrv
{

    NDIView::~NDIView()
    {
        vkDestroyShaderModule(m_device, m_frag_shader_module, NULL);
        vkDestroyShaderModule(m_device, m_vert_shader_module, NULL);
    }

    NDIView::NDIView(int x, int y, int w, int h, const char* l) :
        Fl_Vk_Window(x, y, w, h, l)
    {
        mode(FL_RGB | FL_DOUBLE | FL_ALPHA | FL_DEPTH);
        m_vert_shader_module = VK_NULL_HANDLE;
        m_frag_shader_module = VK_NULL_HANDLE;
    }

    NDIView::NDIView(int w, int h, const char* l) :
        Fl_Vk_Window(w, h, l)
    {
        mode(FL_RGB | FL_DOUBLE | FL_ALPHA | FL_DEPTH);
        m_vert_shader_module = VK_NULL_HANDLE;
        m_frag_shader_module = VK_NULL_HANDLE;
    }

    // Uses m_cmd_pool, m_setup_cmd
    void NDIView::set_image_layout(
        VkImage image, VkImageAspectFlags aspectMask,
        VkImageLayout old_image_layout, VkImageLayout new_image_layout,
        int srcAccessMaskInt)
    {
        VkResult err;

        VkAccessFlagBits srcAccessMask =
            static_cast<VkAccessFlagBits>(srcAccessMaskInt);
        if (m_setup_cmd == VK_NULL_HANDLE)
        {
            VkCommandBufferAllocateInfo cmd = {};
            cmd.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
            cmd.pNext = NULL;
            cmd.commandPool = m_cmd_pool;
            cmd.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
            cmd.commandBufferCount = 1;

            err = vkAllocateCommandBuffers(m_device, &cmd, &m_setup_cmd);
            VkCommandBufferBeginInfo cmd_buf_info = {};
            cmd_buf_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            cmd_buf_info.pNext = NULL;
            cmd_buf_info.flags = 0;
            cmd_buf_info.pInheritanceInfo = NULL;

            err = vkBeginCommandBuffer(m_setup_cmd, &cmd_buf_info);
        }

        VkImageMemoryBarrier image_memory_barrier = {};
        image_memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        image_memory_barrier.pNext = NULL;
        image_memory_barrier.srcAccessMask = srcAccessMask;
        image_memory_barrier.dstAccessMask = 0;
        image_memory_barrier.oldLayout = old_image_layout;
        image_memory_barrier.newLayout = new_image_layout;
        image_memory_barrier.image = image;
        image_memory_barrier.subresourceRange = {aspectMask, 0, 1, 0, 1};

        VkPipelineStageFlags src_stages =
            VK_PIPELINE_STAGE_HOST_BIT; // Default for host writes
        VkPipelineStageFlags dest_stages =
            VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT; // Default

        // Adjust source and destination stages based on access and layout
        if (srcAccessMask & VK_ACCESS_HOST_WRITE_BIT)
        {
            src_stages = VK_PIPELINE_STAGE_HOST_BIT;
        }
        else if (srcAccessMask & VK_ACCESS_TRANSFER_WRITE_BIT)
        {
            src_stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }

        if (new_image_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            image_memory_barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            dest_stages = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (new_image_layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL)
        {
            image_memory_barrier.dstAccessMask =
                VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            dest_stages = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        }
        else if (
            new_image_layout ==
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            image_memory_barrier.dstAccessMask =
                VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            dest_stages = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        else if (new_image_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            image_memory_barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
            dest_stages = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }

        vkCmdPipelineBarrier(
            m_setup_cmd, src_stages, dest_stages, 0, 0, NULL, 0, NULL, 1,
            &image_memory_barrier);
    }

    void NDIView::prepare_texture_image(
        const uint32_t* tex_colors, Fl_Vk_Texture* tex_obj,
        VkImageTiling tiling, VkImageUsageFlags usage, VkFlags required_props)
    {
        const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
        const int32_t tex_width = 2;
        const int32_t tex_height = 2;
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
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;

        VkMemoryAllocateInfo mem_alloc = {};
        mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        mem_alloc.pNext = NULL;
        mem_alloc.allocationSize = 0;
        mem_alloc.memoryTypeIndex = 0;

        VkMemoryRequirements mem_reqs;

        result =
            vkCreateImage(m_device, &image_create_info, NULL, &tex_obj->image);
        VK_CHECK_RESULT(result);

        vkGetImageMemoryRequirements(m_device, tex_obj->image, &mem_reqs);

        mem_alloc.allocationSize = mem_reqs.size;
        pass = memory_type_from_properties(
            mem_reqs.memoryTypeBits, required_props,
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

            for (y = 0; y < tex_height; y++)
            {
                uint32_t* row = (uint32_t*)((char*)data + layout.rowPitch * y);
                for (x = 0; x < tex_width; x++)
                    row[x] = tex_colors[(x & 1) ^ (y & 1)];
            }

            vkUnmapMemory(m_device, tex_obj->mem);
        }

        tex_obj->imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        set_image_layout(
            tex_obj->image, VK_IMAGE_ASPECT_COLOR_BIT,
            VK_IMAGE_LAYOUT_PREINITIALIZED, tex_obj->imageLayout,
            VK_ACCESS_HOST_WRITE_BIT);
        /* setting the image layout does not reference the actual memory so no
         * need to add a mem ref */
    }

    void NDIView::prepare_textures()
    {
        const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
        VkFormatProperties props;
        const uint32_t tex_colors[DEMO_TEXTURE_COUNT][2] = {
            {0xffff0000, 0xff00ff00},
        };
        uint32_t i;
        VkResult result;

        vkGetPhysicalDeviceFormatProperties(m_gpu, tex_format, &props);

        for (i = 0; i < DEMO_TEXTURE_COUNT; i++)
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
                /* Must use staging buffer to copy linear texture to optimized
                 */
                Fl_Vk_Texture staging_texture;

                memset(&staging_texture, 0, sizeof(staging_texture));
                prepare_texture_image(
                    tex_colors[i], &staging_texture, VK_IMAGE_TILING_LINEAR,
                    VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
                    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

                prepare_texture_image(
                    tex_colors[i], &m_textures[i], VK_IMAGE_TILING_OPTIMAL,
                    (VK_IMAGE_USAGE_TRANSFER_DST_BIT |
                     VK_IMAGE_USAGE_SAMPLED_BIT),
                    VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

                set_image_layout(
                    staging_texture.image, VK_IMAGE_ASPECT_COLOR_BIT,
                    staging_texture.imageLayout,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 0);

                set_image_layout(
                    m_textures[i].image, VK_IMAGE_ASPECT_COLOR_BIT,
                    m_textures[i].imageLayout,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 0);

                VkImageCopy copy_region = {};
                copy_region.srcSubresource = {
                    VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
                copy_region.srcOffset = {0, 0, 0};
                copy_region.dstSubresource = {
                    VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1};
                copy_region.dstOffset = {0, 0, 0};
                copy_region.extent = {
                    (uint32_t)staging_texture.tex_width,
                    (uint32_t)staging_texture.tex_height, 1};
                vkCmdCopyImage(
                    m_setup_cmd, staging_texture.image,
                    VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, m_textures[i].image,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copy_region);

                set_image_layout(
                    m_textures[i].image, VK_IMAGE_ASPECT_COLOR_BIT,
                    VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    m_textures[i].imageLayout, 0);

                flush_init_cmd();

                destroy_texture_image(&staging_texture);
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
        // clang-format off
        struct Vertex
        {
            float x, y;
            float u, v;      // UV coordinates
        };

        std::vector<Vertex> vertices;
        vertices.push_back({-1.F, -1.F, 0.F, 0.F});
        vertices.push_back({ 1.F, -1.F, 1.F, 0.F});
        vertices.push_back({ 1.F,  1.F, 1.F, 1.F});
        vertices.push_back({-1.F,  1.F, 0.F, 1.F});
    
            
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

        memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));

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

        // Example GLSL vertex shader
        std::string frag_shader_glsl = R"(
        #version 450

        // Input from vertex shader
        layout(location = 0) in vec2 inTexCoord;

        // Output color
        layout(location = 0) out vec4 outColor;

        // Texture sampler (bound via descriptor set)
        layout(binding = 0) uniform sampler2D textureSampler;

        void main() {
            outColor = texture(textureSampler, inTexCoord);
        }
    )";
        // Compile to SPIR-V
        try
        {

            std::vector<uint32_t> spirv = compile_glsl_to_spirv(
                frag_shader_glsl,
                shaderc_fragment_shader, // Shader type
                "frag_shader.glsl"       // Filename for error reporting
            );
            // Assuming you have a VkDevice 'device' already created
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

    void NDIView::prepare()
    {
        prepare_textures();
        prepare_vertices();
        prepare_descriptor_layout();
        prepare_render_pass();
        prepare_pipeline();
        prepare_descriptor_pool();
        prepare_descriptor_set();
    }

    void NDIView::draw()
    {
        if (!shown() || w() <= 0 || h() <= 0)
            return;

        // Background color
        m_clearColor = {0.0, 0.0, 1.0, 1.0};

        draw_begin();

        // Draw the triangle
        VkDeviceSize offsets[1] = {0};
        vkCmdBindVertexBuffers(
            m_draw_cmd, VERTEX_BUFFER_BIND_ID, 1, &m_vertices.buf, offsets);

        vkCmdDraw(m_draw_cmd, 4, 1, 0, 0);

        Fl_Window::draw();

        draw_end();
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

} // namespace mrv
