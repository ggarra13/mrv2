
#define PRINT_STATS 0
#define BAKE_JOINTS 1

// #include "USDProcessSkeletonRoot.h"  // \@todo: do deformation in compute shader
#include "USDCollectTextures.h"

#include <pxr/pxr.h>

// math primitives
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/vec3f.h>

// diagnostics
#include <pxr/base/tf/diagnostic.h>
#include <pxr/base/tf/token.h>

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

// Primitive types
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/nurbsCurves.h>
#include <pxr/usd/usdGeom/nurbsPatch.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/xformCache.h>
#include <pxr/usd/usdGeom/primvarsAPI.h>

// Skeleton types
#include <pxr/usd/usdSkel/animQuery.h>
#include <pxr/usd/usdSkel/bakeSkinning.h>
#include <pxr/usd/usdSkel/bindingAPI.h>
#include <pxr/usd/usdSkel/cache.h>
#include <pxr/usd/usdSkel/root.h>
#include <pxr/usd/usdSkel/skeletonQuery.h>
#include <pxr/usd/usdSkel/utils.h>

// Material and Shaders
#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/usdShade/materialBindingAPI.h>
#include <pxr/usd/usdShade/shader.h>

// Not sure about these
#include <pxr/imaging/hdx/tokens.h>
#include <pxr/imaging/hdx/types.h>

#include <tlTimelineVk/Render.h>
#include <tlTimelineVk/RenderShadersBinary.h>
#include <tlTimelineVk/USDTextureSlots.h>

#include <tlVk/Mesh.h>
#include <tlVk/OffscreenBuffer.h>
#include <tlVk/Shader.h>

#include <tlCore/Image.h>
#include <tlCore/Mesh.h>
#include <tlCore/Path.h>

#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfArray.h>
#include <OpenEXR/ImfHeader.h>

#include <FL/platform.H>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/math.h>

#include <FL/Fl_Vk_Window.H>
#include <FL/Fl_Vk_Utils.H>

#include <memory>
#include <iostream>
#include <string>
#include <unordered_map>

using namespace PXR_NS;

namespace
{
    UsdGeomCamera getCamera(
        const UsdStageRefPtr& stage,
        const std::string& name = std::string())
    {
        UsdGeomCamera out;
        if (!name.empty())
        {
            out = UsdAppUtilsGetCameraAtPath(stage, SdfPath(name));
        }
        if (!out)
        {
            const TfToken primaryCameraName =
                UsdUtilsGetPrimaryCameraName();
            out = UsdAppUtilsGetCameraAtPath(
                stage, SdfPath(primaryCameraName));
        }
        if (!out)
        {
            for (const auto& prim : stage->Traverse())
            {
                if (prim.IsA<UsdGeomCamera>())
                {
                    out = UsdGeomCamera(prim);
                    break;
                }
            }
        }
        return out;
    }

