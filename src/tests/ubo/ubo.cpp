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
 * http://www.apache.org/licenses/LICENSE-2.0
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

#include <tlVk/Mesh.h>
#include <tlVk/OffscreenBuffer.h>
#include <tlVk/Shader.h>

#define MAX_FRAMES_IN_FLIGHT 5

#include <tlCore/Size.h>
#include <tlCore/Vector.h>

#include <FL/platform.H>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Hor_Slider.H>
#include <FL/math.h>

#include <iostream>
#include <limits>
#include <FL/Fl_Vk_Window.H>
#include <FL/Fl_Vk_Utils.H>

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

//
// FBO shaders (mix two colors)
//
std::string fbo_vertex_shader_glsl = R"(
        #version 450
        layout(location = 0) in vec3 inPos;
        void main() {
            gl_Position = vec4(inPos, 1.0);
        }
)";

std::string fbo_frag_shader_glsl = R"(
#version 450

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0, std140) uniform UBO {
    vec3 redColor;
    vec3 blueColor;
} ubo;

void main() {
    outColor = vec4(ubo.redColor + ubo.blueColor, 1);
}
)";

class vk_shape_window : public Fl_Vk_Window
{
    void draw() FL_OVERRIDE;

public:
    int sides;
    vk_shape_window(int x, int y, int w, int h, const char* l = 0);
    vk_shape_window(int w, int h, const char* l = 0);

    const char* application_name() FL_OVERRIDE { return "vk_shape_textured"; };
    void prepare() FL_OVERRIDE;
    
    void prepare_pipeline(); // Main swapchain pipeline
    
    void destroy_resources() FL_OVERRIDE;

    void destroy_mesh();
    void prepare_mesh();

    uint32_t frame_counter = 0;

protected:
    //! This is for the fbo pipeline
    VkPipeline m_fbo_pipeline;
    VkPipelineLayout m_fbo_pipeline_layout;

    //! This is for swapchain pipeline
    VkPipelineLayout m_pipeline_layout;

    //! Memory for descriptor pools (need one for FBO shader UBO, and one for main shader texture)
    VkDescriptorPool m_fbo_desc_pool; // Pool for FBO shader's descriptor sets
    VkDescriptorPool m_main_desc_pool; // Pool for main shader's descriptor sets

    //! Describe texture bindings within desc. set
    VkDescriptorSetLayout m_main_desc_layout; // Layout for main shader's descriptor sets

    //! Actual data bound to shaders like texture or
    //! uniform buffers
    std::vector<VkDescriptorSet> m_main_desc_sets; // Per-frame descriptor sets for the main pipeline
    
private:
    void _init();

    // Pipelines and layouts are managed outside the per-frame draw loop
    void destroy_fbo_pipeline();
    void prepare_fbo_pipeline(); // Still need this, but called elsewhere

    void prepare_descriptor_layout(); // For the main shader (composition)
    void prepare_render_pass(); // Main swapchain render pass
    void prepare_shaders();

    // Descriptor pool/set preparation needs to be updated for per-frame sets
    void prepare_descriptor_pools(); // Create both pools
    void prepare_descriptor_sets(); // Allocate and initially update per-frame sets


    //! Shaders used in demo

    std::shared_ptr<tl::vlk::OffscreenBuffer> fbo;
    std::shared_ptr<tl::vlk::VBO> fbo_vbo;
    std::shared_ptr<tl::vlk::VAO> fbo_vao;
    std::shared_ptr<tl::vlk::Shader> fboShader; // This shader needs its own descriptor set(s) and layout for the UBO

    std::shared_ptr<tl::vlk::VBO> vbo; // Geometry for the main composition pass (screen quad)
    std::shared_ptr<tl::vlk::VAO> vao; // VAO for the main composition pass
    std::shared_ptr<tl::vlk::Shader> shader; // Main composition shader
};

