
//#define ANIMATE 1
#define BAKE_JOINTS 1

#define LOG_STATUS(x) std::cerr << x << std::endl;
#define _(x) x

// #include "USDProcessSkeletonRoot.h"  // \@todo: do deformation in compute shader
#include "USDCollectTextures.h"
#include "USDRenderEngine.h"
#include "USDRender.h"
#include "USDRenderShadersBinary.h"
#include "USDTextureSlots.h"

#include <tlCore/Context.h>

#include <pxr/pxr.h>

// math primitives
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3f.h>

// diagnostics
#include <pxr/base/tf/diagnostic.h>
#include <pxr/base/tf/token.h>
#include <pxr/base/tf/stringUtils.h>

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/timeCode.h>
#include <pxr/usd/usd/primRange.h>

#include <pxr/usd/usdUtils/pipeline.h>

#include <pxr/usdImaging/usdAppUtils/api.h>
#include <pxr/usdImaging/usdAppUtils/camera.h>
#include <pxr/usdImaging/usdAppUtils/frameRecorder.h>

#include <pxr/imaging/hd/renderBuffer.h>
#include <pxr/imaging/hdSt/hioConversions.h>
#include <pxr/imaging/hdSt/textureUtils.h>
#include <pxr/imaging/hdx/tokens.h>
#include <pxr/imaging/hdx/types.h>


#include <tlVk/OffscreenBuffer.h>
#include <tlVk/Shader.h>

#include <tlCore/Image.h>
#include <tlCore/Mesh.h>
#include <tlCore/Path.h>
#include <tlCore/StringFormat.h>

#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfArray.h>
#include <OpenEXR/ImfHeader.h>

#include <FL/platform.H>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/math.h>

#include <FL/Fl_Vk_Window.H>
#include <FL/Fl_Vk_Utils.H>

#include <FL/vk_enum_string_helper.h>

#include <memory>
#include <iostream>
#include <string>
#include <unordered_map>

using namespace PXR_NS;

void saveHalfRGB(const char* filename,
                 const std::shared_ptr<tl::image::Image> image) {

    using namespace Imf;
    using namespace Imath;

    try {
        // 3. Define the header and create the output file
        // WRITE_RGBA is the default, but you can specify WRITE_RGB if preferred

        int width = image->getWidth();
        int height = image->getHeight();
        
        // 1. Create a buffer of Rgba pixels (which use the 'half' type internally)
        Array2D<Rgba> pixels(height, width);

        half* data = reinterpret_cast<half*>(image->getData());
        
        // 2. Fill the buffer with some sample data
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                Rgba& p = pixels[y][x];
                p.r = *data++; // Convert float to half
                p.g = *data++;
                p.b = *data++;
                p.a = *data++;
            }
        }
    
        Header header(width, height);
        RgbaOutputFile file(filename, header, WRITE_RGBA);

        // 4. Set the frame buffer and write to disk
        file.setFrameBuffer(&pixels[0][0], 1, width);
        file.writePixels(height);
        
        //std::cout << "Image saved successfully to " << filename << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

using namespace tl;

class usd_window : public Fl_Vk_Window
{
public:
    usd_window(int X, int Y, int W, int H);

    void draw() override;
    void hide() override;

    void flush() override;

    void nextTimeCode();
    void setUSDFile(const std::string&);

public:
    //! Vulkan creation function.
    void prepare() FL_OVERRIDE;

    //! Vulkan destruction function.
    void destroy() FL_OVERRIDE;

    //! Vulkan color space negotiation.
    void init_colorspace() FL_OVERRIDE;

protected:
    void prepare_render_pass();
    void prepare_shaders();
    void prepare_pipeline_layout();
    void prepare_pipeline();
    
    math::Matrix4x4f _createTexturedRectangle();
    
    TLRENDER_PRIVATE();
};

using namespace tl;

struct usd_window::Private
{
    // Capture information
    bool pendingReadback = false;
    std::shared_ptr<image::Image> captureImage;
    std::shared_ptr<vlk::OffscreenBuffer> buffer;

    // Engine information
    std::shared_ptr<usd::RenderEngine> engine;
    double time;
    double lastTime;
    double startTimeCode;
    double endTimeCode;
    double timeCodesPerSecond;
    UsdStageRefPtr stage = nullptr;

