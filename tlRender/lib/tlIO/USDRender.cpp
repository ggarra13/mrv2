// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/USDPrivate.h>

#include <tlCore/File.h>
#include <tlCore/LRUCache.h>
#include <tlCore/LogSystem.h>
#include <tlCore/StringFormat.h>

#include <tlGL/GLFWWindow.h>

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
#include <pxr/usdImaging/usdAppUtils/frameRecorder.h>
#include <pxr/imaging/hd/renderBuffer.h>
#include <pxr/imaging/hdSt/hioConversions.h>
#include <pxr/imaging/hdSt/textureUtils.h>
#include <pxr/imaging/hdx/tokens.h>
#include <pxr/imaging/hdx/types.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

using namespace PXR_NS;

namespace tl
{
    namespace usd
    {
        struct Render::Private
        {
            std::shared_ptr<io::Cache> cache;
            std::weak_ptr<log::System> logSystem;

            GLFWwindow* glfwWindow = nullptr;

            struct InfoRequest
            {
                int64_t id = -1;
                file::Path path;
                std::promise<io::Info> promise;
            };

            struct Request
            {
                int64_t id = -1;
                file::Path path;
                otime::RationalTime time = time::invalidTime;
                io::Options options;
                std::promise<io::VideoData> promise;
            };

            struct Mutex
            {
                std::list<std::shared_ptr<InfoRequest> > infoRequests;
                std::list<std::shared_ptr<Request> > requests;
                bool stopped = false;
                std::mutex mutex;
            };
            Mutex mutex;

            struct StageCacheItem
            {
                UsdStageRefPtr stage;
                std::shared_ptr<UsdImagingGLEngine> engine;
            };

            struct DiskCacheItem
            {
                ~DiskCacheItem() { file::rm(fileName); }

                std::string fileName;
            };

            struct Thread
            {
                memory::LRUCache<std::string, StageCacheItem> stageCache;
                memory::LRUCache<std::string, std::shared_ptr<DiskCacheItem> >
                    diskCache;
                std::string tempDir;
                std::chrono::steady_clock::time_point logTimer;
                std::condition_variable cv;
                std::thread thread;
                std::atomic<bool> running;
            };
            Thread thread;
        };

        void Render::_init(
            const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            TLRENDER_P();

            p.cache = cache;
            p.logSystem = logSystem;

#if defined(__APPLE__)
            const int glVersionMinor = 1;
            const int glProfile = GLFW_OPENGL_CORE_PROFILE;
#else  //__APPLE__
            const int glVersionMinor = 5;
            const int glProfile = GLFW_OPENGL_COMPAT_PROFILE;
#endif //__APPLE__
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, glVersionMinor);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE, glProfile);
            glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
            gl::windowHint(GLFW_DOUBLEBUFFER, GLFW_FALSE);
            p.glfwWindow =
                glfwCreateWindow(1, 1, "tl::usd::Render", NULL, NULL);
            if (!p.glfwWindow)
            {
                throw std::runtime_error("Cannot create window");
            }

            p.thread.logTimer = std::chrono::steady_clock::now();
            p.thread.running = true;
            p.thread.thread = std::thread(
                [this]
                {
                    TLRENDER_P();
                    glfwMakeContextCurrent(p.glfwWindow);
                    _run();
                    p.thread.stageCache.clear();
                    p.thread.diskCache.clear();
                    glfwMakeContextCurrent(nullptr);
                    _finish();
                });