void vk_shape_window::_init()
{
    mode(FL_RGB | FL_DOUBLE | FL_ALPHA | FL_DEPTH | FL_STENCIL);
    sides = 3;
    // Turn on validations
    m_validate = true;
    m_fbo_pipeline = VK_NULL_HANDLE;
    m_fbo_pipeline_layout = VK_NULL_HANDLE;
    m_pipeline_layout = VK_NULL_HANDLE;
    m_fbo_desc_pool = VK_NULL_HANDLE;
    m_main_desc_pool = VK_NULL_HANDLE;
    m_main_desc_layout = VK_NULL_HANDLE;
}

vk_shape_window::vk_shape_window(int x, int y, int w, int h, const char* l) :
    Fl_Vk_Window(x, y, w, h, l)
{
    _init();
}

vk_shape_window::vk_shape_window(int w, int h, const char* l) :
    Fl_Vk_Window(w, h, l)
{
    _init();
}

void vk_shape_window::prepare_mesh()
{    using namespace tl;

    geom::TriangleMesh2 mesh = geom::box(math::Box2f(-.5F, -0.5F, 1.F, 1.F));
    const size_t numTriangles = mesh.triangles.size();
    if (!fbo_vbo || (fbo_vbo && fbo_vbo->getSize() != numTriangles * 3))
    {
        fbo_vbo =
            vlk::VBO::create(numTriangles * 3, vlk::VBOType::Pos2_F32_UV_U16);
        fbo_vao.reset();
    }
    if (fbo_vbo)
    {
        fbo_vbo->copy(convert(mesh, vlk::VBOType::Pos2_F32_UV_U16));
    }

    if (!fbo_vao && fbo_vbo)
    {
        fbo_vao = vlk::VAO::create(ctx);
        fbo_vao->upload(fbo_vbo->getData());
    }

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

    {
        geom::TriangleMesh3 mesh;
        geom::Triangle3 triangle;

        mesh.triangles.reserve(sides);
        for (int i = 0; i < sides; ++i)
        {
            triangle.v[0].v = 1 + i * 3;
            triangle.v[1].v = 2 + i * 3;
            triangle.v[2].v = 3 + i * 3;
            triangle.v[0].t = triangle.v[0].v;
            triangle.v[1].t = triangle.v[1].v;
            triangle.v[2].t = triangle.v[2].v;
            mesh.triangles.push_back(triangle);
        }
    
        for (const auto& vertex : vertices)
        {
            mesh.v.push_back(math::Vector3f(vertex.x, vertex.y, vertex.z));
            mesh.t.push_back(math::Vector2f(vertex.u, vertex.v));
        }
    
        if (!vbo || (vbo && vbo->getSize() != sides * 3))
        {
            vbo = vlk::VBO::create(sides * 3, vlk::VBOType::Pos3_F32_UV_U16);
            vao.reset();
        }
        if (vbo)
        {
            vbo->copy(convert(mesh, vlk::VBOType::Pos3_F32_UV_U16));
        }

        if (!vao && vbo)
        {
            vao = vlk::VAO::create(ctx);
            vao->upload(vbo->getData());
        }
    }
}

// m_depth (optionally) -> creates m_renderPass
void vk_shape_window::prepare_render_pass()
{
    bool has_depth = mode() & FL_DEPTH;
    bool has_stencil = mode() & FL_STENCIL;

    // Main swapchain render pass setup remains the same
    VkAttachmentDescription attachments[2];
    attachments[0] = VkAttachmentDescription();
    attachments[0].format = format();
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear color attachment
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Store for presentation
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Start undefined
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Final layout for presentation

    attachments[1] = VkAttachmentDescription(); // For depth/stencil if enabled


    VkAttachmentReference color_reference = {};
    color_reference.attachment = 0;
    color_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL; // Layout during subpass

    VkAttachmentReference depth_reference = {};
    depth_reference.attachment = 1;
    depth_reference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // Layout during subpass

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
        attachments[1].format = m_depth.format; // Assuming m_depth is managed by Fl_Vk_Window
        attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Clear depth/stencil
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
        attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Start undefined
        attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; // Final layout

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
    rp_info.dependencyCount = 0; // No subpass dependencies needed for a single subpass
    rp_info.pDependencies = NULL;

    VkResult result;
    result = vkCreateRenderPass(device(), &rp_info, NULL, &m_renderPass);
    VK_CHECK(result);
}