    // Vulkan information.
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;;
    
    //! Compositing shader.
    std::shared_ptr<vlk::Shader> shader;
    
    //! Main rectangle mesh. 
    std::shared_ptr<vlk::VBO> vbo;
    std::shared_ptr<vlk::VAO> vao;
};

usd_window::usd_window(int X, int Y, int W, int H) :
    Fl_Vk_Window(X, Y, W, H),
    _p(new Private)
{
    TLRENDER_P();
    
    mode(FL_RGB | FL_ALPHA);
}

void usd_window::init_colorspace()
{
    TLRENDER_P();

    // This call will try to set colorSpace() to the best color space
    // possible based on what Vulkan returns.
    Fl_Vk_Window::init_colorspace();
            
    // First check if Wayland returned a valid color space for this
    // monitor.
    bool valid_colorspace = false;
    switch (colorSpace())
    {
    case VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT:
    case VK_COLOR_SPACE_EXTENDED_SRGB_LINEAR_EXT:
    case VK_COLOR_SPACE_HDR10_ST2084_EXT:
    case VK_COLOR_SPACE_HDR10_HLG_EXT:
    case VK_COLOR_SPACE_DOLBYVISION_EXT:
        valid_colorspace = true;
        break;
    default:
        break;
    }
            
    // if (valid_colorspace)
    // {
    //     if (p.monitor_first_run)
    //     {
    //         p.screen_index = this->screen_num();
    //         p.monitor = getHDRCapabilities(p.screen_index);
    //         p.monitor_first_run = false; 
    //     }
    //     _getMonitorNits();
    // }
    // else
    {
        // colorSpace() = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
        // format() = VK_FORMAT_B8G8R8A8_SRGB;
        // format() = VK_FORMAT_B8G8R8A8_UNORM;
            
        // if (p.monitor.hdr_enabled)
        //     LOG_STATUS(_("HDR monitor not detected by Vulkan or Window Manager."));
                
        // p.monitor.hdr_enabled = p.monitor.hdr_supported = false;
        // p.monitor.min_nits = 0.001F;
        // p.monitor.max_nits = 100.F;
    }

    std::string msg;            
    msg = string::Format(_("Vulkan color space is {0}")).arg(string_VkColorSpaceKHR(colorSpace()));
    LOG_STATUS(msg);
                    
    msg = string::Format(_("Vulkan format is {0}")).arg(string_VkFormat(format()));
    LOG_STATUS(msg);
}
void usd_window::prepare_render_pass()
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
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Start undefined
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // Final layout for presentation

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

    VkSubpassDependency dependencies[2] = {};

    // External -> subpass: wait for color attachment output before writing
    dependencies[0].srcSubpass    = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass    = 0;
    dependencies[0].srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].dstStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = 0;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = 0;

    // Subpass -> external: ensure writes are done before presentation reads
    dependencies[1].srcSubpass    = 0;
    dependencies[1].dstSubpass    = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask  = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask  = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = 0;
    dependencies[1].dependencyFlags = 0;

    VkRenderPassCreateInfo rp_info = {};
    rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.pNext = NULL;
    rp_info.attachmentCount = (has_depth || has_stencil) ? 2: 1;
    rp_info.pAttachments = attachments;
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &subpass;
    rp_info.dependencyCount = 2;
    rp_info.pDependencies = dependencies;
                    
    VkResult result;
    result = vkCreateRenderPass(device(), &rp_info, NULL, &m_renderPass);
    VK_CHECK(result);

}


void usd_window::prepare_shaders()
{
    TLRENDER_P();
    
    if (!p.shader)
    {
        p.shader = vlk::Shader::create(
            ctx,
            usd::Vertex3_spv,
            usd::Vertex3_spv_len,
            usd::tonemappingFragment_spv,
            usd::tonemappingFragment_spv_len,
            "p.shader");

        // Create parameters for shader.
        math::Matrix4x4f mvp;
        p.shader->createUniform("transform.mvp", mvp, vlk::kShaderVertex);
        p.shader->addFBO("textureSampler"); // default is fragment
        math::Vector4f color(1.F, 1.F, 1.F);
        p.shader->addPush("color", color, vlk::kShaderFragment);
        p.shader->createBindingSet();
                    
        if (p.pipeline_layout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(device(), p.pipeline_layout, nullptr);
            p.pipeline_layout = VK_NULL_HANDLE;
        }
    }

}