    GfCamera getCameraToFrameStage(
        const UsdStagePtr& stage, UsdTimeCode timeCode,
        const TfTokenVector& includedPurposes)
    {
        GfCamera gfCamera;
        UsdGeomBBoxCache bboxCache(timeCode, includedPurposes, true);
        const GfBBox3d bbox =
            bboxCache.ComputeWorldBound(stage->GetPseudoRoot());
        const GfVec3d center = bbox.ComputeCentroid();
        const GfRange3d range = bbox.ComputeAlignedRange();
        const GfVec3d dim = range.GetSize();
        const TfToken upAxis = UsdGeomGetStageUpAxis(stage);

        GfVec2d planeCorner;
        if (upAxis == UsdGeomTokens->y)
        {
            planeCorner = GfVec2d(dim[0], dim[1]) / 2;
        }
        else
        {
            planeCorner = GfVec2d(dim[0], dim[2]) / 2;
        }
        const float planeRadius = sqrt(GfDot(planeCorner, planeCorner));

        const float halfFov =
            gfCamera.GetFieldOfView(GfCamera::FOVHorizontal) / 2.0;
        float distance = planeRadius / tan(GfDegreesToRadians(halfFov));

        if (upAxis == UsdGeomTokens->y)
        {
            distance += dim[2] / 2;
        }
        else
        {
            distance += dim[1] / 2;
        }

        GfMatrix4d xf;
        if (upAxis == UsdGeomTokens->y)
        {
            xf.SetTranslate(center + GfVec3d(0, 0, distance));
        }
        else
        {
            xf.SetRotate(GfRotation(GfVec3d(1, 0, 0), 90));
            xf.SetTranslateOnly(center + GfVec3d(0, -distance, 0));
        }
        gfCamera.SetTransform(xf);
        return gfCamera;
    }

} // namespace


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
        
        std::cout << "Image saved successfully to " << filename << std::endl;
        exit(0);
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

    
    // USD information
    file::Path path;
    UsdStageRefPtr stage = nullptr;

    // Timeline
    double startTimeCode = 0.F;
    double endTimeCode = 100.F;
    double timeCodesPerSecond = 24.0F;

    // Current time
    double time = 0;

    // Vulkan information.
    bool collectTextures = true;
    VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;
    std::unordered_map<std::string, tl::usd::ShaderTextures > textures;

    //! tlRender context
    std::shared_ptr<system::Context> context;
    
    //! Offscreen renderer.
    std::shared_ptr<timeline_vlk::Render> render;
    
    //! Offscreen buffer.
    std::shared_ptr<tl::vlk::OffscreenBuffer> buffer;
    
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

    
    p.context = system::Context::create();
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
    
    if (!p.render) {
        p.render = timeline_vlk::Render::create(ctx, p.context);
    }
    
    if (!p.shader)
    {
        p.shader = vlk::Shader::create(
            ctx,
            timeline_vlk::Vertex3_spv,
            timeline_vlk::Vertex3_spv_len,
            timeline_vlk::textureFragment_spv,
            timeline_vlk::textureFragment_spv_len,
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
    prepare_render_pass();
    prepare_shaders();
    prepare_pipeline_layout();
}

math::Matrix4x4f usd_window::_createTexturedRectangle()
{
    TLRENDER_P();
    
    const math::Size2i renderSize(pixel_w(), pixel_h());

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
    p.render.reset();

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
            //memcpy(p.captureImage->getData(), data,
            //       p.captureImage->getDataByteCount());
            //saveHalfRGB("/home/gga/test.exr", p.captureImage);
        }
    }
}

