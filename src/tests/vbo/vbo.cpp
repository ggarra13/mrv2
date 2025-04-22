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

#include <tlVk/Mesh.h>

#include <FL/platform.H>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Hor_Slider.H>
#include <FL/Fl_Vk_Window.H>
#include <FL/Fl_Vk_Utils.H>
#include <FL/math.h>

#include <iostream>
#include <limits>

class vk_shape_window : public Fl_Vk_Window {
    void draw() FL_OVERRIDE;
public:
    int sides;
    vk_shape_window(int x,int y,int w,int h,const char *l=0);
    vk_shape_window(int w,int h,const char *l=0);
    
    const char* application_name() FL_OVERRIDE { return "vk_shape_textured"; };
    void prepare() FL_OVERRIDE;
    void destroy_resources() FL_OVERRIDE;


    void destroy_mesh();
    void prepare_mesh();
    
protected:
    //! Shaders used in demo
    VkShaderModule m_vert_shader_module;
    VkShaderModule m_frag_shader_module;
    uint32_t frame_counter = 0;
    
    //! This is for holding one texture for the shader.
    Fl_Vk_Texture  m_texture;

    //! Memory for descriptor sets
    VkDescriptorPool      m_desc_pool;

    //! Describe texture bindings whithin desc. set  
    VkDescriptorSetLayout m_desc_layout;

    //! Actual data bound to shaders like texture or
    //! uniform buffers
    VkDescriptorSet       m_desc_set; 
    
    void update_texture();
    
private:
    void _init();

    void prepare_textures();
    void prepare_descriptor_layout();
    void prepare_render_pass();
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

    std::shared_ptr<tl::vk::VBO> vbo;
    std::shared_ptr<tl::vk::VAO> vao;
};


static void texture_cb(vk_shape_window* w)
{
    w->redraw();
    Fl::repeat_timeout(1.0/60.0, (Fl_Timeout_Handler)texture_cb, w);
}