void vk_shape_window::prepare_shaders()
{
    if (!fboShader)
        fboShader = tl::vlk::Shader::create(ctx, fbo_vertex_shader_glsl,
                                           fbo_frag_shader_glsl);

    if (!shader)
        shader = tl::vlk::Shader::create(ctx, vertex_shader_glsl,
                                        frag_shader_glsl);
}

void vk_shape_window::destroy_fbo_pipeline()
{
    if (m_fbo_pipeline_layout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device(), m_fbo_pipeline_layout, nullptr);
        m_fbo_pipeline_layout = VK_NULL_HANDLE;
    }

    if (m_fbo_pipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(device(), m_fbo_pipeline, nullptr);
        m_fbo_pipeline = VK_NULL_HANDLE;
    }
}

void vk_shape_window::prepare_fbo_pipeline() {
    // Ensure FBO and its render pass are valid before creating the pipeline
    if (!fbo || fbo->getRenderPass() == VK_NULL_HANDLE) {
        // Handle error: FBO not created or invalid
        fprintf(stderr, "Error: FBO or its render pass is invalid when preparing FBO pipeline.\n");
        return;
    }

    // Destroy existing pipeline and layout if they exist
    destroy_fbo_pipeline();

    VkGraphicsPipelineCreateInfo pipeline;
    // VkPipelineCacheCreateInfo pipelineCacheCreateInfo; // Not strictly needed for a single pipeline

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

    // UBO update logic remains here, but the descriptor set update
    // for the FBO shader should ideally happen once per frame too,
    // similar to the main shader, if the UBO data changes per frame.
    // For this example, we keep it tied to pipeline creation,
    // assuming the UBO data might be static relative to FBO properties.
    // If UBO changes per frame, FBO shader needs per-frame descriptor sets.
    frame_counter += 10;
    float b = (frame_counter % 255) / 255.F;
    float r = 1.F - b;

    using namespace tl;
    struct UBO
    {
        math::Vector3f redColor; // Match shader UBO definition
        math::Vector3f blueColor;
    };

    UBO ubo;
    ubo.redColor = math::Vector3f(r, 0, 0); // Correct order
    ubo.blueColor = math::Vector3f(0, 0, b);

    // Assuming fboShader manages its own descriptor sets
    fboShader->setUniform("ubo", ubo);
    // createDescriptorSet for fboShader should ideally happen outside prepare_fbo_pipeline,
    // possibly in prepare_descriptor_sets if per-frame, or in prepare if static.
    // Let's assume fboShader->createDescriptorSet() creates a static set for now.
    if (fboShader->getDescriptorSet() == VK_NULL_HANDLE)
        fboShader->createDescriptorSet();


    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext = NULL;
    pPipelineLayoutCreateInfo.setLayoutCount = 1;
    pPipelineLayoutCreateInfo.pSetLayouts = &fboShader->getDescriptorSetLayout(); // Use FBO shader's layout

    result = vkCreatePipelineLayout(device(), &pPipelineLayoutCreateInfo,
                                    NULL,
                                    &m_fbo_pipeline_layout);
    VK_CHECK(result);


    memset(&pipeline, 0, sizeof(pipeline));
    pipeline.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipeline.layout = m_fbo_pipeline_layout; // Use the new FBO pipeline layout

    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vi.pNext = NULL;
    vi.vertexBindingDescriptionCount = 1;
    vi.pVertexBindingDescriptions = fbo_vbo->getBindingDescription(); // Use FBO mesh binding
    vi.vertexAttributeDescriptionCount = fbo_vbo->getAttributes().size();
    vi.pVertexAttributeDescriptions = fbo_vbo->getAttributes().data();

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

    bool has_depth = fbo->hasDepth(); // Check FBO depth
    bool has_stencil = fbo->hasStencil(); // Check FBO stencil

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
    shaderStages[0].module = fboShader->getVertex(); // Use FBO vertex shader
    shaderStages[0].pName = "main";

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = fboShader->getFragment(); // Use FBO fragment shader
    shaderStages[1].pName = "main";

    pipeline.pVertexInputState = &vi;
    pipeline.pInputAssemblyState = &ia;
    pipeline.pRasterizationState = &rs;
    pipeline.pColorBlendState = &cb;
    pipeline.pMultisampleState = &ms;
    pipeline.pViewportState = &vp;
    pipeline.pDepthStencilState = &ds;
    pipeline.pStages = shaderStages;
    pipeline.renderPass = fbo->getRenderPass(); // Use FBO's render pass
    pipeline.pDynamicState = &dynamicState;


    // Create a temporary pipeline cache
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VkPipelineCache pipelineCache;
    result = vkCreatePipelineCache(device(), &pipelineCacheCreateInfo, NULL, &pipelineCache);
    VK_CHECK(result);


    result = vkCreateGraphicsPipelines(device(), pipelineCache, 1,
                                       &pipeline, NULL, &m_fbo_pipeline);
    VK_CHECK(result);

    // Destroy the temporary pipeline cache
    vkDestroyPipelineCache(device(), pipelineCache, NULL);
}


