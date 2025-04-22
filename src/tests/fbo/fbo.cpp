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
/*
 * Code based on:
 *
 * Copyright (c) 2015-2016 The Khronos Group Inc.
 * Copyright (c) 2015-2016 Valve Corporation
 * Copyright (c) 2015-2016 LunarG, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Author: Chia-I Wu <olvaffe@gmail.com>
 * Author: Cody Northrop <cody@lunarg.com>
 * Author: Courtney Goeltzenleuchter <courtney@LunarG.com>
 * Author: Ian Elliott <ian@LunarG.com>
 * Author: Jon Ashburn <jon@lunarg.com>
 * Author: Piers Daniell <pdaniell@nvidia.com>
 * Author: Gwan-gyeong Mun <elongbug@gmail.com>
 * Author: Camilla Löwy <elmindreda@glfw.org>
 * Porter: Gonzalo Garramuño <ggarra13@gmail.com>
 */

#define USE_VBO 1

#include <tlVk/Mesh.h>
#include <tlVk/OffscreenBuffer.h>

#include <tlCore/Size.h>

#include <FL/platform.H>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Hor_Slider.H>
#include <FL/math.h>

#include <iostream>
#include <limits>
#include <FL/Fl_Vk_Window.H>
#include <FL/Fl_Vk_Utils.H>

class vk_shape_window : public Fl_Vk_Window {
    void draw() FL_OVERRIDE;
public:
    int sides;
    vk_shape_window(int x,int y,int w,int h,const char *l=0);
    vk_shape_window(int w,int h,const char *l=0);

    virtual void hide() FL_OVERRIDE;
    
    const char* application_name() FL_OVERRIDE { return "vk_shape_textured"; };
    void prepare() FL_OVERRIDE;
    void destroy_resources() FL_OVERRIDE;


    void destroy_mesh();
    void prepare_mesh();
    
protected:
    //! Shaders used in demo
    VkShaderModule m_vert_shader_fbo_module;
    VkShaderModule m_frag_shader_fbo_module;
    
    VkShaderModule m_vert_shader_module;
    VkShaderModule m_frag_shader_module;
    uint32_t frame_counter = 0;

    //! This is for the fbo pipeline
    VkPipeline m_fbo_pipeline;
    VkPipelineLayout m_fbo_pipeline_layout;
    
    //! This is for holding a mesh
    Fl_Vk_Mesh m_mesh;

    //! Memory for descriptor sets
    VkDescriptorPool      m_desc_pool;

    //! Describe texture bindings whithin desc. set  
    VkDescriptorSetLayout m_desc_layout;

    //! Actual data bound to shaders like texture or
    //! uniform buffers
    VkDescriptorSet       m_desc_set; 
    
private:
    void _init();

    void prepare_textures();
    void prepare_descriptor_layout();
    void prepare_render_pass();
    void prepare_fbo_pipeline();
    void prepare_pipeline();
    void prepare_descriptor_pool();
    void prepare_descriptor_set();
    void prepare_texture_image(const uint32_t *tex_colors,
                               Fl_Vk_Texture* tex_obj,
                               VkImageTiling tiling,
                               VkImageUsageFlags usage,
                               VkFlags required_props);

    VkShaderModule prepare_vs();
    VkShaderModule prepare_fs();
    VkShaderModule prepare_fbo_vs();
    VkShaderModule prepare_fbo_fs();

    std::shared_ptr<tl::vk::OffscreenBuffer> fbo;
    std::shared_ptr<tl::vk::VBO> vbo;
    std::shared_ptr<tl::vk::VAO> vao;
};

void vk_shape_window::_init()
{
    mode(FL_RGB | FL_DOUBLE | FL_ALPHA | FL_DEPTH | FL_STENCIL);
    sides = 3;
    // Turn on validations
    m_validate = true;
    m_vert_shader_module = VK_NULL_HANDLE;
    m_frag_shader_module = VK_NULL_HANDLE;
}

vk_shape_window::vk_shape_window(int x,int y,int w,int h,const char *l) :
Fl_Vk_Window(x,y,w,h,l) {
    _init();
}

vk_shape_window::vk_shape_window(int w,int h,const char *l) :
Fl_Vk_Window(w,h,l)
{
    _init();
}

void vk_shape_window::hide()
{
    fbo.reset();
    vao.reset();
    vbo.reset();
    Fl_Vk_Window::hide();
}