void usd_window::prepare_pipeline_layout()
{
    TLRENDER_P();

    // Early return if the layout already exists (avoids recreation on resize)
    if (p.pipeline_layout != VK_NULL_HANDLE) {
        return;  // Already created; reuse it
    }
            
    VkResult result;

    //
    // Prepare main buffer comping layout 
    //

    VkPipelineLayoutCreateInfo pPipelineLayoutCreateInfo = {};
    pPipelineLayoutCreateInfo.sType =
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pPipelineLayoutCreateInfo.pNext = NULL;
    pPipelineLayoutCreateInfo.setLayoutCount = 1;

    VkDescriptorSetLayout setLayout = p.shader->getDescriptorSetLayout();
    pPipelineLayoutCreateInfo.pSetLayouts = &setLayout;

    VkPushConstantRange pushConstantRange = {};
    std::size_t pushSize = p.shader->getPushSize();
    if (pushSize > 0)
    {
        pushConstantRange.stageFlags = p.shader->getPushStageFlags();
        pushConstantRange.offset = 0;
        pushConstantRange.size = pushSize;

        pPipelineLayoutCreateInfo.pushConstantRangeCount = 1;
        pPipelineLayoutCreateInfo.pPushConstantRanges =
            &pushConstantRange;
    }

    result = vkCreatePipelineLayout(
        device(), &pPipelineLayoutCreateInfo, NULL,
        &p.pipeline_layout);
}

void usd_window::prepare()
{
    TLRENDER_P();
    
    prepare_render_pass();
    prepare_shaders();
    prepare_pipeline_layout();

    if (!p.engine)
    {
        p.engine = usd::RenderEngine::create(ctx);
        p.engine->setTimeCode(p.stage, p.time);
    }
}

math::Matrix4x4f usd_window::_createTexturedRectangle()
{
    TLRENDER_P();

    math::Size2i renderSize = math::Size2i(p.buffer->getWidth(),
                                           p.buffer->getHeight());
    
    const auto& mesh =
        geom::box(math::Box2i(0, 0, renderSize.w, renderSize.h));
    const size_t numTriangles = mesh.triangles.size();
    if (!p.vbo || (p.vbo && p.vbo->getSize() != numTriangles * 3))
    {
        p.vbo = vlk::VBO::create(
                    numTriangles * 3, vlk::VBOType::Pos2_F32_UV_U16);
        p.vao.reset();
    }
    if (p.vbo)
    {
        p.vbo->copy(convert(mesh, vlk::VBOType::Pos2_F32_UV_U16));
    }

    if (!p.vao && p.vbo)
    {
        p.vao = vlk::VAO::create(ctx);
        prepare_pipeline();
    }

    // \@todo: (okay as is) - no projection matrix needed
    math::Matrix4x4f mvp = math::ortho(0.F, (float)renderSize.w, 0.F, (float)renderSize.h, -1.F, 1.F);
    return mvp;
}