void usd_window::draw()
{
    TLRENDER_P();

    VkCommandBuffer cmd = getCurrentCommandBuffer();

    if (p.collectTextures)
    {
        p.collectTextures = false;
        usd::CollectTextures(ctx, cmd, p.stage, p.time, p.textures);
    }

    const double time = p.time;

    std::string cameraName;
    auto usdCamera = getCamera(p.stage, cameraName);

    GfCamera gfCamera;
    if (usdCamera)
    {
        gfCamera = usdCamera.GetCamera(time);
    }
    else
    {
        const TfTokenVector purposes(
            {UsdGeomTokens->default_, UsdGeomTokens->proxy});
        gfCamera = getCameraToFrameStage(p.stage, time, purposes);
    }
    
    float aspectRatio = gfCamera.GetAspectRatio();
    if (GfIsClose(aspectRatio, 0.F, 1e-4))
    {
        aspectRatio = 1.F;
    }
    const GfFrustum frustum = gfCamera.GetFrustum();
    const GfVec3d cameraPos = frustum.GetPosition();
    
    const GfMatrix4d& viewMatrix = frustum.ComputeViewMatrix();
    const GfMatrix4d& projectionMatrix = frustum.ComputeProjectionMatrix();

    const GfMatrix4d& Gfmvp = viewMatrix * projectionMatrix;
    const math::Matrix4x4f mvp(
        Gfmvp[0][0], Gfmvp[0][1], Gfmvp[0][2], Gfmvp[0][3],
        Gfmvp[1][0], Gfmvp[1][1], Gfmvp[1][2], Gfmvp[1][3],
        Gfmvp[2][0], Gfmvp[2][1], Gfmvp[2][2], Gfmvp[2][3],
        Gfmvp[3][0], Gfmvp[3][1], Gfmvp[3][2], Gfmvp[3][3]);
    
    const size_t renderWidth = pixel_w();
    const size_t renderHeight = pixel_h();

    vlk::OffscreenBufferOptions offscreenBufferOptions;
    offscreenBufferOptions.colorType = image::PixelType::RGBA_F16;
    offscreenBufferOptions.depth = vlk::OffscreenDepth::_32;
    offscreenBufferOptions.stencil = vlk::OffscreenStencil::kNone;
    offscreenBufferOptions.pbo = true;

    const math::Size2i renderSize(pixel_w(), pixel_h());
    if (vlk::doCreate(
            p.buffer, renderSize, offscreenBufferOptions))
    {
        p.buffer = vlk::OffscreenBuffer::create(
            ctx, renderSize, offscreenBufferOptions);
                    
        // As render resolution might have changed,
        // we need to reset the quad size.
        p.vbo.reset();
    }

    // locale::SetAndRestore saved;
    timeline::RenderOptions renderOptions;
    renderOptions.colorBuffer = image::PixelType::RGBA_F16;
    renderOptions.clear = true;
    renderOptions.clearColor = image::Color4f(0.0F, 0.0F, 0.0F, 0.F);
    
    p.render->begin(
        cmd, p.buffer, m_currentFrameIndex, renderSize,
        renderOptions);
    p.render->applyTransforms();
    p.render->beginLoadRenderPass();
    p.render->setupViewportAndScissor();
    
    auto oldTransform = p.render->getTransform();
    p.render->setTransform(mvp);


    
    UsdPrimRange range(p.stage->GetPseudoRoot(),
                       UsdTraverseInstanceProxies());
    GfMatrix4d matrix;
    UsdSkelCache skelCache;
    UsdGeomXformCache xformCache(time);
    std::string primPath;

    //
    // Stats
    //
    std::size_t numMeshes = 0;
    std::size_t numSubdivs = 0;
    std::size_t numNurbsPatches = 0;
    std::size_t numNurbsCurves = 0;
    std::size_t numBasisCurves = 0;
    std::size_t numSkeletons = 0;
    std::size_t numPoints = 0;
    std::size_t numSpheres = 0;
    
    std::shared_ptr<vlk::Texture> texture;    
    for (auto it = range.begin(); it != range.end(); ++it) {
        
        if (it->IsA<UsdGeomImageable>()) {
            UsdGeomImageable imageable(*it);
            
            if (imageable.ComputeVisibility(time) ==
                UsdGeomTokens->invisible) {
                // If this prim is invisible, its entire subtree is invisible.
                // Prune the traversal to skip all children.
                it.PruneChildren();
                continue;
            }

            // If purpose is not default or not render, don't use this
            // geometry.
            TfToken purpose = imageable.ComputePurpose();
            if (purpose != UsdGeomTokens->default_ &&
                purpose != UsdGeomTokens->render)
                continue;
        }
        
        primPath = it->GetPath().GetString();
        matrix = xformCache.GetLocalToWorldTransform(*it);
        const math::Matrix4x4f modelMatrix(matrix[0][0], matrix[0][1],
                                           matrix[0][2], matrix[0][3],
                                           matrix[1][0], matrix[1][1],
                                           matrix[1][2], matrix[1][3],
                                           matrix[2][0], matrix[2][1],
                                           matrix[2][2], matrix[2][3],
                                           matrix[3][0], matrix[3][1],
                                           matrix[3][2], matrix[3][3]);
        
        VtArray<GfVec3f> colors;
        UsdGeomGprim gprim(*it);
        if (gprim)
            gprim.GetDisplayColorAttr().Get(&colors);

        image::Color4f color(0.5F, 0.5F, 0.5F);
        std::string shaderName;
        if (colors.size() == 0)
        {
            UsdShadeMaterial material = UsdShadeMaterialBindingAPI(*it).
                                        ComputeBoundMaterial();
            if (material)
            {
                UsdShadeShader shader = material.ComputeSurfaceSource();
                TfToken shaderId;
                shader.GetIdAttr().Get(&shaderId);
                shaderName = shaderId.GetString();
                if (shader)
                {
                    GfVec3f diffuse;
                    UsdShadeInput diffuseColorInput = shader.GetInput(TfToken("diffuseColor"));
                    if (diffuseColorInput)
                    {
                        diffuseColorInput.Get(&diffuse);
                        color.r = diffuse[0];
                        color.g = diffuse[1];
                        color.b = diffuse[2];
                    }
                }
            }
        }
        else if (colors.size() == 1)
        {
            color.r = colors[0][0];
            color.g = colors[0][1];
            color.b = colors[0][2];
        }
        
        if (it->IsA<UsdSkelRoot>())
        {
            ++numSkeletons;

            //UsdSkelRoot skelRoot(*it);
            //usd::ProcessSkeletonRoot(skelRoot, skelCache, time);
        }
        else if (it->IsA<UsdGeomMesh>())
        {
            ++numMeshes;
            UsdGeomMesh usdMesh = UsdGeomMesh(*it);

            //bool doubleSided = usdMesh.GetDoubleSidedAttr().Get();
            //std::cerr << primPath << " double sided=" << doubleSided << std::endl;

            // -------------------------
            // 1. VERTICES (Points)
            // -------------------------
            VtArray<GfVec3f> points;
            usdMesh.GetPointsAttr().Get(&points, time);

            // Faces vertex counts: number vertices per face.
            VtArray<int> faceVertexCounts;
            usdMesh.GetFaceVertexCountsAttr().Get(&faceVertexCounts, time);
            
            // faceVertexIndices: flat list of vertex indices for all faces
            VtArray<int> faceVertexIndices;
            usdMesh.GetFaceVertexIndicesAttr().Get(&faceVertexIndices, time);

            
            geom::TriangleMesh3 geom;

            // Get points.
            geom.v.reserve(points.size());
            for (int i = 0; i < points.size(); ++i)
            {
                const auto& p = points[i];
                geom.v.push_back(math::Vector3f(p[0], p[1], p[2]));
            }

            // Get Normals if any.
            UsdGeomPrimvarsAPI primvarsAPI(usdMesh);
            UsdGeomPrimvar normalsPrimvar = primvarsAPI.GetPrimvar(UsdGeomTokens->normals);
            if (normalsPrimvar.IsDefined()) {
                
                VtArray<GfVec3f> normals;
                normalsPrimvar.ComputeFlattened(&normals, time);
                
                TfToken interp = normalsPrimvar.GetInterpolation();
                // std::cout << "Normals count: " << normals.size()
                //           << "  interpolation: " << interp << "\n";

                // walks faceVertexIndices for faceVarying
                int faceCornerIdx = 0; 

                for (size_t faceIdx = 0; faceIdx < faceVertexCounts.size();
                     ++faceIdx) {
                    int vertCount = faceVertexCounts[faceIdx];

                    for (int i = 0; i < vertCount; ++i) {
                        int pointIdx = faceVertexIndices[faceCornerIdx + i];
                        GfVec3f n;

                        if (interp == UsdGeomTokens->faceVarying) {
                            // One normal per face-corner, in face-winding order
                            n = normals[faceCornerIdx + i];
                        } else if (interp == UsdGeomTokens->vertex) {
                            // Normal indexed the same way as points
                            n = normals[pointIdx];
                        } else if (interp == UsdGeomTokens->uniform) {
                            // One normal for the entire face
                            n = normals[faceIdx];
                        } else {
                            // constant — one normal for the whole mesh
                            n = normals[0];
                        }

                        geom.n.push_back(math::Vector3f(n[0], n[1], n[2]));
                    }
                    faceCornerIdx += vertCount;
                }
            }

            //
            // Get UVs (st)
            //
            UsdGeomPrimvar st = primvarsAPI.GetPrimvar(TfToken("st"));
            if (st.IsDefined()) {
                
                VtArray<GfVec2f> values;
                if (st.Get(&values))
                {
                    TfToken interp = st.GetInterpolation();
                    // std::cerr << "STs found=" << values.size()
                    //           << " interpolation=" << interp << std::endl;

                    if (st.IsIndexed())
                    {
                        VtArray<int> indices;
                        st.GetIndices(&indices);

                        std::vector<GfVec2f> expanded(indices.size());
                        for (size_t i = 0; i < indices.size(); ++i)
                            expanded[i] = values[indices[i]];

                        // walks faceVertexIndices for faceVarying
                        int faceCornerIdx = 0; 

                        for (size_t faceIdx = 0; faceIdx < faceVertexCounts.size();
                             ++faceIdx) {
                            int vertCount = faceVertexCounts[faceIdx];
                                
                            for (int i = 0; i < vertCount; ++i) {
                                int pointIdx = faceVertexIndices[faceCornerIdx + i];
                                GfVec2f uv;

                                if (interp == UsdGeomTokens->faceVarying) {
                                    // One uv per face-corner, in face-winding order
                                    uv = expanded[faceCornerIdx + i];
                                } else if (interp == UsdGeomTokens->vertex) {
                                    // UV indexed the same way as points
                                    uv = expanded[pointIdx];
                                } else if (interp == UsdGeomTokens->uniform) {
                                    // One UV for the entire face
                                    uv = expanded[faceIdx];
                                } else {
                                    // constant — one UV for the whole mesh
                                    uv = expanded[0];
                                }

                                geom.t.push_back(math::Vector2f(uv[0], 1.0F - uv[1]));
                            }
                            faceCornerIdx += vertCount;
                        }
                    }
                    else
                    {
                        // walks faceVertexIndices for faceVarying
                        int faceCornerIdx = 0; 

                        for (size_t faceIdx = 0; faceIdx < faceVertexCounts.size();
                             ++faceIdx) {
                            int vertCount = faceVertexCounts[faceIdx];
                                
                            for (int i = 0; i < vertCount; ++i) {
                                int pointIdx = faceVertexIndices[faceCornerIdx + i];
                                GfVec2f uv;

                                if (interp == UsdGeomTokens->faceVarying) {
                                    // One uv per face-corner, in face-winding order
                                    uv = values[faceCornerIdx + i];
                                } else if (interp == UsdGeomTokens->vertex) {
                                    // UV indexed the same way as points
                                    uv = values[pointIdx];
                                } else if (interp == UsdGeomTokens->uniform) {
                                    // One UV for the entire face
                                    uv = values[faceIdx];
                                } else {
                                    // constant — one UV for the whole mesh
                                    uv = values[0];
                                }

                                geom.t.push_back(math::Vector2f(uv[0], uv[1]));
                            }
                            faceCornerIdx += vertCount;
                        }
                    }
                }
            }

            // Get triangles.
            int indexOffset = 0;
            const bool hasNormals = !geom.n.empty();
            const bool hasUVs = !geom.t.empty();
            
            std::vector<math::Vector3f> normals(points.size());
            for (int vertCount : faceVertexCounts)
            {
                // Fan triangulation: anchor at faceVertexIndices[indexOffset]
                // e.g. a quad [A, B, C, D] → (A,B,C), (A,C,D)
                for (int i = 1; i < vertCount - 1; ++i)
                {
                    geom::Triangle3 triangle;
                    const int i0 = faceVertexIndices[indexOffset];
                    const int i1 = faceVertexIndices[indexOffset + i];
                    const int i2 = faceVertexIndices[indexOffset + i + 1];
                    
                    triangle.v[0].v = i0 + 1;
                    triangle.v[1].v = i1 + 1;
                    triangle.v[2].v = i2 + 1;

                    if (hasNormals)
                    {
                        triangle.v[0].n = indexOffset + 1;
                        triangle.v[1].n = indexOffset + i + 1;
                        triangle.v[2].n = indexOffset + i + 2;
                    }
                    
                    if (hasUVs)
                    {
                        triangle.v[0].t = indexOffset + 1;
                        triangle.v[1].t = indexOffset + i + 1;
                        triangle.v[2].t = indexOffset + i + 2;
                    }
                    
                    geom.triangles.push_back(triangle);
                }
                indexOffset += vertCount;
            }

            //
            // Find the textures if the mesh has UVs 
            //
            std::unordered_map<int, std::shared_ptr<vlk::Texture > > textures;
            if (hasUVs)
            {
                auto i = p.textures.find(primPath);
                if (i != p.textures.end())
                {
                    textures = i->second;
                    shaderName = "UsdPreviewSurface";
                }
            }
            else
            {
                shaderName = "dummy";
            }
            
            p.render->draw3DMesh(geom, modelMatrix, color, shaderName,
                                 textures);
        }
        else if (it->IsA<UsdGeomNurbsPatch>())
        {
            ++numNurbsPatches; 
            UsdGeomNurbsPatch out = UsdGeomNurbsPatch(*it);
        }
        else if (it->IsA<UsdGeomNurbsCurves>())
        {
            ++numNurbsCurves; 
            UsdGeomNurbsCurves out = UsdGeomNurbsCurves(*it);
        }
        else if (it->IsA<UsdGeomBasisCurves>())
        {
            ++numBasisCurves; 
            UsdGeomBasisCurves out = UsdGeomBasisCurves(*it);
        }
        else if (it->IsA<UsdGeomSphere>())
        {
            ++numSpheres; 
            UsdGeomSphere out = UsdGeomSphere(*it);
            float radius = 1;
            out.GetRadiusAttr().Get(&radius, time);
            auto geom = geom::sphere(radius, 16, 16);
            const bool hasUVs = !geom.t.empty();

            std::unordered_map<int, std::shared_ptr<vlk::Texture > > textures;
            if (hasUVs)
            {
                auto i = p.textures.find(primPath);
                if (i != p.textures.end())
                {
                    textures = i->second;
                }
            }
            
            p.render->draw3DMesh(geom, modelMatrix, color,
                                 shaderName, textures);
        }
        // \@todo: cylinder
    }

    p.render->endRenderPass();
    p.render->end();

#if PRINT_STATS
    std::cout << "       Meshes = " << numMeshes << std::endl
              << "      Subdivs = " << numSubdivs << std::endl
              << "Nurbs Patches = " << numNurbsPatches << std::endl
              << " Nurbs Curves = " << numNurbsCurves << std::endl
              << " Basis Curves = " << numBasisCurves << std::endl
              << "       Points = " << numPoints << std::endl
              << "    Skeletons = " << numSkeletons << std::endl
              << "      Spheres = " << numSpheres << std::endl;
#endif
    
    p.render->setTransform(oldTransform);

    // ── Inline readback ──────────────────────────────────────────────────
    // Record the image→buffer copy into the MAIN command buffer right here,
    // while the offscreen image is still in COLOR_ATTACHMENT_OPTIMAL.
    // The copy will execute on the GPU in the same submission as the render.
    // flush() waits on the frame fence before reading the mapped pointer.
    if (p.buffer)
    {
        if (!p.captureImage ||
            renderWidth != p.captureImage->getWidth() ||
            renderHeight != p.captureImage->getHeight())
        {
            p.captureImage = image::Image::create(
                renderWidth, renderHeight, image::PixelType::RGBA_F16);
        }
        
        p.buffer->readPixelsInline(cmd,
                                   0, 0,
                                   p.captureImage->getWidth(),
                                   p.captureImage->getHeight());
        p.pendingReadback = true;
    }
    // ─────────────────────────────────────────────────────────────────────

    
    const math::Matrix4x4f ortho = _createTexturedRectangle();

    if (p.buffer)
    {
        p.buffer->transitionToShaderRead(cmd);
    }
        

    uint32_t W = pixel_w();
    uint32_t H = pixel_h();
            
    VkViewport viewport = {};
    viewport.width = static_cast<float>(W);
    viewport.height = static_cast<float>(H);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd, 0, 1, &viewport);
    
    VkRect2D scissor = {};
    scissor.extent.width = W;
    scissor.extent.height = H;
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
}