void vk_shape_window::prepare_pipeline() {
    // Ensure main render pass is valid
     if (m_renderPass == VK_NULL_HANDLE) {
        // Handle error: Main render pass not created or invalid
        fprintf(stderr, "Error: Main render pass is invalid when preparing main pipeline.\n");
        return;
    }

    // Destroy existing pipeline if it exists
    if (m_pipeline != VK_NULL_HANDLE) {
         vkDestroyPipeline(device(), m_pipeline, nullptr);
         m_pipeline = VK_NULL_HANDLE;
     }


    VkGraphicsPipelineCreateInfo pipeline;

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
    pipeline.layout = m_pipeline_layout; // Use the main pipeline layout

    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vi.pNext = NULL;
    vi.vertexBindingDescriptionCount = 1;
    vi.pVertexBindingDescriptions = vbo->getBindingDescription(); // Use main mesh binding
    vi.vertexAttributeDescriptionCount = vbo->getAttributes().size();
    vi.pVertexAttributeDescriptions = vbo->getAttributes().data();

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

    bool has_depth = mode() & FL_DEPTH; // Check window depth
    bool has_stencil = mode() & FL_STENCIL; // Check window stencil

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
    shaderStages[0].module = shader->getVertex(); // Use main vertex shader
    shaderStages[0].pName = "main";

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = shader->getFragment(); // Use main fragment shader
    shaderStages[1].pName = "main";

    pipeline.pVertexInputState = &vi;
    pipeline.pInputAssemblyState = &ia;
    pipeline.pRasterizationState = &rs;
    pipeline.pColorBlendState = &cb;
    pipeline.pMultisampleState = &ms;
    pipeline.pViewportState = &vp;
    pipeline.pDepthStencilState = &ds;
    pipeline.pStages = shaderStages;
    pipeline.renderPass = m_renderPass; // Use main render pass (swapchain)
    pipeline.pDynamicState = &dynamicState;

    // Create a temporary pipeline cache
    VkPipelineCacheCreateInfo pipelineCacheCreateInfo{};
    pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
    VkPipelineCache pipelineCache;
    result = vkCreatePipelineCache(device(), &pipelineCacheCreateInfo, NULL, &pipelineCache);
    VK_CHECK(result);


    result = vkCreateGraphicsPipelines(device(), pipelineCache, 1,
                                       &pipeline, NULL, &m_pipeline);
    VK_CHECK(result);

    // Destroy the temporary pipeline cache
    vkDestroyPipelineCache(device(), pipelineCache, NULL);
}