void vk_shape_window::_init()
{
    mode(FL_RGB | FL_DOUBLE | FL_ALPHA | FL_STENCIL);
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

void vk_shape_window::prepare_texture_image(const uint32_t *tex_colors,
                                            Fl_Vk_Texture* tex_obj,
                                            VkImageTiling tiling,
                                            VkImageUsageFlags usage,
                                            VkFlags required_props) {
    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    const int32_t tex_width = 2;
    const int32_t tex_height = 2;
    VkResult result;
    bool pass;

    tex_obj->width = tex_width;
    tex_obj->height = tex_height;

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

    result = vkCreateImage(device(), &image_create_info, NULL, &tex_obj->image);
    VK_CHECK(result);

    vkGetImageMemoryRequirements(device(), tex_obj->image, &m_mem_reqs);

    mem_alloc.allocationSize = m_mem_reqs.size;
    mem_alloc.memoryTypeIndex = findMemoryType(gpu(),
                                               m_mem_reqs.memoryTypeBits,
                                               required_props);

    /* allocate memory */
    result = vkAllocateMemory(device(), &mem_alloc, NULL, &tex_obj->mem);
    VK_CHECK(result);

    /* bind memory */
    result = vkBindImageMemory(device(), tex_obj->image, tex_obj->mem, 0);
    VK_CHECK(result);

    if (required_props & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
        VkImageSubresource subres = {};
        subres.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        subres.mipLevel = 0;
        subres.arrayLayer = 0;
        VkSubresourceLayout layout;
        void *data;
        int32_t x, y;

        vkGetImageSubresourceLayout(device(), tex_obj->image, &subres,
                                    &layout);

        result = vkMapMemory(device(), tex_obj->mem, 0,
                             mem_alloc.allocationSize, 0, &data);
        VK_CHECK(result);

        // Tile the texture over tex_height and tex_width
        for (y = 0; y < tex_height; y++) {
            uint32_t *row = (uint32_t *)((char *)data + layout.rowPitch * y);
            for (x = 0; x < tex_width; x++)
                row[x] = tex_colors[(x & 1) ^ (y & 1)];
        }

        vkUnmapMemory(device(), tex_obj->mem);
    }

    
    // Initial transition to shader-readable layout
    set_image_layout(device(), commandPool(), queue(), tex_obj->image,
                     VK_IMAGE_ASPECT_COLOR_BIT,
                     VK_IMAGE_LAYOUT_UNDEFINED,   // Initial layout
                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                     0, // No previous access
                     VK_PIPELINE_STAGE_HOST_BIT, // Host stage
                     VK_ACCESS_SHADER_READ_BIT,  // Shader read
                     VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}


void vk_shape_window::prepare_textures()
{
    VkResult result;
    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    const uint32_t tex_colors[2] = {
        // B G R A     B G R A
        0xffff0000, 0xff00ff00  // Red, Green
            };

    // Query if image supports texture format
    VkFormatProperties props;
    vkGetPhysicalDeviceFormatProperties(gpu(), tex_format, &props);

    if (props.linearTilingFeatures &
         VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT) {
        // Device can texture using linear textures
        prepare_texture_image(
            tex_colors, &m_texture, VK_IMAGE_TILING_LINEAR,
            VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
            VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    } else {
        /* Can't support VK_FORMAT_B8G8R8A8_UNORM !? */
        Fl::fatal("No support for B8G8R8A8_UNORM as texture image format");
    }
        
    VkSamplerCreateInfo sampler_info = {};
    sampler_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    sampler_info.pNext = NULL;
    sampler_info.magFilter = VK_FILTER_NEAREST;
    sampler_info.minFilter = VK_FILTER_NEAREST;
    sampler_info.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
    sampler_info.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    sampler_info.mipLodBias = 0.0f;
    sampler_info.anisotropyEnable = VK_FALSE;
    sampler_info.maxAnisotropy = 1;
    sampler_info.compareOp = VK_COMPARE_OP_NEVER;
    sampler_info.minLod = 0.0f;
    sampler_info.maxLod = 0.0f;
    sampler_info.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
    sampler_info.unnormalizedCoordinates = VK_FALSE;

    VkImageViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view_info.pNext = NULL;
    view_info.image = m_texture.image;
    view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view_info.format = tex_format;
    view_info.components =
        {
            VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G,
            VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A,
        };
    view_info.subresourceRange = {VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1};
    view_info.flags = 0;

    result = vkCreateSampler(device(), &sampler_info, NULL,
                             &m_texture.sampler);
    VK_CHECK(result);

    result = vkCreateImageView(device(), &view_info, NULL, &m_texture.view);
    VK_CHECK(result);
}

void vk_shape_window::update_texture()
{
    // Transition to GENERAL for CPU writes
    set_image_layout(device(), commandPool(), queue(), m_texture.image,
                     VK_IMAGE_ASPECT_COLOR_BIT,
                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                     VK_IMAGE_LAYOUT_GENERAL,
                     VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                     VK_ACCESS_HOST_WRITE_BIT, VK_PIPELINE_STAGE_HOST_BIT);

    void* data;
    vkMapMemory(device(), m_texture.mem, 0, m_mem_reqs.size, 0, &data);
    
    uint32_t* pixels = (uint32_t*)data;
    uint8_t intensity = (frame_counter++ % 255);
    pixels[0] = (intensity << 16) | 0xFF;        // Red
    pixels[1] = (intensity << 8) | 0xFF;         // Green
    pixels[2] = (intensity) | 0xFF;              // Blue
    pixels[3] = ((255 - intensity) << 16) | 0xFF;// Inverted Red
    
    vkUnmapMemory(device(), m_texture.mem);

    // Transition back to SHADER_READ_ONLY_OPTIMAL
    set_image_layout(device(), commandPool(), queue(),
                     m_texture.image, VK_IMAGE_ASPECT_COLOR_BIT,
                     VK_IMAGE_LAYOUT_GENERAL,
                     VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                     VK_ACCESS_HOST_WRITE_BIT, VK_PIPELINE_STAGE_HOST_BIT,
                     VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
}


void vk_shape_window::prepare_mesh()
{
    using namespace tl;

    vbo.reset();
    vao.reset();

    const size_t numTriangles = sides;
    geom::TriangleMesh2 mesh = geom::box(math::Box2f(-0.5F, -0.5F, 1.0F, 1.0F));
    
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

    vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
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

    result = vkCreatePipelineCache(device(), &pipelineCacheCreateInfo, NULL,
                                   &pipelineCache());
    VK_CHECK(result);
    result = vkCreateGraphicsPipelines(device(), pipelineCache(), 1,
                                       &pipeline, NULL, &m_pipeline);
    VK_CHECK(result);

    vkDestroyPipelineCache(device(), pipelineCache(), NULL);
    pipelineCache() = VK_NULL_HANDLE;

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
    VkDescriptorImageInfo tex_descs[1];
    VkResult result;

    VkDescriptorSetAllocateInfo alloc_info = {};
    alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    alloc_info.pNext = NULL;
    alloc_info.descriptorPool = m_desc_pool;
    alloc_info.descriptorSetCount = 1;
    alloc_info.pSetLayouts = &m_desc_layout;
        
    result = vkAllocateDescriptorSets(device(), &alloc_info, &m_desc_set);
    VK_CHECK(result);

    memset(&tex_descs, 0, sizeof(tex_descs));
    tex_descs[0].sampler = m_texture.sampler;
    tex_descs[0].imageView = m_texture.view;
    tex_descs[0].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.dstSet = m_desc_set;
    write.descriptorCount = 1;
    write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    write.pImageInfo = tex_descs;

    vkUpdateDescriptorSets(device(), 1, &write, 0, NULL);
}

void vk_shape_window::prepare()
{
    prepare_textures();
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

    update_texture();

    VkCommandBuffer cmd = getCurrentCommandBuffer();
    if (!m_swapchain || !cmd || !isFrameActive()) {
        return;
    }

    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);
    vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_pipeline_layout, 0, 1, &m_desc_set, 0, nullptr);

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

    vao->draw(cmd, vbo);
    vkCmdEndRenderPass(cmd);
}

void vk_shape_window::destroy_mesh()
{
    vao.reset();
    vbo.reset();
}

void vk_shape_window::destroy_resources()
{
    if (device() == VK_NULL_HANDLE)
        return;

    destroy_mesh();

    m_texture.destroy(device());
    
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
