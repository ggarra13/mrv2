#define USE_GL 0

#include <pxr/pxr.h>
#include <pxr/base/tf/diagnostic.h>
#include <pxr/base/tf/token.h>
#include <pxr/usd/usd/timeCode.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdUtils/pipeline.h>
#include <pxr/usdImaging/usdAppUtils/api.h>
#include <pxr/usdImaging/usdAppUtils/camera.h>
#include <pxr/imaging/hd/renderBuffer.h>

// uses UsdImagingGLEngine *AND* hydra
#include <pxr/usdImaging/usdAppUtils/frameRecorder.h>  


// these should not be used
#include <pxr/imaging/hdSt/hioConversions.h>
#include <pxr/imaging/hdSt/textureUtils.h>

// Not sure about these
#include <pxr/imaging/hdx/tokens.h>
#include <pxr/imaging/hdx/types.h>

#include <tlCore/Image.h>
#include <tlCore/Path.h>

#if USE_GL

#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>

#define SUPERCLASS Fl_Gl_Window

#else

#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/nurbsCurves.h>
#include <pxr/usd/usdGeom/nurbsPatch.h>
#include <pxr/usd/usdGeom/xformCache.h>

#include <tlVk/Mesh.h>
#include <tlVk/OffscreenBuffer.h>
#include <tlVk/Shader.h>

#define SUPERCLASS Fl_Vk_Window

#include <FL/Fl_Vk_Window.H>

#endif

#include <FL/platform.H>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/math.h>

#include <FL/Fl_Vk_Window.H>
#include <FL/Fl_Vk_Utils.H>

#include <iostream>
#include <string>

namespace
{
    using namespace pxr;
    
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

#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfArray.h>
#include <OpenEXR/ImfHeader.h>
#include <iostream>

using namespace Imf;
using namespace Imath;

void saveHalfRGB(const char* filename,
                 const std::shared_ptr<tl::image::Image> image) {

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
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
}

class usd_window : public SUPERCLASS
{
public:
    usd_window(int X, int Y, int W, int H);

    void draw() override;
    void prepare() override {};

    void setUSDFile(const std::string&);

    TLRENDER_PRIVATE();
};

using namespace tl;

struct usd_window::Private
{
    file::Path path;
    UsdStageRefPtr stage = nullptr;
};

usd_window::usd_window(int X, int Y, int W, int H) :
    SUPERCLASS(X, Y, W, H),
    _p(new Private)
{
    mode(FL_RGB | FL_ALPHA | FL_DEPTH | FL_STENCIL | FL_OPENGL3);
}

void usd_window::draw()
{
    TLRENDER_P();

#if USE_GL
    if (!valid())
    {
        valid(1);
        glLoadIdentity();
        glViewport(0,0,pixel_w(),pixel_h());
    }
    glClearStencil(0);
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
#endif
    // const double timeCode = 
    //     request->time.rescaled_to(stage->GetTimeCodesPerSecond()).value();

    const double startTimeCode = p.stage->GetStartTimeCode();
    const double timeCode = startTimeCode;

    std::cerr << __LINE__ << std::endl;
    std::string cameraName;
    auto camera = getCamera(p.stage, cameraName);

    GfCamera gfCamera;
    if (camera)
    {
        gfCamera = camera.GetCamera(timeCode);
    }
    else
    {
        const TfTokenVector purposes(
            {UsdGeomTokens->default_, UsdGeomTokens->proxy});
        gfCamera = getCameraToFrameStage(p.stage, timeCode, purposes);
    }
    
    float aspectRatio = gfCamera.GetAspectRatio();
    if (GfIsClose(aspectRatio, 0.F, 1e-4))
    {
        aspectRatio = 1.F;
    }
    const GfFrustum frustum = gfCamera.GetFrustum();
    const GfVec3d cameraPos = frustum.GetPosition();
    
    std::shared_ptr<image::Image> image;
    const size_t renderWidth = pixel_w();
    const size_t renderHeight = renderWidth / aspectRatio;

#if USE_GL
    const bool gpuEnabled = true;
    auto engine = std::make_shared<UsdImagingGLEngine>(
        HdDriver(), TfToken(), gpuEnabled);
    std::cerr << __LINE__ << std::endl;
    engine->SetCameraState(frustum.ComputeViewMatrix(),
                           frustum.ComputeProjectionMatrix());
    engine->SetRenderViewport(GfVec4d(
                                  0.0, 0.0, static_cast<double>(renderWidth),
                                  static_cast<double>(renderHeight)));
    engine->SetRendererAov(HdAovTokens->color);
    
    // Setup a light.
    GlfSimpleLight cameraLight(GfVec4f(
                                   cameraPos[0], cameraPos[1], cameraPos[2], 1.F));
    cameraLight.SetAmbient(GfVec4f(.01F, .01F, .01F, 01.F));
    const GlfSimpleLightVector lights({cameraLight});
    
    // Setup a material.
    GlfSimpleMaterial material;
    material.SetAmbient(GfVec4f(0.2f, 0.2f, 0.2f, 1.0));
    material.SetSpecular(GfVec4f(0.1f, 0.1f, 0.1f, 1.0f));
    material.SetShininess(32.F);
    const GfVec4f ambient(0.01f, 0.01f, 0.01f, 1.0f);
    engine->SetLightingState(lights, material, ambient);

    // Options
    float complexity = 1.0;
    bool enableLighting = false;
    bool sRGB = false;
    
    // Render the frame.
    UsdImagingGLRenderParams renderParams;
    renderParams.frame = timeCode;
    renderParams.complexity = complexity;
    renderParams.drawMode = UsdImagingGLDrawMode::DRAW_GEOM_SMOOTH; //toUSD(drawMode);
    renderParams.enableLighting = enableLighting;
    renderParams.clearColor = GfVec4f(0.F, 0.F, 0.F, 0.F);
    renderParams.colorCorrectionMode = sRGB ? HdxColorCorrectionTokens->sRGB
                                       : HdxColorCorrectionTokens->disabled;
    const UsdPrim& pseudoRoot = p.stage->GetPseudoRoot();
    engine->Render(pseudoRoot, renderParams);

    uint32_t sleepTime = 10;
    while (1)
    {
        if (engine->IsConverged())
        {
            break;
        }
        else
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(sleepTime));
            sleepTime = std::min(100u, sleepTime + 5);
        }
    }

    std::cerr << __LINE__ << std::endl;
    
    // Copy the rendered frame.
    if (engine->GetGPUEnabled())
    {
        const auto colorTextureHandle = engine->GetAovTexture(
            HdAovTokens->color);
        if (colorTextureHandle)
        {
            size_t size = 0;
            const auto mappedColorTextureBuffer =
                HdStTextureUtils::HgiTextureReadback(engine->GetHgi(),
                                                     colorTextureHandle,
                                                     &size);
            // std::cout <<
            // colorTextureHandle->GetDescriptor().format
            // << std::endl;
            switch (HdxGetHioFormat(colorTextureHandle->GetDescriptor().format))
            {
            case HioFormat::HioFormatFloat16Vec4:
                image = image::Image::create(
                    renderWidth, renderHeight,
                    image::PixelType::RGBA_F16);
                memcpy(
                    image->getData(),
                    mappedColorTextureBuffer.get(),
                    image->getDataByteCount());
                break;
            default:
                break;
            }
        }
    }
    else
    {
        const auto colorRenderBuffer = engine->GetAovRenderBuffer(
            HdAovTokens->color);
        if (colorRenderBuffer)
        {
            colorRenderBuffer->Resolve();
            colorRenderBuffer->Map();
            switch (HdStHioConversions::GetHioFormat(
                        colorRenderBuffer->GetFormat()))
            {
            case HioFormat::HioFormatFloat16Vec4:
                image = image::Image::create(
                    renderWidth, renderHeight,
                    image::PixelType::RGBA_F16);
                memcpy(
                    image->getData(),
                    colorRenderBuffer->Map(),
                    image->getDataByteCount());
                break;
            default:
                break;
            }
        }
    }