void vk_shape_window::prepare_descriptor_pools() {
    // Create descriptor pool for FBO shader (UBO)
    VkDescriptorPoolSize fbo_pool_size = {};
    fbo_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    fbo_pool_size.descriptorCount = MAX_FRAMES_IN_FLIGHT; // If FBO UBO changes per frame

    VkDescriptorPoolCreateInfo fbo_pool_info = {};
    fbo_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    fbo_pool_info.pNext = NULL;
    fbo_pool_info.maxSets = MAX_FRAMES_IN_FLIGHT; // One set per frame for the FBO shader
    fbo_pool_info.poolSizeCount = 1;
    fbo_pool_info.pPoolSizes = &fbo_pool_size;

    VkResult result = vkCreateDescriptorPool(device(), &fbo_pool_info, NULL, &m_fbo_desc_pool);
    VK_CHECK(result);

    // Create descriptor pool for main shader (Combined Image Sampler)
    VkDescriptorPoolSize main_pool_size = {};
    main_pool_size.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    main_pool_size.descriptorCount = MAX_FRAMES_IN_FLIGHT; // One texture sampler per frame

    VkDescriptorPoolCreateInfo main_pool_info = {};
    main_pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    main_pool_info.pNext = NULL;
    main_pool_info.maxSets = MAX_FRAMES_IN_FLIGHT; // One set per frame for the main shader
    main_pool_info.poolSizeCount = 1;
    main_pool_info.pPoolSizes = &main_pool_size;

    result = vkCreateDescriptorPool(device(), &main_pool_info, NULL, &m_main_desc_pool);
    VK_CHECK(result);
}

void vk_shape_window::prepare_descriptor_sets() {
    VkResult result;
    
    // Prepare descriptor sets for the main shader (composition)
    m_main_desc_sets.resize(MAX_FRAMES_IN_FLIGHT);
    std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, m_main_desc_layout); // Use the main layout

    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.pNext = NULL;
    alloc_info.descriptorPool = m_main_desc_pool; // Allocate from the main pool
    alloc_info.descriptorSetCount = static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
    alloc_info.pSetLayouts = layouts.data();

    result = vkAllocateDescriptorSets(device(), &alloc_info, m_main_desc_sets.data());
    VK_CHECK(result);

    // You would also allocate descriptor sets for the FBO shader here if its UBO changes per frame
    // fboShader->createDescriptorSets(MAX_FRAMES_IN_FLIGHT, m_fbo_desc_pool); // Example

    // Initial update of descriptor sets.
    // You might update with placeholder data or the initial state here.
    // The actual FBO texture reference update happens in the draw loop.
    // For the FBO shader's UBO, if it's per-frame, you would update its sets here initially too.
}

void vk_shape_window::prepare()
{
    // FBO and its pipeline creation/recreation should be tied to size/option changes,
    // happening *after* vkDeviceWaitIdle(). We assume this is handled elsewhere.

    prepare_mesh();
    prepare_shaders(); // Compile shaders
    prepare_render_pass(); // Main swapchain render pass

    // Prepare layouts and pools *before* creating pipelines and allocating sets
    prepare_descriptor_layout(); // Main shader layout
    prepare_descriptor_pools(); // Both FBO and main pools

    // Prepare pipelines *after* render passes, layouts, and shaders are ready
    // FBO pipeline creation should happen AFTER FBO render pass is created (within OffscreenBuffer)
    // The call to prepare_fbo_pipeline() should be in a handler for FBO recreation.
    // prepare_fbo_pipeline(); // REMOVED FROM HERE

    prepare_pipeline(); // Main swapchain pipeline

    // Prepare descriptor sets *after* pools and layouts are ready
    prepare_descriptor_sets(); // Allocate and initially update sets

     // Initialize fbo here or in a dedicated fbo setup function
}