void usd_window::prepare_pipeline()
{
    TLRENDER_P();
    if (pipeline() != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(device(), pipeline(), nullptr);
        pipeline() = VK_NULL_HANDLE;
    }

    // Elements of new Pipeline (fill with mesh info)
    vlk::VertexInputStateInfo vi;
    vi.bindingDescriptions = p.vbo->getBindingDescription();
    vi.attributeDescriptions = p.vbo->getAttributes();
            
    // Defaults are fine
    vlk::InputAssemblyStateInfo ia;

    // Defaults are fine
    vlk::RasterizationStateInfo rs;
            
    // Defaults are fine
    vlk::ViewportStateInfo vp;
            
    vlk::ColorBlendStateInfo cb;
    vlk::ColorBlendAttachmentStateInfo colorBlendAttachment;
    colorBlendAttachment.blendEnable = VK_FALSE;
    cb.attachments.push_back(colorBlendAttachment);

    const bool hasDepth = mode() & FL_DEPTH;
    const bool hasStencil = mode() & FL_STENCIL;
            
    vlk::DepthStencilStateInfo ds;
    ds.depthTestEnable = hasDepth ? VK_TRUE : VK_FALSE;
    ds.depthWriteEnable = hasDepth ? VK_TRUE : VK_FALSE;
    ds.stencilTestEnable = hasStencil ? VK_TRUE : VK_FALSE;
            
    vlk::DynamicStateInfo dynamicState;
    dynamicState.dynamicStates = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_COLOR_WRITE_MASK_EXT,
    };
            
    // Defaults are fine
    vlk::MultisampleStateInfo ms;

    // Get the vertex and fragment shaders
    std::vector<vlk::PipelineCreationState::ShaderStageInfo>
        shaderStages(2);

    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].name = p.shader->getName();
    shaderStages[0].module = p.shader->getVertex();
    shaderStages[0].entryPoint = "main";

    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].name = p.shader->getName();
    shaderStages[1].module = p.shader->getFragment();
    shaderStages[1].entryPoint = "main";

    //
    // Pass pipeline creation parameters to pipelineState.
    //
    vlk::PipelineCreationState pipelineState;
    pipelineState.vertexInputState = vi;
    pipelineState.inputAssemblyState = ia;
    pipelineState.colorBlendState = cb;
    pipelineState.rasterizationState = rs;
    pipelineState.depthStencilState = ds;
    pipelineState.viewportState = vp;
    pipelineState.multisampleState = ms;
    pipelineState.dynamicState = dynamicState;
    pipelineState.stages = shaderStages;
    pipelineState.renderPass = renderPass();
    pipelineState.layout = p.pipeline_layout;

    pipeline() = pipelineState.create(device());
    if (pipeline() == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Composition pipeline failed");
    }
}

void usd_window::destroy()
{
    TLRENDER_P();
    
    p.vbo.reset();
    p.vao.reset();

    if (pipeline() != VK_NULL_HANDLE)
    {
        vkDestroyPipeline(device(), pipeline(), nullptr);
        pipeline() = VK_NULL_HANDLE;
    }
    
    Fl_Vk_Window::destroy();
}

void usd_window::hide()
{
    TLRENDER_P();

    // Needed
    wait_device();
    
    // Destroy main renderer
    p.engine.reset();

    // Destroy shader
    p.shader.reset();
    
    if (p.pipeline_layout != VK_NULL_HANDLE)
    {
        vkDestroyPipelineLayout(device(), p.pipeline_layout, nullptr);
        p.pipeline_layout = VK_NULL_HANDLE;
    }
    
    Fl_Vk_Window::hide();
}

void usd_window::flush()
{
    TLRENDER_P();

    // Capture the frame index BEFORE the base flush() advances it.
    // swap_buffers() will submit the main cmd and signal m_frames[captureFrame].fence.
    uint32_t captureFrame = m_currentFrameIndex;

    Fl_Vk_Window::flush();  // → draw() records copy into main cmd
                             // → swap_buffers() submits it and signals the fence

    if (p.pendingReadback && p.buffer && p.captureImage)
    {
        p.pendingReadback = false;

        // Wait until the submitted frame (and thus the inline copy) is done on GPU.
        // This is the same fence FLTK already manages for that frame.
        vkWaitForFences(device(), 1,
                        &m_frames[captureFrame].fence,
                        VK_TRUE, UINT64_MAX);

        void* data = p.buffer->getInlineReadbackPtr();
        if (data)
        {
            // memcpy(p.captureImage->getData(), data,
            //        p.captureImage->getDataByteCount());
            // saveHalfRGB("/home/gga/test.exr", p.captureImage);
        }
    }
}