#else
    image = image::Image::create(
        renderWidth, renderHeight,
        image::PixelType::RGBA_F16);

    GfMatrix4d matrix;
    UsdGeomXformCache xformCache(timeCode);
    for (const auto& gprim : p.stage->Traverse())
    {
        std::cerr << gprim.GetTypeName() << " " << gprim.GetName() << std::endl;
        matrix = xformCache.GetLocalToWorldTransform(gprim);
        std::cerr << "\t" << matrix << std::endl;
        if (gprim.IsA<UsdGeomMesh>())
        {
            UsdGeomMesh out = UsdGeomMesh(gprim);
        }
        else if (gprim.IsA<UsdGeomNurbsPatch>())
        {
            UsdGeomNurbsPatch out = UsdGeomNurbsPatch(gprim);
        }
        else if (gprim.IsA<UsdGeomNurbsCurves>())
        {
            UsdGeomNurbsCurves out = UsdGeomNurbsCurves(gprim);
        }
        else if (gprim.IsA<UsdGeomBasisCurves>())
        {
            UsdGeomBasisCurves out = UsdGeomBasisCurves(gprim);
        }
    }
#endif
    
    saveHalfRGB("/home/gga/test.exr", image);
    exit(0);  // \@todo: remove
}

void usd_window::setUSDFile(const std::string& fileName)
{
    TLRENDER_P();
    
    p.path = file::Path(fileName);
    
    TfDiagnosticMgr::GetInstance().SetQuiet(false);

    
    p.stage = UsdStage::Open(fileName);
    
    const double startTimeCode = p.stage->GetStartTimeCode();
    const double endTimeCode = p.stage->GetEndTimeCode();
    const double timeCodesPerSecond =  p.stage->GetTimeCodesPerSecond();
    

}

int main(int argc, char **argv) {
    if (argc != 2)
    {
        std::cerr << argv[0] << " <file.usd>" << std::endl;
        exit(1);
    }
    
    Fl::use_high_res_GL(1);

    Fl_Window window(300, 330);
  
    usd_window sw(10, 10, 280, 280);
    sw.setUSDFile(argv[1]);

    window.end();
    window.show();
        
    return Fl::run();
}