void vk_shape_window::draw() {
    if (!shown() || w() <= 0 || h() <= 0)
        return;

    using namespace tl;

    // --- Per-frame synchronization and Command Buffer acquisition ---
    // This is handled by Fl_Vk_Window::vk_draw_begin() and swap_buffers().
    // vk_draw_begin() ensures we have the command buffer 'cmd', waited on its fence,
    // reset it, and started recording. It also handles swapchain image acquisition
    // and initial swapchain image layout transition.

    VkCommandBuffer cmd = getCurrentCommandBuffer(); // Get the command buffer started by vk_draw_begin()
    vkCmdEndRenderPass(cmd);  // end the clear screen command pass.
    
    uint32_t currentFrameIndex = m_currentFrameIndex; // Get the current frame index

    
    math::Size2i renderSize(4096, 2980);
    vlk::OffscreenBufferOptions options;
    options.colorType = vlk::offscreenColorDefault;

    // The doCreate function checks that renderSize did not change and
    // that options (like a depth or stencil added have not changed).
    if (vlk::doCreate(fbo, renderSize, options))
    {
        fbo = vlk::OffscreenBuffer::create(ctx, renderSize, options);
        destroy_fbo_pipeline();
        prepare_fbo_pipeline();
    }

    // --- First Render Pass: Render to FBO ---

    // Clear yellow (assuming this clear value is for the FBO)
    VkClearValue fboClearValues[1];
    fboClearValues[0].color = { 1.f, 1.f, 0.f, 1.f }; // Clear color for the FBO

    VkRenderPassBeginInfo fboRpBegin{};
    fboRpBegin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    fboRpBegin.renderPass = fbo->getRenderPass(); // Use the FBO's render pass
    // Use the FBO's framebuffer for THIS frame if using per-frame FBO attachments,
    // otherwise use the static one if fbo->getFramebuffer() is static.
    fboRpBegin.framebuffer = fbo->getFramebuffer(/*currentFrameIndex*/); // Pass frame index if needed
    fboRpBegin.renderArea.offset = {0, 0}; // Assuming render area starts at 0,0
    fboRpBegin.renderArea.extent = fbo->getExtent(); // Use FBO extent
    fboRpBegin.clearValueCount = 1;
    fboRpBegin.pClearValues = fboClearValues;

    // Begin the first render pass instance within the single command buffer
    vkCmdBeginRenderPass(cmd, &fboRpBegin, VK_SUBPASS_CONTENTS_INLINE);

    // Bind the FBO pipeline (created/managed outside this draw loop)
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_fbo_pipeline);

    // Update and Bind descriptor sets for the FBO render pass.
    // If fboShader uses resources that change per frame (like source video texture or UBO),
    // update and bind the set for currentFrameIndex.
    // Assuming UBO updates here and fboShader manages per-frame sets internally:
    frame_counter += 10;
    float b = (frame_counter % 255) / 255.F;
    float r = 1.F - b;
     struct UBO { math::Vector3f redColor; math::Vector3f blueColor; };
     UBO ubo; ubo.redColor = math::Vector3f(r, 0, 0); ubo.blueColor = math::Vector3f(0, 0, b);
     
     // Update UBO for this frame/set (this updates fboShader's DescriptorSet).
     fboShader->setUniform("ubo", ubo);

     // WAS:
     // fboShader->setUniform("ubo", ubo, currentFrameIndex); // Update UBO for this frame/set
     // fboShader->updateDescriptorSet(device(), currentFrameIndex); // Update the set for this frame

    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_fbo_pipeline_layout, 0, 1,
                            //&fboShader->getDescriptorSet(currentFrameIndex), // Bind the set for THIS frame
                            &fboShader->getDescriptorSet(), // Bind the set for THIS frame
                            0, nullptr);

    fbo->setupViewportAndScissor(cmd);

    // Draw commands here (draw the content into the FBO)
    fbo_vao->draw(cmd, fbo_vbo);

    vkCmdEndRenderPass(cmd);

    // --- Barrier: Transition FBO Color Attachment from Color Write to Shader Read ---
    // This barrier must occur *after* the FBO render pass ends (writes are finished)
    // and *before* the second render pass begins (reads in shader start).
    // It's recorded into the SAME command buffer.

    // Assuming fbo->transitionToShaderRead(cmd) internally calls vkCmdPipelineBarrier
    // with oldLayout=COLOR_ATTACHMENT_OPTIMAL, newLayout=SHADER_READ_ONLY_OPTIMAL,
    // srcStageMask=COLOR_ATTACHMENT_OUTPUT_BIT, dstStageMask=FRAGMENT_SHADER_BIT,
    // srcAccessMask=COLOR_ATTACHMENT_WRITE_BIT, dstAccessMask=SHADER_READ_BIT.
    fbo->transitionToShaderRead(cmd);


    // --- Second Render Pass: Render to Swapchain (Composition) ---
    begin_render_pass();

    // Bind the main composition pipeline (created/managed outside this draw loop)
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

    // --- Update Descriptor Set for the SECOND pass (Composition) ---
    // This updates the descriptor set for the CURRENT frame index on the CPU.
    VkDescriptorImageInfo tex_descs = {}; // No array needed for a single write
    // Get the sampler and image view from the FBO (these should be valid)
    tex_descs.sampler = fbo->getSampler();
    tex_descs.imageView = fbo->getImageView();
    // The layout MUST be the one the image is in *at the time of binding*
    // which is VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL due to the barrier recorded earlier.
    tex_descs.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    // Update the descriptor set for the CURRENT frame index from the per-frame vector
    write.dstSet = m_main_desc_sets[currentFrameIndex]; // Use the set for THIS frame
    write.dstBinding = 0; // Your binding index for the FBO texture in the composition shader
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = &tex_descs;

    // Call vkUpdateDescriptorSets on the CPU. This is safe because vk_draw_begin() waited on the fence.
    vkUpdateDescriptorSets(device(), 1, &write, 0, NULL);

    // --- Bind Descriptor Set for the SECOND pass ---
    // Record the command to bind the descriptor set for the CURRENT frame index
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_pipeline_layout, 0, 1,
                            &m_main_desc_sets[currentFrameIndex], // Bind the set for THIS frame
                            0, nullptr);

    // Set dynamic state (viewport, scissor) for the composition pass
    VkViewport viewport = {};
    viewport.width = static_cast<float>(w());
    viewport.height = static_cast<float>(h());
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.extent.width = w();
    scissor.extent.height = h();
    vkCmdSetScissor(cmd, 0, 1, &scissor);

    // Draw calls for the composition geometry (e.g., a screen-filling quad)
    vao->draw(cmd, vbo);
}