void vk_shape_window::prepare_mesh()
{
    // clang-format off
    struct Vertex
    {
        float x, y, z;  // 3D position
        float u, v;      // UV coordinates
    };
    
    // Add the center vertex
    Vertex center = {0.0f, 0.0f, 0.0f, 0.5f, 0.5f};

    // Generate the outer vertices
    std::vector<Vertex> outerVertices(sides);
    float z = 0.0F;
    for (int j = 0; j < sides; ++j) {
        double ang = j * 2 * M_PI / sides;
        float x = cos(ang);
        float y = sin(ang);
        outerVertices[j].x = x;
        outerVertices[j].y = y;
        outerVertices[j].z = z;
        
        // Map NDC coordinates [-1, 1] to UV coordinates [0, 1], flipping V
        outerVertices[j].u = (x + 1.0f) / 2.0f;
        outerVertices[j].v = 1.0f - (y + 1.0f) / 2.0f;
        z += std::min(j / (float)(sides - 1), 1.F);
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
    
    using namespace tl;

    vbo.reset();
    vao.reset();

    const size_t numTriangles = sides;
    
    geom::TriangleMesh2 mesh = geom::box(math::Box2f(-.5F, -0.5F, 1.F, 1.F));    
    if (!vbo || (vbo && vbo->getSize() != numTriangles * 3))
    {
        vbo = vk::VBO::create(numTriangles * 3, vk::VBOType::Pos2_F32_UV_U16);
        vao.reset();
    }
    if (vbo)
    {
        vbo->copy(convert(mesh, vk::VBOType::Pos2_F32_UV_U16));
    }

    if (!vao && vbo)
    {
        vao = vk::VAO::create(ctx, vbo->getType(), vbo->getID());
        vao->upload(vbo->getData());
    }
    
    VkResult result;
    VkDeviceSize buffer_size = sizeof(vertices[0]) * vertices.size();
    
    // clang-format on
    VkBufferCreateInfo buf_info = {};
    buf_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    buf_info.pNext = NULL;
    buf_info.size = buffer_size;
    buf_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    buf_info.flags = 0;

    result = vkCreateBuffer(device(), &buf_info, NULL, &m_mesh.buf);
    VK_CHECK(result);
    
    // Use a local variable instead of overwriting m_mem_reqs
    VkMemoryRequirements vertex_mem_reqs;
    vkGetBufferMemoryRequirements(device(), m_mesh.buf, &vertex_mem_reqs);
    
    VkMemoryAllocateInfo mem_alloc = {};
    mem_alloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    mem_alloc.pNext = NULL;
    mem_alloc.allocationSize = vertex_mem_reqs.size;
    mem_alloc.memoryTypeIndex = 0;
    
    bool pass;
    void *data;
    
    mem_alloc.memoryTypeIndex =
        findMemoryType(gpu(),
                       vertex_mem_reqs.memoryTypeBits,
                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                       VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

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
    m_mesh.vi.vertexAttributeDescriptionCount = 2;
    m_mesh.vi.pVertexAttributeDescriptions = m_mesh.vi_attrs;

    m_mesh.vi_bindings[0].binding = 0;
    m_mesh.vi_bindings[0].stride = sizeof(vertices[0]);
    m_mesh.vi_bindings[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    m_mesh.vi_attrs[0].binding = 0;
    m_mesh.vi_attrs[0].location = 0;
    m_mesh.vi_attrs[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    m_mesh.vi_attrs[0].offset = 0;

    m_mesh.vi_attrs[1].binding = 0;
    m_mesh.vi_attrs[1].location = 1;
    m_mesh.vi_attrs[1].format = VK_FORMAT_R32G32_SFLOAT;
    m_mesh.vi_attrs[1].offset = sizeof(float) * 3;  // skip 3 vertex coords    
}

// m_depth (optionally) -> creates m_renderPass
void vk_shape_window::prepare_render_pass() 
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

VkShaderModule vk_shape_window::prepare_vs() {
    if (m_vert_shader_module != VK_NULL_HANDLE)
        return m_vert_shader_module;
    
    // Example GLSL vertex shader
    std::string vertex_shader_glsl = R"(
        #version 450
        layout(location = 0) in vec3 inPos;
        layout(location = 1) in vec2 inTexCoord;
        layout(location = 0) out vec2 outTexCoord;
        void main() {
            gl_Position = vec4(inPos, 1.0);
            outTexCoord = inTexCoord;
        }
    )";
    
    try {
        std::vector<uint32_t> spirv = compile_glsl_to_spirv(
            vertex_shader_glsl,
            shaderc_vertex_shader,  // Shader type
            "vertex_shader.glsl"    // Filename for error reporting
        );

        m_vert_shader_module = create_shader_module(device(), spirv);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        m_vert_shader_module = VK_NULL_HANDLE;
    }
    return m_vert_shader_module;
}

VkShaderModule vk_shape_window::prepare_fbo_vs() {
    if (m_vert_shader_fbo_module != VK_NULL_HANDLE)
        return m_vert_shader_fbo_module;
    
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

        m_vert_shader_fbo_module = create_shader_module(device(), spirv);
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        m_vert_shader_fbo_module = VK_NULL_HANDLE;
    }
    return m_vert_shader_fbo_module;
}

VkShaderModule vk_shape_window::prepare_fbo_fs() {
    if (m_frag_shader_fbo_module != VK_NULL_HANDLE)
        return m_frag_shader_fbo_module;
    
    // Example GLSL vertex shader
    std::string frag_shader_glsl = R"(
        #version 450
        // Output color
        layout(location = 0) out vec4 outColor;

        void main() {
            outColor = vec4(1, 1, 1, 1);
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
        m_frag_shader_fbo_module = create_shader_module(device(), spirv);
    
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        m_frag_shader_fbo_module = VK_NULL_HANDLE;
    }
    return m_frag_shader_fbo_module;
}

VkShaderModule vk_shape_window::prepare_fs() {
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
    try {

        std::vector<uint32_t> spirv = compile_glsl_to_spirv(
            frag_shader_glsl,
            shaderc_fragment_shader,  // Shader type
            "frag_shader.glsl"    // Filename for error reporting
        );
        // Assuming you have a VkDevice 'device' already created
        m_frag_shader_module = create_shader_module(device(), spirv);
    
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        m_frag_shader_module = VK_NULL_HANDLE;
    }
    return m_frag_shader_module;
}

void vk_shape_window::prepare_fbo_pipeline() {
    VkGraphicsPipelineCreateInfo pipeline;
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo;

    VkPipelineVertexInputStateCreateInfo vi = {};
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


    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext = NULL;
    pPipelineLayoutCreateInfo.setLayoutCount = 0;

    result = vkCreatePipelineLayout(device(), &pPipelineLayoutCreateInfo, NULL,
                                    &m_fbo_pipeline_layout);

    
    memset(&pipeline, 0, sizeof(pipeline));
    pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline.layout = m_fbo_pipeline_layout;

    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vi.pNext = NULL;
    vi.vertexBindingDescriptionCount = 1;
    vi.pVertexBindingDescriptions = vbo->getBindingDescription();
    vi.vertexAttributeDescriptionCount = vbo->getAttributes().size();
    vi.pVertexAttributeDescriptions = vbo->getAttributes().data();

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

    bool has_depth = fbo->hasDepth();
    bool has_stencil = fbo->hasStencil();
    
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
    shaderStages[0].module = prepare_fbo_vs();
    shaderStages[0].pName = "main";

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = prepare_fbo_fs();
    shaderStages[1].pName = "main";

    pipeline.pVertexInputState = &vi;
    pipeline.pInputAssemblyState = &ia;
    pipeline.pRasterizationState = &rs;
    pipeline.pColorBlendState = &cb;
    pipeline.pMultisampleState = &ms;
    pipeline.pViewportState = &vp;
    pipeline.pDepthStencilState = &ds;
    pipeline.pStages = shaderStages;
    pipeline.renderPass = fbo->getRenderPass();
    pipeline.pDynamicState = &dynamicState;

    memset(&pipelineCacheCreateInfo, 0, sizeof(pipelineCacheCreateInfo));
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;

    VkPipelineCache pipelineCache;
    
    result = vkCreatePipelineCache(device(), &pipelineCacheCreateInfo, NULL,
                                   &pipelineCache);
    VK_CHECK(result);
    
    result = vkCreateGraphicsPipelines(device(), pipelineCache, 1,
                                       &pipeline, NULL, &m_fbo_pipeline);
    VK_CHECK(result);

    vkDestroyPipelineCache(device(), pipelineCache, NULL);
}

void vk_shape_window::prepare_pipeline() {
    VkGraphicsPipelineCreateInfo pipeline;
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo;

    VkPipelineVertexInputStateCreateInfo vi = {};
    VkPipelineInputAssemblyStateCreateInfo ia = {};
    VkPipelineRasterizationStateCreateInfo rs = {};
    VkPipelineColorBlendStateCreateInfo cb = {};
    VkPipelineDepthStencilStateCreateInfo ds = {};
    VkPipelineViewportStateCreateInfo vp = {};
    VkPipelineMultisampleStateCreateInfo ms = {};
    VkDynamicState dynamicStateEnables[(VK_DYNAMIC_STATE_STENCIL_REFERENCE - VK_DYNAMIC_STATE_VIEWPORT + 1)];
    VkPipelineDynamicStateCreateInfo dynamicState = {};

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
    rs.cullMode = VK_CULL_MODE_BACK_BIT;
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

    VkPipelineCache pipelineCache;
    result = vkCreatePipelineCache(device(), &pipelineCacheCreateInfo, NULL,
                                   &pipelineCache);
    VK_CHECK(result);
    result = vkCreateGraphicsPipelines(device(), pipelineCache, 1,
                                       &pipeline, NULL, &m_pipeline);
    VK_CHECK(result);

    vkDestroyPipelineCache(device(), pipelineCache, NULL);

}


void vk_shape_window::prepare_descriptor_pool() {
    VkDescriptorPoolSize type_count = {};
    type_count.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    type_count.descriptorCount = 1;  // one texture
    
    VkDescriptorPoolCreateInfo descriptor_pool = {};
    descriptor_pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    descriptor_pool.pNext = NULL;
    descriptor_pool.maxSets = 1;
    descriptor_pool.poolSizeCount = 1;
    descriptor_pool.pPoolSizes = &type_count;

    VkResult result;
             
    result = vkCreateDescriptorPool(device(), &descriptor_pool, NULL,
                                    &m_desc_pool);
    VK_CHECK(result);
}

void vk_shape_window::prepare_descriptor_set() {
    VkResult result;

    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.pNext = NULL;
    alloc_info.descriptorPool = m_desc_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &m_desc_layout;
        
    result = vkAllocateDescriptorSets(device(), &alloc_info, &m_desc_set);
    VK_CHECK(result);
}

void vk_shape_window::prepare()
{
    prepare_mesh();
    prepare_descriptor_layout();
    prepare_render_pass();
    prepare_pipeline();
    prepare_descriptor_pool();
    prepare_descriptor_set();
}

void vk_shape_window::draw() {
    if (!shown() || w() <= 0 || h() <= 0)
        return;
    
    using namespace tl;

    VkCommandBuffer cmd = getCurrentCommandBuffer();
    if (!m_swapchain || !cmd || !isFrameActive()) {
        return;
    }
    
    math::Size2i size(320, 240);
    vk::OffscreenBufferOptions options;
    options.colorType = vk::offscreenColorDefault;
    fbo = vk::OffscreenBuffer::create(ctx, size, options);
    
    // Clear red + clear depth/stencil
    VkClearValue clearValues[1];
    clearValues[0].color = { 1.f, 1.f, 0.f, 1.f };

    VkRenderPassBeginInfo rpBegin{};
    rpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpBegin.renderPass = fbo->getRenderPass();
    rpBegin.framebuffer = fbo->getFramebuffer();
    rpBegin.renderArea.extent = fbo->getExtent();
    rpBegin.clearValueCount = 1;
    rpBegin.pClearValues = clearValues;

    VkCommandBuffer cmdBuffer = beginSingleTimeCommands(device(), commandPool());
    
    vkCmdBeginRenderPass(cmdBuffer, &rpBegin, VK_SUBPASS_CONTENTS_INLINE);

    prepare_fbo_pipeline();
    
    vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      m_fbo_pipeline);
    
    VkExtent2D extent = fbo->getExtent();

    // Draw commands here...
    VkViewport viewport = {};
    viewport.width = static_cast<float>(extent.width);
    viewport.height = static_cast<float>(extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    
    vkCmdSetViewport(cmdBuffer, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.extent = extent;
    vkCmdSetScissor(cmdBuffer, 0, 1, &scissor);
    
    vao->draw(cmdBuffer, vbo);
    
    vkCmdEndRenderPass(cmdBuffer);
    
    // After rendering
    fbo->transitionToShaderRead(cmdBuffer);
    
    endSingleTimeCommands(cmdBuffer, device(), commandPool(), queue());

    
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

    VkDescriptorImageInfo tex_descs[1];
    memset(&tex_descs, 0, sizeof(tex_descs));
    tex_descs[0].sampler = fbo->getSampler();
    tex_descs[0].imageView = fbo->getImageView();
    tex_descs[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = m_desc_set;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = tex_descs;

    vkUpdateDescriptorSets(device(), 1, &write, 0, NULL);

    
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_pipeline_layout, 0, 1, &m_desc_set, 0, nullptr);

    viewport = {};
    viewport.width = static_cast<float>(w());
    viewport.height = static_cast<float>(h());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    scissor = {};
    scissor.extent.width = w();
    scissor.extent.height = h();
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    VkDeviceSize offsets[1] = {0};
    vkCmdBindVertexBuffers(cmd, 0, 1, &m_mesh.buf, offsets);
    vkCmdDraw(cmd, 3 * sides, 1, 0, 0); // Draw shape
    
    vkCmdEndRenderPass(cmd);
    
    // Before next render
    // cmdBuffer = beginSingleTimeCommands(device(), commandPool());
    // fbo->transitionToColorAttachment(cmdBuffer);
    // endSingleTimeCommands(cmdBuffer, device(), commandPool(), queue());
}

void vk_shape_window::destroy_mesh()
{
    m_mesh.destroy(device());
}

void vk_shape_window::destroy_resources()
{
    if (device() == VK_NULL_HANDLE)
        return;

    destroy_mesh();
    
    if (m_pipeline_layout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(device(), m_pipeline_layout, nullptr);
        m_pipeline_layout = VK_NULL_HANDLE;
    }
    
    if (m_desc_layout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device(), m_desc_layout, nullptr);
        m_desc_layout = VK_NULL_HANDLE;
    }
    
    if (m_desc_pool != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorPool(device(), m_desc_pool, nullptr);
        m_desc_pool = VK_NULL_HANDLE;
    }
    
    if (m_vert_shader_module != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(device(), m_vert_shader_module, nullptr);
        m_vert_shader_module = VK_NULL_HANDLE;
    }
    
    if (m_frag_shader_module != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(device(), m_frag_shader_module, nullptr);
        m_frag_shader_module = VK_NULL_HANDLE;
    }
    
    if (m_fbo_pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(device(), m_fbo_pipeline, nullptr);
        m_fbo_pipeline = VK_NULL_HANDLE;
    }
    
    if (m_fbo_pipeline_layout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(device(), m_fbo_pipeline_layout, nullptr);
        m_fbo_pipeline_layout = VK_NULL_HANDLE;
    }
    
    if (m_vert_shader_fbo_module != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(device(), m_vert_shader_fbo_module, nullptr);
        m_vert_shader_module = VK_NULL_HANDLE;
    }
    
    if (m_frag_shader_fbo_module != VK_NULL_HANDLE)
    {
        vkDestroyShaderModule(device(), m_frag_shader_fbo_module, nullptr);
        m_frag_shader_fbo_module = VK_NULL_HANDLE;
    }
}


void vk_shape_window::prepare_descriptor_layout()
{
    VkDescriptorSetLayoutBinding layout_binding = {};
    layout_binding.binding = 0;
    layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    layout_binding.pImmutableSamplers = NULL;
  
    VkDescriptorSetLayoutCreateInfo descriptor_layout = {};
    descriptor_layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    descriptor_layout.pNext = NULL;
    descriptor_layout.bindingCount = 1;
    descriptor_layout.pBindings = &layout_binding;
                 
    VkResult result;

    result = vkCreateDescriptorSetLayout(device(), &descriptor_layout, NULL,
                                         &m_desc_layout);
    VK_CHECK(result);

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext = NULL;
    pPipelineLayoutCreateInfo.setLayoutCount = 1;
    pPipelineLayoutCreateInfo.pSetLayouts = &m_desc_layout;

    result = vkCreatePipelineLayout(device(), &pPipelineLayoutCreateInfo, NULL,
                                    &m_pipeline_layout);
    VK_CHECK(result);
}

// when you change the data, as in this callback, you must call redraw():
void sides_cb(Fl_Widget *o, void *p) {
  vk_shape_window *sw = (vk_shape_window *)p;
  sw->sides = int(((Fl_Slider *)o)->value());
  sw->wait_queue();
  sw->destroy_mesh();
  sw->prepare_mesh();
  sw->redraw();
}

int main(int argc, char **argv) {
    Fl::use_high_res_VK(1);

    Fl_Window window(300, 330);
  
    vk_shape_window sw(10, 10, 280, 280);

    Fl_Hor_Slider slider(50, 295, window.w()-60, 30, "Sides:");
    slider.align(FL_ALIGN_LEFT);
    slider.step(1);
    slider.bounds(3,40);

    window.resizable(&sw);
    slider.value(sw.sides);
    slider.callback(sides_cb,&sw);
    window.end();
    window.show(argc,argv);
        
    return Fl::run();
}
