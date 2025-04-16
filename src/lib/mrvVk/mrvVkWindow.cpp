// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>

#include <FL/platform.H>
#include <FL/Fl_Vk_Utils.H>


#ifdef FLTK_USE_WAYLAND
#    include <wayland-client.h>
#endif

#include "mrvVk/mrvVkWindow.h"

namespace
{
    const char* kModule = "vk";
}

namespace mrv
{
    namespace vulkan
    {

        VkWindow::VkWindow(int X, int Y, int W, int H, const char* L) :
            Fl_Vk_Window(X, Y, W, H, L)
        {
            m_validate = true;
        }

        VkWindow::VkWindow(int W, int H, const char* L) :
            Fl_Vk_Window(W, H, L)
        {
            m_validate = true;
        }
        

        VkShaderModule VkWindow::prepare_vs() {
            VkShaderModule out = VK_NULL_HANDLE;
            // Example GLSL vertex shader
            std::string vertex_shader_glsl = R"(
        #version 450
        layout(location = 0) in vec3 inPos;
        void main() {
            gl_Position = vec4(inPos, 1.0);
        }
    )";
    
            try {
                std::vector<uint32_t> spirv = compile_glsl_to_spirv(
                    vertex_shader_glsl,
                    shaderc_vertex_shader,  // Shader type
                    "vertex_shader.glsl"    // Filename for error reporting
                    );

                out = create_shader_module(device(), spirv);
            } catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
            return out;
        }