void vk_shape_window::destroy_mesh()
{
    vao.reset();
    vbo.reset();
    fbo_vao.reset();
    fbo_vbo.reset();
}

void vk_shape_window::destroy_resources()
{
    if (device() == VK_NULL_HANDLE)
        return;

    // Wait for the device to be idle before destroying resources
    vkDeviceWaitIdle(device());

    destroy_mesh();

    // Destroy descriptor sets *before* destroying pools
    m_main_desc_sets.clear(); // This doesn't free the sets, just clears the vector

    // Descriptor sets allocated from pools must be freed or the pool reset
    if (m_main_desc_pool != VK_NULL_HANDLE) {
         vkResetDescriptorPool(device(), m_main_desc_pool, 0); // Reset main pool
         vkDestroyDescriptorPool(device(), m_main_desc_pool, nullptr);
         m_main_desc_pool = VK_NULL_HANDLE;
     }
    
    if (m_fbo_desc_pool != VK_NULL_HANDLE) {
         vkResetDescriptorPool(device(), m_fbo_desc_pool, 0); // Reset FBO pool
         vkDestroyDescriptorPool(device(), m_fbo_desc_pool, nullptr);
         m_fbo_desc_pool = VK_NULL_HANDLE;
     }

    if (m_main_desc_layout != VK_NULL_HANDLE)
    {
        vkDestroyDescriptorSetLayout(device(), m_main_desc_layout, nullptr);
        m_main_desc_layout = VK_NULL_HANDLE;
    }

    if (m_pipeline_layout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(device(), m_pipeline_layout, nullptr);
        m_pipeline_layout = VK_NULL_HANDLE;
    }

    if (m_pipeline != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(device(), m_pipeline, nullptr);
        m_pipeline = VK_NULL_HANDLE;
    }

    // Destroy FBO pipeline and layout (handled by destroy_fbo_pipeline)
    destroy_fbo_pipeline();

    shader.reset(); // Main shader module
    fboShader.reset(); // FBO shader module

    // Destroy the FBO and its resources (includes the FBO render pass)
    fbo.reset(); // Assuming OffscreenBuffer destructor handles destruction
}