void usd_window::draw()
{
    TLRENDER_P();

    VkCommandBuffer cmd = getCurrentCommandBuffer();

    // Clear the viewport
    m_clearColor = { 0.2, 0.2, 0.2, 0.0 };
    begin_render_pass();
    end_render_pass();

    if (p.time != p.lastTime)
    {
        p.engine->draw(cmd, m_currentFrameIndex, 1024);
        
        p.buffer = p.engine->getFBO();
    }
    
    // ── Inline readback ──────────────────────────────────────────────────
    // Record the image→buffer copy into the MAIN command buffer right here,
    // while the offscreen image is still in COLOR_ATTACHMENT_OPTIMAL.
    // The copy will execute on the GPU in the same submission as the render.
    // flush() waits on the frame fence before reading the mapped pointer.
    if (!p.buffer)
        return;
    
    const uint32_t renderWidth = p.buffer->getWidth();
    const uint32_t renderHeight = p.buffer->getHeight();
        
    if (!p.captureImage ||
        renderWidth != p.captureImage->getWidth() ||
        renderHeight != p.captureImage->getHeight())
    {
        p.vbo.reset();
            
        p.captureImage = image::Image::create(
            renderWidth, renderHeight, image::PixelType::RGBA_F16);
    }
        
    p.buffer->readPixelsInline(cmd,
                               0, 0,
                               renderWidth,
                               renderHeight);
    p.pendingReadback = true;
    // ─────────────────────────────────────────────────────────────────────

    
    const math::Matrix4x4f ortho = _createTexturedRectangle();

    p.buffer->transitionToShaderRead(cmd);

    math::Size2i renderSize(p.buffer->getWidth(), p.buffer->getHeight());
    
    float windowW = (float)pixel_w();
    float windowH = (float)pixel_h();

    float renderW = (float)renderSize.w;
    float renderH = (float)renderSize.h;

    // 1. Calculate scaling factors for both axes
    float scaleX = windowW / renderW;
    float scaleY = windowH / renderH;

    // 2. Use the smaller scale to "fit" the render inside the window
    // This prevents the render from being larger than the window
    float scale = (scaleX < scaleY) ? scaleX : scaleY;

    // 3. Calculate the final scaled dimensions
    float finalW = renderW * scale;
    float finalH = renderH * scale;

    // 4. Calculate offsets to center the scaled render
    float offsetX = (windowW - finalW) * 0.5f;
    float offsetY = (windowH - finalH) * 0.5f;

    VkViewport viewport = {};
    viewport.x        = offsetX;
    viewport.y        = offsetY;
    viewport.width    = finalW;
    viewport.height   = finalH;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);

    VkRect2D scissor = {};
    scissor.offset.x      = (int32_t)std::max(0.0f, offsetX);
    scissor.offset.y      = (int32_t)std::max(0.0f, offsetY);
    scissor.extent.width  = (uint32_t)finalW;
    scissor.extent.height = (uint32_t)finalH;
    vkCmdSetScissor(cmd, 0, 1, &scissor);
            
    // Bind the shaders to the current frame index.
    p.shader->bind(m_currentFrameIndex);

    // Bind the main composition pipeline (created/managed outside this
    // draw loop)
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS,
                      pipeline());
    
    // --- Update Descriptor Set for the SECOND pass (Composition) ---
    // This updates the descriptor set for the CURRENT frame index on
    // the CPU.
    p.shader->setUniform("transform.mvp", ortho, vlk::kShaderVertex);
    p.shader->setFBO("textureSampler", p.buffer);
 
    begin_render_pass(cmd);
    
    // --- Bind Descriptor Set for the SECOND pass ---
    // Record the command to bind the descriptor set for the CURRENT
    // frame index
    VkDescriptorSet descriptorSet = p.shader->getDescriptorSet();
    vkCmdBindDescriptorSets(
        cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p.pipeline_layout, 0, 1,
        &descriptorSet, 0, nullptr);

    math::Vector4f color(1.F, 1.F, 1.F);
    vkCmdPushConstants(
        cmd, p.pipeline_layout,
        p.shader->getPushStageFlags(), 0, sizeof(math::Vector4f), &color);

    if (p.vao && p.vbo)
    {
        const VkColorComponentFlags allMask[] =
            { VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
              VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT };
        ctx.vkCmdSetColorWriteMaskEXT(cmd, 0, 1, allMask);
            
        p.vao->bind(m_currentFrameIndex);
        p.vao->draw(cmd, p.vbo);
    }

    end_render_pass(cmd);

    p.lastTime = p.time;
}

static void increment_timecode_cb(usd_window* w)
{
    w->nextTimeCode();
}

void usd_window::nextTimeCode()
{
    TLRENDER_P();

    if (p.engine)
    {
#if ANIMATE
        p.time += 1.0;
        
        if (p.time > p.endTimeCode)
            p.time = p.startTimeCode;

        p.engine->setTimeCode(p.stage, p.time);
    
        redraw();
#endif
    }

    double timeout = 1.0 / p.timeCodesPerSecond;
    Fl::repeat_timeout(timeout, (Fl_Timeout_Handler) increment_timecode_cb,
                       this);
}