        VkShaderModule VkWindow::prepare_fs() {
            VkShaderModule out = VK_NULL_HANDLE;
            // Example GLSL vertex shader
            std::string frag_shader_glsl = R"(
        #version 450

        // Output color
        layout(location = 0) out vec4 outColor;

        void main() {
            outColor = vec4(0.5, 0.6, 0.7, 1.0);
        }
    )";
            // Compile to SPIR-V
            try {

                std::vector<uint32_t> spirv = compile_glsl_to_spirv(
                    frag_shader_glsl,
                    shaderc_fragment_shader,  // Shader type
                    "frag_shader.glsl"    // Filename for error reporting
                    );
                // Assuming you have a VkDevice 'device' already created
                out = create_shader_module(device(), spirv);
    
            } catch (const std::exception& e) {
                std::cerr << e.what() << std::endl;
            }
            return out;
        }

        // m_depth (optionally) -> creates m_renderPass
        void VkWindow::prepare_render_pass() 
        {
            bool has_depth = mode() & FL_DEPTH;
            bool has_stencil = mode() & FL_STENCIL;

            VkAttachmentDescription attachments[2];
            attachments[0] = VkAttachmentDescription();
            attachments[0].format = format();
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
            depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    
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
            rp_info.attachmentCount = (has_depth || has_stencil) ? 2: 1;
            rp_info.pAttachments = attachments;
            rp_info.subpassCount = 1;
            rp_info.pSubpasses = &subpass;
            rp_info.dependencyCount = 0;
            rp_info.pDependencies = NULL;
                    
            VkResult result;
            result = vkCreateRenderPass(device(), &rp_info, NULL, &m_renderPass);
            VK_CHECK(result);
        }
        
        void VkWindow::prepare_vertices()
        {
            // clang-format off
            struct Vertex
            {
                float x, y, z;  // 3D position
            };
    
            // Add the center vertex
            Vertex center = {0.0f, 0.0f, 0.0f};

            // Generate the outer vertices
            std::vector<Vertex> outerVertices(sides);
            for (int j = 0; j < sides; ++j) {
                double ang = j * 2 * M_PI / sides;
                float x = cos(ang);
                float y = sin(ang);
                outerVertices[j].x = x;
                outerVertices[j].y = y;
                outerVertices[j].z = 0.0f;
            }

            // Create the triangle list
            std::vector<Vertex> vertices;
            for (int i = 0; i < sides; ++i) {
                // First vertex of the triangle: the center
                vertices.push_back(center);

                // Second vertex: current outer vertex
                vertices.push_back(outerVertices[i]);

                // Third vertex: next outer vertex (wrap around for the last side)
                vertices.push_back(outerVertices[(i + 1) % sides]);
            }
    
            
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
    
            VkResult result;
            bool pass;
            void *data;

            memset(&m_mesh, 0, sizeof(m_mesh));

            result = vkCreateBuffer(device(), &buf_info, NULL, &m_mesh.buf);
            VK_CHECK(result);

            vkGetBufferMemoryRequirements(device(), m_mesh.buf, &m_mem_reqs);
            VK_CHECK(result);

            mem_alloc.allocationSize = m_mem_reqs.size;
            pass = memory_type_from_properties(gpu(),
                                               m_mem_reqs.memoryTypeBits,
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                               &mem_alloc.memoryTypeIndex);

            result = vkAllocateMemory(device(), &mem_alloc, NULL, &m_mesh.mem);
            VK_CHECK(result);

            result = vkMapMemory(device(), m_mesh.mem, 0,
                                 mem_alloc.allocationSize, 0, &data);
            VK_CHECK(result);

            memcpy(data, vertices.data(), static_cast<size_t>(buffer_size));

            vkUnmapMemory(device(), m_mesh.mem);

            result = vkBindBufferMemory(device(), m_mesh.buf, m_mesh.mem, 0);
            VK_CHECK(result);

            m_mesh.vi.sType =
                VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
            m_mesh.vi.pNext = NULL;
            m_mesh.vi.vertexBindingDescriptionCount = 1;
            m_mesh.vi.pVertexBindingDescriptions = m_mesh.vi_bindings;
            m_mesh.vi.vertexAttributeDescriptionCount = 1;
            m_mesh.vi.pVertexAttributeDescriptions = m_mesh.vi_attrs;

            m_mesh.vi_bindings[0].binding = 0;
            m_mesh.vi_bindings[0].stride = sizeof(vertices[0]);
            m_mesh.vi_bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            m_mesh.vi_attrs[0].binding = 0;
            m_mesh.vi_attrs[0].location = 0;
            m_mesh.vi_attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
            m_mesh.vi_attrs[0].offset = 0;
        }

        void VkWindow::prepare_pipeline() {
            VkGraphicsPipelineCreateInfo pipeline;
            VkPipelineCacheCreateInfo pipelineCacheCreateInfo;

            VkPipelineVertexInputStateCreateInfo vi;
            VkPipelineInputAssemblyStateCreateInfo ia;
            VkPipelineRasterizationStateCreateInfo rs;
            VkPipelineColorBlendStateCreateInfo cb;
            VkPipelineDepthStencilStateCreateInfo ds;
            VkPipelineViewportStateCreateInfo vp;
            VkPipelineMultisampleStateCreateInfo ms;
            VkDynamicState dynamicStateEnables[(VK_DYNAMIC_STATE_STENCIL_REFERENCE - VK_DYNAMIC_STATE_VIEWPORT + 1)];
            VkPipelineDynamicStateCreateInfo dynamicState;

            VkResult result;
    
            memset(dynamicStateEnables, 0, sizeof dynamicStateEnables);
            memset(&dynamicState, 0, sizeof dynamicState);
            dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
            dynamicState.pDynamicStates = dynamicStateEnables;

            memset(&pipeline, 0, sizeof(pipeline));
            pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
            pipeline.layout = m_pipeline_layout;

            vi = m_mesh.vi;

            memset(&ia, 0, sizeof(ia));
            ia.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
            ia.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

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

            shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
            shaderStages[0].module = prepare_vs();
            shaderStages[0].pName = "main";

            shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
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

            memset(&pipelineCacheCreateInfo, 0, sizeof(pipelineCacheCreateInfo));
            pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

            result = vkCreatePipelineCache(device(), &pipelineCacheCreateInfo, NULL,
                                           &pipelineCache());
            VK_CHECK(result);
            result = vkCreateGraphicsPipelines(device(), pipelineCache(), 1,
                                               &pipeline, NULL, &m_pipeline);
            VK_CHECK(result);

            vkDestroyPipelineCache(device(), pipelineCache(), NULL);

        }

        void VkWindow::prepare_descriptor_layout() {
            VkResult result;
    
            VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
            pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
            pPipelineLayoutCreateInfo.pNext = NULL;
            pPipelineLayoutCreateInfo.setLayoutCount = 0;
            pPipelineLayoutCreateInfo.pSetLayouts = NULL;

            result = vkCreatePipelineLayout(device(), &pPipelineLayoutCreateInfo, NULL,
                                            &m_pipeline_layout);
            VK_CHECK(result);
        }

        void VkWindow::prepare()
        {
            prepare_vertices();
            prepare_descriptor_layout();
            prepare_render_pass();
            prepare_pipeline();
            prepare_descriptor_pool();
            prepare_descriptor_set();
        }


        void VkWindow::vk_draw_begin()
        {
            // Background color
            m_clearColor = { 0.0, 0.0, 0.0, 0.0 };

            Fl_Vk_Window::vk_draw_begin();
        }

        void VkWindow::draw() {
            if (!shown() || w() <= 0 || h() <= 0) return;

            // Draw the shape
            VkDeviceSize offsets[1] = {0};
            vkCmdBindVertexBuffers(m_draw_cmd, 0, 1,
                                   &m_mesh.buf, offsets);

            vkCmdDraw(m_draw_cmd, sides * 3, 1, 0, 0);
        }

        void VkWindow::destroy_resources() {
            m_mesh.destroy(device());

            Fl_Vk_Window::destroy_resources();
        }


        void VkWindow::show()
        {
            Fl_Vk_Window::show();

#ifdef FLTK_USE_WAYLAND
            // Not sure if this is needed
            if (fl_wl_display())
            {
                wl_surface_set_opaque_region(fl_wl_surface(fl_wl_xid(this)), NULL);
            }
#endif
        }

    }

    
} // namespace mrv