void vk_shape_window::prepare_descriptor_layout()
{
    // Layout for the main shader (composition) - uses a combined image sampler (the FBO texture)
    VkDescriptorSetLayoutBinding main_layout_binding = {};
    main_layout_binding.binding = 0;
    main_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    main_layout_binding.descriptorCount = 1;
    main_layout_binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT; // Used in fragment shader
    main_layout_binding.pImmutableSamplers = NULL;

    VkDescriptorSetLayoutCreateInfo main_descriptor_layout_info = {};
    main_descriptor_layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    main_descriptor_layout_info.pNext = NULL;
    main_descriptor_layout_info.bindingCount = 1;
    main_descriptor_layout_info.pBindings = &main_layout_binding;

    VkResult result = vkCreateDescriptorSetLayout(device(), &main_descriptor_layout_info, NULL,
                                         &m_main_desc_layout);
    VK_CHECK(result);

    // Main pipeline layout references the main descriptor set layout
    VkPipelineLayoutCreateInfo main_pipeline_layout_info = {};
    main_pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    main_pipeline_layout_info.pNext = NULL;
    main_pipeline_layout_info.setLayoutCount = 1;
    main_pipeline_layout_info.pSetLayouts = &m_main_desc_layout;

    result = vkCreatePipelineLayout(device(), &main_pipeline_layout_info, NULL,
                                    &m_pipeline_layout);
    VK_CHECK(result);

    // --- FBO Shader Descriptor Set Layout ---
    // The FBO shader uses a UBO. Its layout should be handled by the fboShader class.
    // Assuming fboShader->getDescriptorSetLayout() provides the layout for the UBO.
    // This layout needs to be used when creating the m_fbo_pipeline_layout.
    // This is already handled correctly in prepare_fbo_pipeline().
}

// when you change the data, as in this callback, you must call redraw():
void sides_cb(Fl_Widget *o, void *p) {
  vk_shape_window *sw = (vk_shape_window *)p;
  sw->sides = int(((Fl_Slider *)o)->value());
  sw->wait_device(); // Wait for device idle before recreating mesh/pipeline
  sw->destroy_mesh(); // Destroy old mesh
  // If FBO parameters changed due to sides (unlikely here), destroy FBO and FBO pipeline here too
  // sw->fbo.reset();
  // sw->destroy_fbo_pipeline();
  sw->prepare_mesh(); // Create new mesh
  // If FBO recreated, recreate FBO and FBO pipeline here
  // sw->fbo = vlk::OffscreenBuffer::create(...);
  // sw->prepare_fbo_pipeline();
  sw->prepare_pipeline(); // Recreate main pipeline if vertex format changed (it does with mesh)
  sw->redraw();
}

void change_ubo_cb(void *p) {
  vk_shape_window *sw = (vk_shape_window *)p;
  // Redrawing will trigger the draw() call, which updates the UBO and descriptor set
  sw->redraw();
  Fl::repeat_timeout(0.005, (Fl_Timeout_Handler)change_ubo_cb, sw);
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

    Fl::add_timeout(0.005, (Fl_Timeout_Handler)change_ubo_cb, &sw);

    return Fl::run();
}