static void increment_timecode_cb(usd_window* w)
{
    w->nextTimeCode();
}

void usd_window::nextTimeCode()
{
    TLRENDER_P();

    p.time += 1.0;

    if (p.time > p.endTimeCode)
        p.time = p.startTimeCode;
    
    redraw();

    double timeout = 1.0 / p.timeCodesPerSecond;
    Fl::repeat_timeout(timeout, (Fl_Timeout_Handler) increment_timecode_cb,
                       this);
}

void usd_window::setUSDFile(const std::string& fileName)
{
    TLRENDER_P();
    
    p.path = file::Path(fileName);

    
    TfDiagnosticMgr::GetInstance().SetQuiet(true);

    
    p.stage = UsdStage::Open(fileName);
    if (!p.stage)
    {
        std::cerr << "Could not read stage.  Aborting..." << std::endl;
        exit(1);
    }

    p.startTimeCode = p.stage->GetStartTimeCode();
    p.endTimeCode   = p.stage->GetEndTimeCode();
    p.timeCodesPerSecond = p.stage->GetTimeCodesPerSecond();
    p.time = p.startTimeCode;

#if BAKE_JOINTS
    std::cout << "Baking joints..." << std::endl;
    // Bake the all skeletons and bound geometry over the time range.
    // \@todo: this is done on the CPU (slow) and it uses a lot of memory.
    UsdPrimRange range(p.stage->GetPseudoRoot(),
                       UsdTraverseInstanceProxies());
    GfInterval interval(p.startTimeCode, p.endTimeCode);
    UsdSkelBakeSkinning(range, interval);
    std::cout << "Baked joints..." << std::endl;
#endif
    
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