void usd_window::setUSDFile(const std::string& fileName)
{
    TLRENDER_P();
    
    TfDiagnosticMgr::GetInstance().SetQuiet(false);


    std::cout << "Reading stage..." << std::endl;
    p.stage = UsdStage::Open(fileName);
    if (!p.stage)
    {
        std::cerr << "Could not read stage " << fileName
                  << ".  Aborting..." << std::endl;
        exit(1);
    }
    std::cout << "Read stage..." << std::endl;

    p.startTimeCode = p.stage->GetStartTimeCode();
    p.endTimeCode   = p.stage->GetEndTimeCode();
    p.timeCodesPerSecond = p.stage->GetTimeCodesPerSecond();
    p.time = p.startTimeCode;
    p.lastTime = p.startTimeCode - 1000000;
    
    double timeout = 1.0 / p.timeCodesPerSecond;

    Fl::add_timeout(timeout, (Fl_Timeout_Handler) increment_timecode_cb, this);
}

int main(int argc, char **argv) {
    if (argc != 2)
    {
        std::cout << argv[0] << " <file.usd>" << std::endl;
        exit(1);
    }
    
    Fl::use_high_res_VK(1);

    Fl_Window window(640, 640);
  
    usd_window sw(10, 10, window.w() - 20, window.h() - 20);
    sw.setUSDFile(argv[1]);

    window.resizable(&sw);
    window.end();
    window.show();
        
    return Fl::run();
}

#if defined(_WIN32) && defined(_MSC_VER)

#include <stdio.h>

#include <FL/fl_utf8.h>
#include <FL/fl_string_functions.h>

int WINAPI WinMain(
    HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int rc;
    int i;
    int argc;
    char** argv;

    /*
     * If we are compiling in debug mode, open a console window so
     * we can see any printf's, etc...
     *
     * While we can detect if the program was run from the command-line -
     * look at the CMDLINE environment variable, it will be "WIN" for
     * programs started from the GUI - the shell seems to run all Windows
     * applications in the background anyways...
     */

    /* Convert the command line arguments to UTF-8 */
    LPWSTR* wideArgv = CommandLineToArgvW(GetCommandLineW(), &argc);

    /* Allocate an array of 'argc + 1' string pointers */
    argv = (char **)malloc((argc + 1) * sizeof(char *));
  
    /* Convert the command line arguments to UTF-8 */
    for (i = 0; i < argc; i++) {
        /* find the required size of the buffer */
        int u8size = WideCharToMultiByte(CP_UTF8,     /* CodePage */
                                         0,           /* dwFlags */
                                         wideArgv[i], /* lpWideCharStr */
                                         -1,          /* cchWideChar */
                                         NULL,        /* lpMultiByteStr */
                                         0,           /* cbMultiByte */
                                         NULL,        /* lpDefaultChar */
                                         NULL);       /* lpUsedDefaultChar */
        if (u8size > 0) {
            char *strbuf = (char *)malloc(u8size);
            int ret = WideCharToMultiByte(CP_UTF8,     /* CodePage */
                                          0,           /* dwFlags */
                                          wideArgv[i], /* lpWideCharStr */
                                          -1,          /* cchWideChar */
                                          strbuf,      /* lpMultiByteStr */
                                          u8size,      /* cbMultiByte */
                                          NULL,        /* lpDefaultChar */
                                          NULL);       /* lpUsedDefaultChar */
            if (ret) {
                argv[i] = strbuf;
            } else {
                argv[i] = _strdup("");
                free(strbuf);
                fprintf(stderr, "Failed to convert arg %d\n", i);
            }
        } else {
            argv[i] = _strdup("");
        }
    }
    argv[argc] = NULL; /* required by C standard at end of list */

    /* Free the wide character string array */
    LocalFree(wideArgv);

    /* Call the program's entry point main() */
    rc = main(argc, argv);

    /* Cleanup allocated memory for argv */
    for (int i = 0; i < argc; ++i)
    {
        free((void*)argv[i]);
    }
    free((void*)argv);

    return rc;
}

#endif