            if (auto logSystem = p.logSystem.lock())
            {
                std::vector<std::string> renderers;
                for (const auto& id : UsdImagingGLEngine::GetRendererPlugins())
                {
                    renderers.push_back(
                        UsdImagingGLEngine::GetRendererDisplayName(id));
                }
                logSystem->print(
                    "tl::usd::Render", string::Format("\n"
                                                      "    Renderers: {0}")
                                           .arg(string::join(renderers, ", ")));
            }
        }

        Render::Render() :
            _p(new Private)
        {
        }

        Render::~Render()
        {
            TLRENDER_P();
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.thread.running = false;
            }
            p.thread.cv.notify_one();
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
            if (p.glfwWindow)
            {
                glfwDestroyWindow(p.glfwWindow);
            }
        }

        std::shared_ptr<Render> Render::create(
            const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Render>(new Render);
            out->_init(cache, logSystem);
            return out;
        }

        std::future<io::Info>
        Render::getInfo(int64_t id, const file::Path& path)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::InfoRequest>();
            request->id = id;
            request->path = path;
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.infoRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::Info());
            }
            return future;
        }

        std::future<io::VideoData> Render::render(
            int64_t id, const file::Path& path, const otime::RationalTime& time,
            const io::Options& options)
        {
            TLRENDER_P();
            auto request = std::make_shared<Private::Request>();
            request->id = id;
            request->path = path;
            request->time = time;
            request->options = options;
            auto future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.requests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(io::VideoData());
            }
            return future;
        }

        void Render::cancelRequests(int64_t id)
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
            std::list<std::shared_ptr<Private::Request> > requests;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                auto i = p.mutex.infoRequests.begin();
                while (i != p.mutex.infoRequests.end())
                {
                    if (id == (*i)->id)
                    {
                        infoRequests.push_back(*i);
                        i = p.mutex.infoRequests.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
                auto j = p.mutex.requests.begin();
                while (j != p.mutex.requests.end())
                {
                    if (id == (*j)->id)
                    {
                        requests.push_back(*j);
                        j = p.mutex.requests.erase(j);
                    }
                    else
                    {
                        ++j;
                    }
                }
            }
            for (auto& request : infoRequests)
            {
                request->promise.set_value(io::Info());
            }
            for (auto& request : requests)
            {
                request->promise.set_value(io::VideoData());
            }
        }

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

            UsdImagingGLDrawMode toUSD(DrawMode value)
            {
                const std::vector<UsdImagingGLDrawMode> data = {
                    UsdImagingGLDrawMode::DRAW_POINTS,
                    UsdImagingGLDrawMode::DRAW_WIREFRAME,
                    UsdImagingGLDrawMode::DRAW_WIREFRAME_ON_SURFACE,
                    UsdImagingGLDrawMode::DRAW_SHADED_FLAT,
                    UsdImagingGLDrawMode::DRAW_SHADED_SMOOTH,
                    UsdImagingGLDrawMode::DRAW_GEOM_ONLY,
                    UsdImagingGLDrawMode::DRAW_GEOM_FLAT,
                    UsdImagingGLDrawMode::DRAW_GEOM_SMOOTH};
                return data[static_cast<size_t>(value)];
            };
        } // namespace

        void Render::_open(
            const std::string& fileName, UsdStageRefPtr& stage,
            std::shared_ptr<UsdImagingGLEngine>& engine)
        {
            TLRENDER_P();
            stage = UsdStage::Open(fileName);
            const bool gpuEnabled = true;
            engine = std::make_shared<UsdImagingGLEngine>(
                HdDriver(), TfToken(), gpuEnabled);
            if (stage && engine)
            {
                if (auto logSystem = p.logSystem.lock())
                {
                    const std::string renderer =
                        UsdImagingGLEngine::GetRendererDisplayName(
                            engine->GetCurrentRendererId());
                    std::vector<std::string> aovs;
                    for (const auto& i : engine->GetRendererAovs())
                    {
                        aovs.push_back(i.GetText());
                    }
                    logSystem->print(
                        "tl::usd::Render",
                        string::Format("\n"
                                       "    File name: {0}\n"
                                       "    Time code: {1}-{2}:{3}\n"
                                       "    GPU enabled: {4}\n"
                                       "    Renderer ID: {5}\n"
                                       "    Renderer AOVs available: {6}")
                            .arg(fileName)
                            .arg(stage->GetStartTimeCode())
                            .arg(stage->GetEndTimeCode())
                            .arg(stage->GetTimeCodesPerSecond())
                            .arg(engine->GetGPUEnabled())
                            .arg(renderer)
                            .arg(string::join(aovs, ", ")));
                }
            }
        }

        void Render::_run()
        {
            TLRENDER_P();

            TfDiagnosticMgr::GetInstance().SetQuiet(true);

            const TfTokenVector purposes(
                {UsdGeomTokens->default_, UsdGeomTokens->proxy});

            size_t stageCacheCount = 10;
            size_t diskCacheByteCount = 0;
            int renderWidth = 1920;
            while (p.thread.running)
            {
                // Check requests.
                std::shared_ptr<Private::InfoRequest> infoRequest;
                std::shared_ptr<Private::Request> request;
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    p.thread.cv.wait(
                        lock, [this] {
                            return ( !_p->mutex.infoRequests.empty() ||
                                     !_p->mutex.requests.empty() ||
                                     !_p->thread.running );
                        });

                    if (!p.thread.running)
                        break;
                    
                    if (!p.mutex.infoRequests.empty())
                    {
                        infoRequest = p.mutex.infoRequests.front();
                        p.mutex.infoRequests.pop_front();
                    }
                    if (!p.mutex.requests.empty())
                    {
                        request = p.mutex.requests.front();
                        p.mutex.requests.pop_front();
                    }                    
                }

                // Set options.
                io::Options ioOptions;
                if (request)
                {
                    ioOptions = request->options;
                }
                auto i = ioOptions.find("USD/stageCacheCount");
                if (i != ioOptions.end())
                {
                    stageCacheCount = std::atoll(i->second.c_str());
                }
                i = ioOptions.find("USD/diskCacheByteCount");
                if (i != ioOptions.end())
                {
                    diskCacheByteCount = std::atoll(i->second.c_str());
                }
                p.thread.stageCache.setMax(stageCacheCount);
                p.thread.diskCache.setMax(diskCacheByteCount);
                if (diskCacheByteCount > 0 && p.thread.tempDir.empty())
                {
                    p.thread.tempDir = file::createTempDir();
                    if (auto logSystem = p.logSystem.lock())
                    {
                        logSystem->print(
                            "tl::usd::Render",
                            string::Format("\n"
                                           "    Temp directory: {0}\n"
                                           "    Disk cache: {1}GB")
                                .arg(p.thread.tempDir)
                                .arg(diskCacheByteCount / memory::gigabyte));
                    }
                }
                else if (0 == diskCacheByteCount && !p.thread.tempDir.empty())
                {
                    p.thread.tempDir = std::string();
                }

                // Handle information requests.
                i = ioOptions.find("USD/renderWidth");
                if (i != ioOptions.end())
                {
                    renderWidth = std::atoi(i->second.c_str());
                }
                std::string cameraName;
                i = ioOptions.find("USD/cameraName");
                if (i != ioOptions.end())
                {
                    cameraName = i->second;
                }
                if (infoRequest)
                {
                    const std::string fileName =
                        infoRequest->path.getFileName(true);
                    Private::StageCacheItem stageCacheItem;
                    if (!p.thread.stageCache.get(fileName, stageCacheItem))
                    {
                        _open(
                            fileName, stageCacheItem.stage,
                            stageCacheItem.engine);
                        p.thread.stageCache.add(fileName, stageCacheItem);
                    }
                    io::Info info;
                    if (stageCacheItem.stage)
                    {
                        const double startTimeCode =
                            stageCacheItem.stage->GetStartTimeCode();
                        const double endTimeCode =
                            stageCacheItem.stage->GetEndTimeCode();
                        const double timeCodesPerSecond =
                            stageCacheItem.stage->GetTimeCodesPerSecond();
                        GfCamera gfCamera;
                        auto camera =
                            getCamera(stageCacheItem.stage, cameraName);
                        if (camera)
                        {
                            // std::cout << fileName << " camera: " <<
                            //     camera.GetPath().GetAsToken().GetText() <<
                            //     std::endl;
                            gfCamera = camera.GetCamera(startTimeCode);
                        }
                        else
                        {
                            gfCamera = getCameraToFrameStage(
                                stageCacheItem.stage, startTimeCode, purposes);
                        }
                        float aspectRatio = gfCamera.GetAspectRatio();
                        if (GfIsClose(aspectRatio, 0.F, 1e-4))
                        {
                            aspectRatio = 1.F;
                        }
                        info.video.push_back(image::Info(
                            renderWidth, renderWidth / aspectRatio,
                            image::PixelType::RGBA_F16));
                        info.videoTime = otime::TimeRange::
                            range_from_start_end_time_inclusive(
                                otime::RationalTime(
                                    startTimeCode, timeCodesPerSecond),
                                otime::RationalTime(
                                    endTimeCode, timeCodesPerSecond));
                        // std::cout << fileName << " range: " << info.videoTime
                        // << std::endl;
                    }
                    infoRequest->promise.set_value(info);
                }

                // Check the I/O cache.
                io::VideoData videoData;
                if (request && p.cache)
                {
                    const std::string cacheKey = io::getVideoCacheKey(
                        request->path, request->time, ioOptions, {});
                    if (p.cache->getVideo(cacheKey, videoData))
                    {
                        image::Tags tags;
                        tags["otioClipName"] = request->path.get();
                        {
                            std::stringstream ss;
                            ss << request->time;
                            tags["otioClipTime"] = ss.str();
                        }
                        videoData.image->setTags(tags);

                        request->promise.set_value(videoData);
                        request.reset();
                    }
                }

                // Check the disk cache.
                if (request)
                {
                    std::shared_ptr<Private::DiskCacheItem> diskCacheItem;
                    const std::string cacheKey = io::getVideoCacheKey(
                        request->path, request->time, ioOptions, {});
                    if (diskCacheByteCount > 0 &&
                        p.thread.diskCache.get(cacheKey, diskCacheItem))
                    {
                        std::shared_ptr<image::Image> image;
                        try
                        {
                            // std::cout << "read temp file: " <<
                            // diskCacheItem->fileName << std::endl;
                            auto fileIO = file::FileIO::create(
                                diskCacheItem->fileName, file::Mode::Read);
                            uint16_t w = 0;
                            uint16_t h = 0;
                            fileIO->readU16(&w);
                            fileIO->readU16(&h);
                            uint32_t pixelType = 0;
                            fileIO->readU32(&pixelType);
                            image = image::Image::create(
                                w, h, static_cast<image::PixelType>(pixelType));
                            fileIO->read(
                                image->getData(), image->getDataByteCount());
                        }
                        catch (const std::exception& e)
                        {
                            // std::cout << e.what() << std::endl;
                            if (auto logSystem = p.logSystem.lock())
                            {
                                const std::string id =
                                    string::Format("tl::usd::Render ({0}: {1})")
                                        .arg(__FILE__)
                                        .arg(__LINE__);
                                logSystem->print(
                                    id, e.what(), log::Type::Error);
                            }
                        }

                        videoData.time = request->time;

                        image::Tags tags;
                        tags["otioClipName"] = request->path.get();
                        {
                            std::stringstream ss;
                            ss << request->time;
                            tags["otioClipTime"] = ss.str();
                        }
                        image->setTags(tags);

                        videoData.image = image;
                        request->promise.set_value(videoData);

                        if (p.cache)
                        {
                            p.cache->addVideo(cacheKey, videoData);
                        }

                        request.reset();
                    }
                }

                // Handle requests.
                if (request)
                {
                    std::shared_ptr<image::Image> image;
                    const std::string cacheKey = io::getVideoCacheKey(
                        request->path, request->time, ioOptions, {});
                    try
                    {
                        // Check the stage cache for a previously opened stage.
                        const std::string fileName =
                            request->path.getFileName(true);
                        Private::StageCacheItem stageCacheItem;
                        if (!p.thread.stageCache.get(fileName, stageCacheItem))
                        {
                            _open(
                                fileName, stageCacheItem.stage,
                                stageCacheItem.engine);
                            p.thread.stageCache.add(fileName, stageCacheItem);
                        }
                        if (stageCacheItem.stage && stageCacheItem.engine)
                        {
                            const double timeCode =
                                request->time
                                    .rescaled_to(stageCacheItem.stage
                                                     ->GetTimeCodesPerSecond())
                                    .value();
                            // std::cout << fileName << " timeCode: " <<
                            // timeCode << std::endl;

                            // Get options.
                            i = ioOptions.find("USD/renderWidth");
                            if (i != ioOptions.end())
                            {
                                renderWidth = std::atoi(i->second.c_str());
                            }
                            float complexity = 1.F;
                            i = ioOptions.find("USD/complexity");
                            if (i != ioOptions.end())
                            {
                                complexity = std::atof(i->second.c_str());
                            }
                            DrawMode drawMode = DrawMode::ShadedSmooth;
                            i = ioOptions.find("USD/drawMode");
                            if (i != ioOptions.end())
                            {
                                std::stringstream ss(i->second);
                                ss >> drawMode;
                            }
                            bool enableLighting = true;
                            i = ioOptions.find("USD/enableLighting");
                            if (i != ioOptions.end())
                            {
                                enableLighting = std::atoi(i->second.c_str());
                            }
                            bool sRGB = true;
                            i = ioOptions.find("USD/sRGB");
                            if (i != ioOptions.end())
                            {
                                sRGB = std::atoi(i->second.c_str());
                            }

                            // Setup the camera.
                            std::string cameraName;
                            i = ioOptions.find("USD/cameraName");
                            if (i != ioOptions.end())
                            {
                                cameraName = i->second;
                            }
                            GfCamera gfCamera;
                            auto camera =
                                getCamera(stageCacheItem.stage, cameraName);
                            if (camera)
                            {
                                gfCamera = camera.GetCamera(timeCode);
                            }
                            else
                            {
                                gfCamera = getCameraToFrameStage(
                                    stageCacheItem.stage, timeCode, purposes);
                            }
                            const GfFrustum frustum = gfCamera.GetFrustum();
                            const GfVec3d cameraPos = frustum.GetPosition();
                            stageCacheItem.engine->SetCameraState(
                                frustum.ComputeViewMatrix(),
                                frustum.ComputeProjectionMatrix());
                            float aspectRatio = gfCamera.GetAspectRatio();
                            if (GfIsClose(aspectRatio, 0.F, 1e-4))
                            {
                                aspectRatio = 1.F;
                            }
                            const size_t renderHeight =
                                renderWidth / aspectRatio;
                            stageCacheItem.engine->SetRenderViewport(GfVec4d(
                                0.0, 0.0, static_cast<double>(renderWidth),
                                static_cast<double>(renderHeight)));

                            // for (const auto& token :
                            // stageCacheItem.engine->GetRendererAovs())
                            //{
                            //     std::cout << token.GetText() << std::endl;
                            // }
                            stageCacheItem.engine->SetRendererAov(
                                HdAovTokens->color);

                            // Setup a light.
                            GlfSimpleLight cameraLight(GfVec4f(
                                cameraPos[0], cameraPos[1], cameraPos[2], 1.F));
                            cameraLight.SetAmbient(
                                GfVec4f(.01F, .01F, .01F, 01.F));
                            const GlfSimpleLightVector lights({cameraLight});

                            // Setup a material.
                            GlfSimpleMaterial material;
                            material.SetAmbient(GfVec4f(0.2f, 0.2f, 0.2f, 1.0));
                            material.SetSpecular(
                                GfVec4f(0.1f, 0.1f, 0.1f, 1.0f));
                            material.SetShininess(32.F);
                            const GfVec4f ambient(0.01f, 0.01f, 0.01f, 1.0f);
                            stageCacheItem.engine->SetLightingState(
                                lights, material, ambient);

                            // Render the frame.
                            UsdImagingGLRenderParams renderParams;
                            renderParams.frame = timeCode;
                            renderParams.complexity = complexity;
                            renderParams.drawMode = toUSD(drawMode);
                            renderParams.enableLighting = enableLighting;
                            renderParams.clearColor =
                                GfVec4f(0.F, 0.F, 0.F, 0.F);
                            renderParams.colorCorrectionMode =
                                sRGB ? HdxColorCorrectionTokens->sRGB
                                     : HdxColorCorrectionTokens->disabled;
                            const UsdPrim& pseudoRoot =
                                stageCacheItem.stage->GetPseudoRoot();
                            unsigned int sleepTime = 10;
                            while (p.thread.running)
                            {
                                stageCacheItem.engine->Render(
                                    pseudoRoot, renderParams);
                                if (stageCacheItem.engine->IsConverged())
                                {
                                    break;
                                }
                                else
                                {
                                    std::this_thread::sleep_for(
                                        std::chrono::milliseconds(sleepTime));
                                    sleepTime = std::min(100u, sleepTime + 5);
                                }
                            }

                            // Copy the rendered frame.
                            if (stageCacheItem.engine->GetGPUEnabled())
                            {
                                const auto colorTextureHandle =
                                    stageCacheItem.engine->GetAovTexture(
                                        HdAovTokens->color);
                                if (colorTextureHandle)
                                {
                                    size_t size = 0;
                                    const auto mappedColorTextureBuffer =
                                        HdStTextureUtils::HgiTextureReadback(
                                            stageCacheItem.engine->GetHgi(),
                                            colorTextureHandle, &size);
                                    // std::cout <<
                                    // colorTextureHandle->GetDescriptor().format
                                    // << std::endl;
                                    switch (HdxGetHioFormat(
                                        colorTextureHandle->GetDescriptor()
                                            .format))
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
                                const auto colorRenderBuffer =
                                    stageCacheItem.engine->GetAovRenderBuffer(
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

                            // Add the rendered frame to the disk cache.
                            if (diskCacheByteCount > 0 && image)
                            {
                                auto diskCacheItem =
                                    std::make_shared<Private::DiskCacheItem>();
                                diskCacheItem->fileName =
                                    string::Format("{0}/{1}.img")
                                        .arg(p.thread.tempDir)
                                        .arg(diskCacheItem);
                                // std::cout << "write temp file: " <<
                                // diskCacheItem->fileName << std::endl;
                                auto tempFile = file::FileIO::create(
                                    diskCacheItem->fileName, file::Mode::Write);
                                tempFile->writeU16(image->getWidth());
                                tempFile->writeU16(image->getHeight());
                                tempFile->writeU32(static_cast<uint32_t>(
                                    image->getPixelType()));
                                const size_t byteCount =
                                    image->getDataByteCount();
                                tempFile->write(image->getData(), byteCount);
                                p.thread.diskCache.add(
                                    cacheKey, diskCacheItem, byteCount);
                            }
                        }
                    }
                    catch (const std::exception& e)
                    {
                        // std::cout << e.what() << std::endl;
                        if (auto logSystem = p.logSystem.lock())
                        {
                            const std::string id =
                                string::Format("tl::usd::Render ({0}: {1})")
                                    .arg(__FILE__)
                                    .arg(__LINE__);
                            logSystem->print(id, e.what(), log::Type::Error);
                        }
                    }

                    videoData.time = request->time;
                    videoData.image = image;
                    request->promise.set_value(videoData);

                    if (p.cache)
                    {
                        p.cache->addVideo(cacheKey, videoData);
                    }
                }

                // Logging.
                {
                    const auto now = std::chrono::steady_clock::now();
                    const std::chrono::duration<float> diff =
                        now - p.thread.logTimer;
                    if (diff.count() > 10.F)
                    {
                        p.thread.logTimer = now;
                        if (auto logSystem = p.logSystem.lock())
                        {
                            size_t requestsSize = 0;
                            {
                                std::unique_lock<std::mutex> lock(
                                    p.mutex.mutex);
                                requestsSize = p.mutex.requests.size();
                            }
                            logSystem->print(
                                "tl::usd::Render",
                                string::Format("\n"
                                               "    Requests: {0}\n"
                                               "    Stage cache: {1}/{2}\n"
                                               "    Disk cache: {3}/{4}GB")
                                    .arg(requestsSize)
                                    .arg(p.thread.stageCache.getSize())
                                    .arg(p.thread.stageCache.getMax())
                                    .arg(
                                        p.thread.diskCache.getSize() /
                                        memory::gigabyte)
                                    .arg(
                                        p.thread.diskCache.getMax() /
                                        memory::gigabyte));
                        }
                    }
                }
            }
        }

        void Render::_finish()
        {
            TLRENDER_P();
            std::list<std::shared_ptr<Private::InfoRequest> > infoRequests;
            std::list<std::shared_ptr<Private::Request> > requests;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.stopped = true;
                infoRequests = std::move(p.mutex.infoRequests);
                requests = std::move(p.mutex.requests);
            }
            for (auto& request : infoRequests)
            {
                request->promise.set_value(io::Info());
            }
            for (auto& request : requests)
            {
                request->promise.set_value(io::VideoData());
            }
        }
    } // namespace usd
} // namespace tl
