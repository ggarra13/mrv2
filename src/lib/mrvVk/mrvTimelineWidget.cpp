// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrViewer.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrvNetwork/mrvTCP.h"

#include "mrvEdit/mrvEditCallbacks.h"
#include "mrvEdit/mrvEditUtil.h"

#include "mrvUI/mrvDesktop.h"

#include "mrvFl/mrvIO.h"

#include "mrvVk/mrvVkShadersBinary.h"
#include "mrvVk/mrvTimelineWidget.h"

#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvHotkey.h"
#include "mrvCore/mrvTimeObject.h"

#include <tlTimelineUI/TimelineWidget.h>

#include <tlTimelineVk/Render.h>
#include <tlTimelineVk/RenderShadersBinary.h>

#include <tlUI/IClipboard.h>
#include <tlUI/IWindow.h>

#include <tlVk/Init.h>
#include <tlVk/Mesh.h>
#include <tlVk/OffscreenBuffer.h>
#include <tlVk/Shader.h>
#include <tlVk/ShaderBindingSet.h>

#include <FL/Fl_Box.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl.H>
#include <FL/names.h>


namespace mrv
{
    namespace
    {
        const int kTHUMB_WIDTH = 96;
        const int kTHUMB_HEIGHT = 64;
        const int kLABEL_SIZE = 16;
        const int kWINDOW_BORDERS = 2;
        const int kBOX_BORDERS = 2;

        const double kTimeout = 0.008; // approx. 120 fps
        const char* kModule = "timeline";
    } // namespace

    namespace
    {
        int getIndex(
            const otio::SerializableObject::Retainer<otio::Composable>&
                composable)
        {
            int out = -1;
            if (composable && composable->parent())
            {
                const auto& children = composable->parent()->children();
                for (int i = 0; i < children.size(); ++i)
                {
                    if (composable == children[i].value)
                    {
                        out = i;
                        break;
                    }
                }
            }
            return out;
        }
    } // namespace

    namespace
    {
        // These classes (TimelineWindow and Clipboard) are needed to act
        // from FLTK to Darby's UI translations.
        class TimelineWindow : public ui::IWindow
        {
            TLRENDER_NON_COPYABLE(TimelineWindow);

        public:
            void _init(const std::shared_ptr<system::Context>& context)
            {
                IWindow::_init(
                    "tl::anonymous::TimelineWindow", context, nullptr);
            }

            TimelineWindow() {}

        public:
            virtual ~TimelineWindow() {}

            static std::shared_ptr<TimelineWindow>
            create(const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<TimelineWindow>(new TimelineWindow);
                out->_init(context);
                return out;
            }

            bool key(ui::Key key, bool press, int modifiers)
            {
                return _key(key, press, modifiers);
            }

            void text(const std::string& text) { _text(text); }

            void cursorEnter(bool enter) { _cursorEnter(enter); }

            void cursorPos(const math::Vector2i& value) { _cursorPos(value); }

            void mouseButton(int button, bool press, int modifiers)
            {
                _mouseButton(button, press, modifiers);
            }

            void scroll(const math::Vector2f& value, int modifiers)
            {
                _scroll(value, modifiers);
            }

            void setGeometry(const math::Box2i& value) override
            {
                IWindow::setGeometry(value);
                for (const auto& i : _children)
                {
                    i->setGeometry(value);
                }
            }
        };

        class Clipboard : public ui::IClipboard
        {
            TLRENDER_NON_COPYABLE(Clipboard);

        public:
            void _init(const std::shared_ptr<system::Context>& context)
            {
                IClipboard::_init(context);
            }

            Clipboard() {}

        public:
            ~Clipboard() override {}

            static std::shared_ptr<Clipboard>
            create(const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<Clipboard>(new Clipboard);
                out->_init(context);
                return out;
            }

            std::string getText() const override
            {
                std::string text;
                if (Fl::event_text())
                    text = Fl::event_text();
                return text;
            }

            void setText(const std::string& value) override
            {
                Fl::copy(value.c_str(), value.size());
            }
        };
    } // namespace

    namespace vulkan
    {

        struct TimelineWidget::Private
        {
            std::weak_ptr<system::Context> context;

            ViewerUI* ui = nullptr;
            Fl_Window* topWindow = nullptr;

            
            TimelinePlayer* player = nullptr;

            std::weak_ptr<timelineui_vk::ThumbnailSystem> thumbnailSystem;

            struct ThumbnailData
            {
                timelineui_vk::ThumbnailRequest request;
                std::shared_ptr<image::Image> image;
            };
            ThumbnailData thumbnail;

            Fl_Double_Window* thumbnailWindow = nullptr; // thumbnail window
            Fl_Box* box = nullptr;

            timeline::OCIOOptions ocioOptions;
            timeline::LUTOptions lutOptions;

            mrv::TimeUnits units = mrv::TimeUnits::Timecode;

            // Render data
            std::shared_ptr<ui::Style> style;
            std::shared_ptr<ui::IconLibrary> iconLibrary;
            std::shared_ptr<image::FontSystem> fontSystem;
            std::shared_ptr<Clipboard> clipboard;
            std::shared_ptr<timeline_vlk::Render> render;
            timelineui_vk::DisplayOptions displayOptions;
            std::shared_ptr<timelineui_vk::TimelineWidget> timelineWidget;
            std::shared_ptr<TimelineWindow> timelineWindow;
            std::shared_ptr<tl::vlk::Shader> shader;
            std::shared_ptr<tl::vlk::OffscreenBuffer> buffer;
            std::shared_ptr<vlk::VBO> vbo;
            std::shared_ptr<vlk::VAO> vao;
            std::chrono::steady_clock::time_point mouseWheelTimer;
            VkPipelineLayout pipeline_layout = VK_NULL_HANDLE;

            //! Flags
            bool draggingClip = false;
            bool continueReversePlaying = false;

            //! Observers
            std::shared_ptr<observer::ValueObserver<timeline::PlayerCacheInfo> >
                cacheInfoObserver;

            std::vector<otime::RationalTime> annotationTimes;
            otime::TimeRange timeRange = time::invalidTimeRange;
        };

        TimelineWidget::TimelineWidget(
            int X, int Y, int W, int H, const char* L) :
            VkWindow(X, Y, W, H, L),
            _p(new Private)
        {
            int fl_double = FL_DOUBLE;
            mode(FL_RGB | FL_ALPHA | fl_double);
            // m_debugSync = true;
        }

        void TimelineWidget::init_colorspace()
        {
            Fl_Vk_Window::init_colorspace();

            colorSpace() = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
            format() = VK_FORMAT_B8G8R8A8_UNORM;
        }
        
        void TimelineWidget::setContext(
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<timeline::TimeUnitsModel>& timeUnitsModel,
            ViewerUI* ui)
        {
            TLRENDER_P();

            p.context = context;

            p.ui = ui;
            p.topWindow = ui->uiMain;

            if (!p.thumbnailWindow)
            {
                _createThumbnailWindow();
            }


            p.style = ui::Style::create(context);
            p.iconLibrary = ui::IconLibrary::create(context);
            p.fontSystem = image::FontSystem::create(context);
            p.clipboard = Clipboard::create(context);

            p.timelineWindow = TimelineWindow::create(context);
            p.timelineWindow->setClipboard(p.clipboard);
            
            
            p.timelineWidget =
                timelineui_vk::TimelineWidget::create(timeUnitsModel, ctx,
                                                      context);
            p.timelineWidget->setEditable(false);
            p.timelineWidget->setFrameView(true);
            p.timelineWidget->setEditMode(timeline::EditMode::Move);
            p.timelineWidget->setScrollBarsVisible(false);
            p.timelineWidget->setMoveCallback(
                std::bind(
                    &mrv::vulkan::TimelineWidget::moveCallback, this,
                    std::placeholders::_1));

            p.timelineWidget->setParent(p.timelineWindow);
            
            auto settings = ui->app->settings();
            
            timelineui_vk::DisplayOptions displayOptions;
            displayOptions.trackInfo =
                settings->getValue<bool>("Timeline/TrackInfo");
            displayOptions.clipInfo =
                settings->getValue<bool>("Timeline/ClipInfo");
            p.timelineWidget->setDisplayOptions(displayOptions);
                
            if (!context->getSystem<timelineui_vk::ThumbnailSystem>())
            {
                context->addSystem(timelineui_vk::ThumbnailSystem::create(context, ctx));
            }
            
            p.thumbnailSystem = context->getSystem<timelineui_vk::ThumbnailSystem>();

            setStopOnScrub(false);

            _styleUpdate();

            Fl::add_timeout(kTimeout, (Fl_Timeout_Handler)timerEvent_cb, this);
        }

        void TimelineWidget::setStyle(const std::shared_ptr<ui::Style>& style)
        {
            _p->style = style;
            _styleUpdate();
        }

        void TimelineWidget::hideThumbnail()
        {
            TLRENDER_P();
            if (!p.thumbnailWindow)
                return;
            p.thumbnailWindow->hide();
        }

        TimelineWidget::~TimelineWidget()
        {
            _cancelThumbnailRequests();
            Fl::remove_timeout(timerEvent_cb, this);
        }

        std::vector<const otio::Item* > TimelineWidget::getSelectedItems() const
        {
            return _p->timelineWidget->getSelectedItems();
        }
        
        bool TimelineWidget::isEditable() const
        {
            return _p->timelineWidget->isEditable();
        }

        void TimelineWidget::setEditable(bool value)
        {
            _p->timelineWidget->setEditable(value);
        }

        void TimelineWidget::setEditMode(const timeline::EditMode value)
        {
            _p->timelineWidget->setEditMode(value);
        }
        
        void TimelineWidget::setScrollBarsVisible(bool value)
        {
            _p->timelineWidget->setScrollBarsVisible(value);
        }

        void TimelineWidget::setScrollToCurrentFrame(bool value)
        {
            _p->timelineWidget->setScrollToCurrentFrame(value);
        }

        static void continue_playing_cb(TimelineWidget* t)
        {
            t->continuePlaying();
        }

        void TimelineWidget::continuePlaying()
        {
            TLRENDER_P();

            //
            // Thie observer will watch the cache and start a reverse playback
            // once it is filled.
            //
            p.cacheInfoObserver =
                observer::ValueObserver<timeline::PlayerCacheInfo>::create(
                    p.player->player()->observeCacheInfo(),
                    [this](const timeline::PlayerCacheInfo& value)
                    {
                        TLRENDER_P();

                        const auto& time = p.player->currentTime();

                        const auto& cache =
                            p.player->player()->observeCacheOptions()->get();
                        const auto& rate = time.rate();
                        const auto& readAhead =
                            cache.readAhead.rescaled_to(rate);
                        const auto& readBehind =
                            cache.readBehind.rescaled_to(rate);
                        const auto& timeRange = p.player->inOutRange();

                        auto startTime = time + readBehind;
                        auto endTime = time - readAhead;
                        if (endTime < timeRange.start_time())
                        {
                            auto diff = timeRange.start_time() - endTime;
                            endTime = timeRange.end_time_exclusive();
                            startTime = endTime - diff;

                            // Check in case of negative frames
                            if (startTime > endTime)
                                startTime = endTime;
                        }

                        // Sanity check just in case
                        if (endTime < startTime)
                        {
                            const auto tmp = endTime;
                            endTime = startTime;
                            startTime = tmp;
                        }

                        // Avoid rounding errors
                        endTime = endTime.floor();
                        startTime = startTime.ceil();

                        for (const auto& t : value.videoFrames)
                        {
                            if (t.start_time() <= startTime &&
                                t.end_time_exclusive() >= endTime)
                            {
                                p.ui->uiView->setPlayback(
                                    timeline::Playback::Reverse);
                                p.cacheInfoObserver.reset();
                                return;
                            }
                        }
                    });
        }

        int TimelineWidget::_seek()
        {
            TLRENDER_P();
            const int maxY = 48;
            const int Y = _toUI(Fl::event_y());
            const int X = _toUI(Fl::event_x());
            if ((Y < maxY && !p.timelineWidget->isDraggingClip()) ||
                !p.timelineWidget->isEditable())
            {
                p.timeRange = p.player->player()->getTimeRange();
                const auto& info = p.player->ioInfo();
                const auto& time = _posToTime(X);
                p.player->seek(time);
                // \@note: Jumping frames when playing can
                //         lead to seeking issues in tlRender when the images
                //         are not in cache. We stop the playback and set an
                //         FLTK timeout to watch on the cache until it is filled
                //         and we continue playing from there.
                //
                const auto& path = p.player->path();
                if (file::isMovie(path) &&
                    p.player->playback() == timeline::Playback::Reverse)
                {
                    p.player->stop();
                    Fl::add_timeout(
                        0.005, (Fl_Timeout_Handler)continue_playing_cb, this);
                }
                return 1;
            }
            else
            {
                p.draggingClip = p.timelineWidget->isDraggingClip();
                return 0;
            }
        }

        void TimelineWidget::_createThumbnailWindow()
        {
            TLRENDER_P();

            int X = kWINDOW_BORDERS;
            int Y = kWINDOW_BORDERS;
            int W = kTHUMB_WIDTH;
            int H = kTHUMB_HEIGHT;

            // Open a thumbnail window just above the timeline
            Fl_Group::current(p.topWindow);

            const int wX = X - kWINDOW_BORDERS;
            const int wY = Y - kWINDOW_BORDERS;
            const int wW = W + kWINDOW_BORDERS * 2 + kBOX_BORDERS * 2;
            const int wH =
                H + kWINDOW_BORDERS * 2 + kBOX_BORDERS * 2 + kLABEL_SIZE;

            const int bX = kBOX_BORDERS;
            const int bY = kBOX_BORDERS;
            const int bW = W + kBOX_BORDERS * 2;
            const int bH = H + kBOX_BORDERS * 2 + kLABEL_SIZE;

            p.thumbnailWindow = new Fl_Double_Window(wX, wY, wW, wH);
            p.thumbnailWindow->box(FL_FLAT_BOX);
            p.thumbnailWindow->color(0xffffffff);
            p.thumbnailWindow->clear_border();
            p.thumbnailWindow->begin();

            p.box = new Fl_Box(bX, bY, bW, bH);
            p.box->box(FL_FLAT_BOX);
            p.box->labelcolor(fl_contrast(p.box->labelcolor(), p.box->color()));
            p.thumbnailWindow->end();
            p.thumbnailWindow->resizable(0);
            p.thumbnailWindow->hide();
            Fl_Group::current(nullptr);
        }

        void
        TimelineWidget::_getThumbnailPosition(int& X, int& Y, int& W, int& H)
        {
            TLRENDER_P();

            W = kTHUMB_WIDTH;
            H = kTHUMB_HEIGHT;
            if (p.box)
            {
                Fl_Image* image = p.box->image();
                if (image)
                {
                    W = image->w();
                    H = image->h();
                }
            }

            X = Fl::event_x_root() - p.topWindow->x_root() - W / 2;
            if (X < kWINDOW_BORDERS)
                X = kWINDOW_BORDERS;

            int maxW = p.topWindow->w();
            if (p.thumbnailWindow)
                maxW -= p.thumbnailWindow->w();

            if (X > maxW)
                X = maxW;

            Y = y_root() - p.topWindow->y_root();

            // 8 here is the size of the dragbar.
            if (p.thumbnailWindow)
                Y -= (p.thumbnailWindow->h() + 8);

            if (Y < 0)
                Y = 0;
        }

        void TimelineWidget::repositionThumbnail()
        {
            TLRENDER_P();
            if (p.thumbnailWindow && Fl::belowmouse() == this && p.player &&
                p.ui->uiPrefs->uiPrefsTimelineThumbnails->value())
            {
                int X, Y, W, H;
                _getThumbnailPosition(X, Y, W, H);

                const int wX = X - kWINDOW_BORDERS;
                const int wY = Y - kWINDOW_BORDERS;
                const int wW = W + kWINDOW_BORDERS * 2 + kBOX_BORDERS * 2;
                const int wH =
                    H + kWINDOW_BORDERS * 2 + kBOX_BORDERS * 2 + kLABEL_SIZE;

                const int bX = kBOX_BORDERS;
                const int bY = kBOX_BORDERS;
                const int bW = W + kBOX_BORDERS * 2;
                const int bH = H + kBOX_BORDERS * 2 + kLABEL_SIZE;

                p.thumbnailWindow->resize(wX, wY, wW, wH);
                p.box->resize(bX, bY, bW, bH);
                p.thumbnailWindow->show(); // needed for Windows
            }
            else
            {
                hideThumbnail();
            }
        }

        int TimelineWidget::requestThumbnail(bool fetch)
        {
            TLRENDER_P();

            if (!p.player || !p.ui->uiPrefs->uiPrefsTimelineThumbnails->value())
            {
                hideThumbnail();
                return 0;
            }

            const auto player = p.player->player();
            p.timeRange = player->getTimeRange();

            char buffer[64];

            repositionThumbnail();

            file::Path path;
            auto model = p.ui->app->filesModel();
            auto Aitem = model->observeA()->get();
            if (Aitem)
                path = Aitem->path;
            else
                path = player->getPath();

            const image::Size size(kTHUMB_WIDTH, kTHUMB_HEIGHT);
            const auto& time = _posToTime(_toUI(Fl::event_x()));

            if (auto thumbnailSystem = p.thumbnailSystem.lock())
            {
                p.thumbnail.request =
                    thumbnailSystem->getThumbnail(path, size.h, time);
            }

            timeToText(buffer, time, _p->units);
            p.box->copy_label(buffer);
            return 1;
        }

        void TimelineWidget::_cancelThumbnailRequests()
        {
            TLRENDER_P();

            if (auto thumbnailSystem = p.thumbnailSystem.lock())
            {
                if (p.thumbnail.request.future.valid())
                {
                    thumbnailSystem->cancelRequests({p.thumbnail.request.id});
                }
            }
        }

        timelineui_vk::ItemOptions TimelineWidget::getItemOptions() const
        {
            return _p->timelineWidget->getItemOptions();
        }

        timelineui_vk::DisplayOptions TimelineWidget::getDisplayOptions() const
        {
            return _p->timelineWidget->getDisplayOptions();
        }

        void TimelineWidget::setTimelinePlayer(TimelinePlayer* player)
        {
            TLRENDER_P();
            if (player == p.player)
                return;
            p.player = player;

            p.annotationTimes.clear();

            if (player)
            {
                const auto innerPlayer = player->player();
                p.timeRange = innerPlayer->getTimeRange();
                p.timelineWidget->setPlayer(innerPlayer);

                // These calls are needed to refresh the timeline display
                // properly.
                _sizeHintEvent();
                _setGeometry();
                _clipEvent();
            }
            else
            {
                _cancelThumbnailRequests();
                p.box->image(nullptr);

                p.timeRange = time::invalidTimeRange;
                p.timelineWidget->setPlayer(nullptr);
            }
        }

        bool TimelineWidget::hasFrameView() const
        {
            return _p->timelineWidget->hasFrameView();
        }

        void TimelineWidget::setFrameView(bool value)
        {
            _p->timelineWidget->setFrameView(value);
        }

        void TimelineWidget::setScrollKeyModifier(ui::KeyModifier value)
        {
            _p->timelineWidget->setScrollKeyModifier(value);
        }

        void TimelineWidget::setStopOnScrub(bool value)
        {
            _p->timelineWidget->setStopOnScrub(value);
        }

        void TimelineWidget::setThumbnails(bool value)
        {
            TLRENDER_P();
            p.displayOptions.thumbnails = value;
            _p->timelineWidget->setDisplayOptions(p.displayOptions);
        }

        void TimelineWidget::setMouseWheelScale(float value)
        {
            _p->timelineWidget->setMouseWheelScale(value);
        }

        void
        TimelineWidget::setItemOptions(const timelineui_vk::ItemOptions& value)
        {
            _p->timelineWidget->setItemOptions(value);
        }

        void TimelineWidget::setDisplayOptions(
            const timelineui_vk::DisplayOptions& value)
        {
            _p->timelineWidget->setDisplayOptions(value);
        }

        void TimelineWidget::prepare_shaders()
        {
            TLRENDER_P();

            if (auto context = p.context.lock())
            {
                try
                {
                    if (!p.render)
                        p.render = timeline_vlk::Render::create(ctx, context);

                    if (!p.shader)
                    {
                        p.shader = vlk::Shader::create(
                            ctx,
                            timeline_vlk::Vertex2_spv,
                            timeline_vlk::Vertex2_spv_len,
                            textureFragment_spv,
                            textureFragment_spv_len,
                            "timeline p.shader");
                        math::Matrix4x4f pm;
                        p.shader->createUniform(
                            "transform.mvp", pm, vlk::kShaderVertex);
                        p.shader->addFBO("textureSampler");
                        p.shader->addPush("opacity", 1.0, vlk::kShaderFragment);
                        auto bindingSet = p.shader->createBindingSet();
                        p.shader->useBindingSet(bindingSet);
                    }
                }
                catch (const std::exception& e)
                {
                    context->log(
                        "mrv::mrvTimelineWidget", e.what(), log::Type::Error);
                }
                _sizeHintEvent();
                _setGeometry();
                _clipEvent();
            }
        }

        void TimelineWidget::prepare_mesh()
        {
            TLRENDER_P();

            const math::Size2i renderSize(pixel_w(), pixel_h());
            
            const auto& mesh =
                geom::box(math::Box2i(0, 0, renderSize.w, renderSize.h));
            if (!p.vbo)
            {
                p.vbo = vlk::VBO::create(
                    mesh.triangles.size() * 3, vlk::VBOType::Pos2_F32_UV_U16);
                p.vao.reset();
            }
            if (p.vbo)
            {
                p.vbo->copy(convert(mesh, vlk::VBOType::Pos2_F32_UV_U16));
            }

            if (!p.vao && p.vbo)
            {
                p.vao = vlk::VAO::create(ctx);
            }
        }
        
        void TimelineWidget::prepare()
        {
            TLRENDER_P();
            
            prepare_shaders();
            prepare_mesh();
            prepare_render_pass();
            prepare_pipeline_layout(); // Main shader layout
            prepare_pipeline();
        }

        void TimelineWidget::destroy()
        {
            TLRENDER_P();
            
            p.buffer.reset();

            if (p.pipeline_layout != VK_NULL_HANDLE)
            {
                vkDestroyPipelineLayout(device(), p.pipeline_layout, nullptr);
                p.pipeline_layout = VK_NULL_HANDLE;
            }

            if (pipeline() != VK_NULL_HANDLE)
            {
                vkDestroyPipeline(device(), pipeline(), nullptr);
                pipeline() = VK_NULL_HANDLE;
            }

            p.vbo.reset();
            p.vao.reset();
        }

        void TimelineWidget::hide()
        {
            TLRENDER_P();

            wait_device();
            
            // Destroy main render
            p.render.reset();

            // Destroy buffers
            p.buffer.reset();

            // Destroy shaders
            p.shader.reset();

            // Destroy meshes
            p.vbo.reset();
            p.vao.reset();

            VkWindow::hide();
        }

        void TimelineWidget::resize(int X, int Y, int W, int H)
        {
            TLRENDER_P();

            Fl_Vk_Window::resize(X, Y, W, H);

            _sizeHintEvent();
            _setGeometry();
            _clipEvent();
            
            if (p.draggingClip)
                toOtioFile(p.player, p.ui);

            repositionThumbnail();
        }

        void TimelineWidget::draw()
        {
            TLRENDER_P();
            
            const math::Size2i renderSize(pixel_w(), pixel_h());
            
            VkCommandBuffer cmd = getCurrentCommandBuffer();

            std::vector<int> markers;
            if (p.player && p.player->hasAnnotations())
            {
                const auto& times = p.player->getAnnotationTimes();
                if (p.annotationTimes != times)
                {
                    p.annotationTimes = times;
                    markers.reserve(times.size());
                    for (const auto& time : times)
                    {
                        markers.push_back(std::round(time.value()));
                    }
                    p.timelineWidget->setFrameMarkers(markers);
                }
            }
            else
            {
                p.timelineWidget->setFrameMarkers(markers);
                p.annotationTimes.clear();
            }

            if (_getDrawUpdate(p.timelineWindow) || !p.buffer)
            {
                try
                {
                    if (renderSize.isValid())
                    {
                        vlk::OffscreenBufferOptions offscreenBufferOptions;
                        offscreenBufferOptions.colorType =
                            image::PixelType::RGBA_U8;
                        offscreenBufferOptions.clear = true;
                        offscreenBufferOptions.clearColor =
                            p.style->getColorRole(ui::ColorRole::Window);
                        if (vlk::doCreate(
                                p.buffer, renderSize, offscreenBufferOptions))
                        { 
                            p.buffer = vlk::OffscreenBuffer::create(
                                ctx, renderSize, offscreenBufferOptions);
                        }
                    }
                    else
                    {
                        wait_device();
                        
                        p.buffer.reset();
                    }

                    if (p.render && p.buffer)
                    {
                        p.buffer->transitionToColorAttachment(cmd);
                        
                        timeline::RenderOptions renderOptions;
                        renderOptions.clear = true;
                        renderOptions.clearColor =
                            p.style->getColorRole(ui::ColorRole::Window);

                        // Clear color in new render pass.
                        p.render->begin(
                            cmd, p.buffer, m_currentFrameIndex, renderSize,
                            renderOptions);
                        const math::Matrix4x4f ortho = math::ortho(
                            0.F, static_cast<float>(renderSize.w),
                            static_cast<float>(renderSize.h), 0.F,
                            -1.F, 1.F);
                        p.render->setTransform(ortho);

                        const auto& ocioOptions = p.ui->uiView->getOCIOOptions();
                        p.render->setOCIOOptions(ocioOptions);
                        
                        const auto& lutOptions = p.ui->uiView->lutOptions();
                        p.render->setLUTOptions(lutOptions);
                        
                        p.render->setClipRectEnabled(true);
                        
                        ui::DrawEvent drawEvent(
                            p.style, p.iconLibrary, p.render, p.fontSystem);
                        p.render->beginLoadRenderPass();
                        _drawEvent(
                            p.timelineWindow, math::Box2i(renderSize),
                            drawEvent);
                        p.render->endRenderPass();
                        
                        p.render->setClipRectEnabled(false);
                        p.render->end();
                    }
                }
                catch (const std::exception& e)
                {
                    LOG_ERROR(e.what());
                }
            }

            const float alpha = p.ui->uiMain->get_alpha() / 255.F;
                
            if (p.ui->uiPrefs->uiPrefsBlitTimeline->value() == kNoBlit ||
                alpha < 1.0F)
            {
                p.buffer->transitionToShaderRead(cmd);
                        
                begin_render_pass(cmd);

                p.shader->bind(m_currentFrameIndex);
                const auto pm = math::ortho(
                    0.F, static_cast<float>(renderSize.w),
                    0.F, static_cast<float>(renderSize.h), -1.F, 1.F);
                p.shader->setUniform("transform.mvp", pm, vlk::kShaderVertex);
                p.shader->setFBO("textureSampler", p.buffer);

#ifdef __APPLE__
                set_window_transparency(alpha);
#endif

                // Bind the main composition pipeline (created/managed outside
                // this draw loop)
                vkCmdBindPipeline(
                    cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline());
                
                VkDescriptorSet descriptorSet = p.shader->getDescriptorSet();
                vkCmdBindDescriptorSets(
                    cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, p.pipeline_layout, 0,
                    1, &descriptorSet, 0, nullptr);
                
                const float opacity = 1.0;
                vkCmdPushConstants(
                    cmd, p.pipeline_layout,
                    p.shader->getPushStageFlags(), 0,
                    sizeof(float), &opacity);

                VkViewport viewport = {};
                viewport.x = 0.0f;
                viewport.y = 0.0f;
                viewport.width = static_cast<float>(pixel_w());
                viewport.height = static_cast<float>(pixel_h());
                viewport.minDepth = 0.0f;
                viewport.maxDepth = 1.0f;
                vkCmdSetViewport(cmd, 0, 1, &viewport);

                VkRect2D scissor = {};
                scissor.offset = {0, 0};
                scissor.extent.width = pixel_w();
                scissor.extent.height = pixel_h();
                vkCmdSetScissor(cmd, 0, 1, &scissor);

                if (p.vao && p.vbo)
                {
            
                    const VkColorComponentFlags allMask[] =
                        { VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                          VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT };
                    ctx.vkCmdSetColorWriteMaskEXT(cmd, 0, 1, allMask);
            
                    p.vao->bind(m_currentFrameIndex);
                    p.vao->draw(cmd, p.vbo);
                }
            }
            else
            {
                VkImage srcImage = p.buffer->getImage();
                VkImage dstImage = get_back_buffer_image();
                
                // srcImage barrier
                transitionImageLayout(cmd, srcImage,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

                // dstImage barrier (swapchain image)
                transitionImageLayout(cmd, dstImage,
                                      VK_IMAGE_LAYOUT_UNDEFINED,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);

                if (p.buffer->getFormat() == format())
                {
                    VkImageCopy region = {};
                    region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    region.srcSubresource.mipLevel = 0;
                    region.srcSubresource.baseArrayLayer = 0;
                    region.srcSubresource.layerCount = 1;
                    region.srcOffset = {0, 0, 0};
                    region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    region.dstSubresource.mipLevel = 0;
                    region.dstSubresource.baseArrayLayer = 0;
                    region.dstSubresource.layerCount = 1;
                    region.dstOffset = {0, 0, 0};
                    region.extent = {
                        static_cast<uint32_t>(renderSize.w),
                        static_cast<uint32_t>(renderSize.h),
                        1};

                    vkCmdCopyImage(
                        cmd,
                        srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        1, &region
                        );
                }
                else
                {

                    VkImageBlit region = {};
                    region.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    region.srcSubresource.mipLevel = 0;
                    region.srcSubresource.baseArrayLayer = 0;
                    region.srcSubresource.layerCount = 1;
                    region.srcOffsets[0] = {0, 0, 0};
                    region.srcOffsets[1] = {pixel_w(), pixel_h(), 1};
                    region.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
                    region.dstSubresource.mipLevel = 0;
                    region.dstSubresource.baseArrayLayer = 0;
                    region.dstSubresource.layerCount = 1;
                    region.dstOffsets[0] = {0, 0, 0};
                    region.dstOffsets[1] = {pixel_w(), pixel_h(), 1};
                    vkCmdBlitImage(
                        cmd,
                        srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                        dstImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                        1, &region,
                        VK_FILTER_NEAREST
                    );
                }
                
                // srcImage barrier
                transitionImageLayout(cmd, srcImage,
                                      VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                                      VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                p.buffer->setImageLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
                
                // dstImage barrier (swapchain image)
                transitionImageLayout(cmd, dstImage,
                                      VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                      VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
            }
        }

        void TimelineWidget::prepare_pipeline_layout()
        {
            TLRENDER_P();

            VkResult result;

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
                device(), &pPipelineLayoutCreateInfo, NULL, &p.pipeline_layout);
        }

        void TimelineWidget::prepare_pipeline()
        {
            TLRENDER_P();
            
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
            
            vlk::DepthStencilStateInfo ds;
            ds.depthTestEnable = VK_FALSE;
            ds.depthWriteEnable = VK_FALSE;
            ds.stencilTestEnable = VK_FALSE;
            
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

        int TimelineWidget::enterEvent()
        {
            TLRENDER_P();

            bool takeFocus = true;
            Fl_Widget* focusWidget = Fl::focus();
            TimelineClass* c = p.ui->uiTimeWindow;
            if (focusWidget == c->uiFrame || focusWidget == c->uiStartFrame ||
                focusWidget == c->uiEndFrame)
                takeFocus = false;
            // if (Fl::focus() == nullptr)
            if (takeFocus)
                Fl::focus(this);
            p.timelineWindow->cursorEnter(true);
            return 1;
        }

        int TimelineWidget::leaveEvent()
        {
            TLRENDER_P();
            p.timelineWindow->cursorEnter(false);
            return 1;
        }

        namespace
        {
            int fromFLTKModifiers()
            {
                int out = 0;
                if (Fl::event_key(FL_Shift_L) || Fl::event_key(FL_Shift_R))
                {
                    out |= static_cast<int>(ui::KeyModifier::Shift);
                }
                if (Fl::event_key(FL_Control_L) || Fl::event_key(FL_Control_R))
                {
                    out |= static_cast<int>(ui::KeyModifier::Control);
                }
                if (Fl::event_key(FL_Alt_L) || Fl::event_key(FL_Alt_R))
                {
                    out |= static_cast<int>(ui::KeyModifier::Alt);
                }
                return out;
            }
        } // namespace

        int TimelineWidget::mousePressEvent(int button, bool on, int modifiers)
        {
            TLRENDER_P();
            bool send = App::ui->uiPrefs->SendTimeline->value();
            if (send)
            {
                Message message;
                message["command"] = "Timeline Mouse Press";
                message["button"] = button;
                message["on"] = on;
                message["modifiers"] = modifiers;
                tcp->pushMessage(message);
            }
            if (p.draggingClip)
            {
                makePathsAbsolute(p.player, p.ui);
            }
            p.timelineWindow->mouseButton(button, on, modifiers);
            return 1;
        }

        int TimelineWidget::mousePressEvent()
        {
            TLRENDER_P();
            Fl::focus(this);

            bool isNDI = file::isTemporaryNDI(p.player->path());
            if (isNDI)
                return 0;

            int button = -1;
            int modifiers = fromFLTKModifiers();
            if (Fl::event_button1())
            {
                button = 0;
                if (modifiers == 0)
                {
                    int ok = _seek();
                    if (!p.draggingClip && ok)
                        return 1;
                }
            }
            else if (Fl::event_button2())
            {
                button = 0;
                modifiers = static_cast<int>(ui::KeyModifier::Control);
            }
            else
            {
                return 0;
            }

            mousePressEvent(button, true, modifiers);
            return 1;
        }

        int TimelineWidget::mouseDragEvent(const int X, const int Y)
        {
            TLRENDER_P();
            bool isNDI = file::isTemporaryNDI(p.player->path());
            if (isNDI)
                return 0;

            int modifiers = fromFLTKModifiers();
            if (Fl::event_button1())
            {
                if (modifiers == 0)
                {
                    int ok = _seek();
                    if (!p.draggingClip && ok)
                        return 1;
                }
            }
            else if (Fl::event_button2())
            {
                // left empty on purpose
            }
            else
            {
                return 0;
            }
            mouseMoveEvent(X, Y);
            return 1;
        }

        int TimelineWidget::mouseReleaseEvent(
            const int X, const int Y, int button, bool on, int modifiers)
        {
            TLRENDER_P();

            bool isNDI = file::isTemporaryNDI(p.player->path());
            if (isNDI)
                return 0;

            if (button == 1)
            {
                int ok = _seek();
                if (!p.draggingClip && ok)
                    return 1;
            }
            mouseMoveEvent(X, Y);
            mousePressEvent(button, on, modifiers);
            if (p.draggingClip)
            {
                toOtioFile(p.player, p.ui);
                p.ui->uiView->redrawWindows();
                panel::redrawThumbnails();
                p.draggingClip = false;
            }
            bool send = App::ui->uiPrefs->SendTimeline->value();
            if (send)
            {
                Message message;
                message["command"] = "Timeline Mouse Release";
                message["X"] = static_cast<float>(X) / pixel_w();
                message["Y"] = static_cast<float>(Y) / pixel_h();
                message["button"] = button;
                message["on"] = on;
                message["modifiers"] = modifiers;
                tcp->pushMessage(message);
            }
            return 1;
        }

        int TimelineWidget::mouseReleaseEvent()
        {
            int button = 0;
            if (Fl::event_button1())
                button = 1;
            mouseReleaseEvent(
                Fl::event_x(), Fl::event_y(), button, false,
                fromFLTKModifiers());
            return 1;
        }

        int TimelineWidget::mouseMoveEvent()
        {
            TLRENDER_P();
            mouseMoveEvent(Fl::event_x(), Fl::event_y());
            return 1;
        }

        void TimelineWidget::mouseMoveEvent(const int X, const int Y)
        {
            TLRENDER_P();
            bool send = App::ui->uiPrefs->SendTimeline->value();
            if (send)
            {
                Message message;
                message["command"] = "Timeline Mouse Move";
                message["X"] = static_cast<float>(X) / pixel_w();
                message["Y"] = static_cast<float>(Y) / pixel_h();
                tcp->pushMessage(message);
            }
            const auto now = std::chrono::steady_clock::now();
            const auto diff =
                std::chrono::duration<float>(now - p.mouseWheelTimer);
            const float delta = Fl::event_dy() / 8.F / 15.F;
            p.mouseWheelTimer = now;
            p.timelineWindow->cursorPos(math::Vector2i(_toUI(X), _toUI(Y)));
        }

        void
        TimelineWidget::scrollEvent(const float X, const float Y, int modifiers)
        {
            TLRENDER_P();
            math::Vector2f pos(X, Y);
            p.timelineWindow->scroll(pos, modifiers);

            Message message;
            message["command"] = "Timeline Widget Scroll";
            message["X"] = X;
            message["Y"] = Y;
            message["modifiers"] = modifiers;
            bool send = App::ui->uiPrefs->SendTimeline->value();
            if (send)
                tcp->pushMessage(message);
        }

        int TimelineWidget::wheelEvent()
        {
            TLRENDER_P();
            const auto now = std::chrono::steady_clock::now();
            const auto diff =
                std::chrono::duration<float>(now - p.mouseWheelTimer);
            const float delta = Fl::event_dy() / 8.F / 15.F;
            p.mouseWheelTimer = now;
            scrollEvent(
                Fl::event_dx() / 8.F / 15.F, -delta, fromFLTKModifiers());
            return 1;
        }

        namespace
        {
            ui::Key fromFLTKKey(unsigned key)
            {
                ui::Key out = ui::Key::Unknown;

#if defined(FLTK_USE_WAYLAND)
                if (key >= 'A' && key <= 'Z')
                {
                    key = tolower(key);
                }
#endif
                switch (key)
                {
                case ' ':
                    out = ui::Key::Space;
                    break;
                case '\'':
                    out = ui::Key::Apostrophe;
                    break;
                case ',':
                    out = ui::Key::Comma;
                    break;
                case '-':
                    out = ui::Key::Minus;
                    break;
                case '.':
                    out = ui::Key::Period;
                    break;
                case '/':
                    out = ui::Key::Slash;
                    break;
                case '0':
                    out = ui::Key::_0;
                    break;
                case '1':
                    out = ui::Key::_1;
                    break;
                case '2':
                    out = ui::Key::_2;
                    break;
                case '3':
                    out = ui::Key::_3;
                    break;
                case '4':
                    out = ui::Key::_4;
                    break;
                case '5':
                    out = ui::Key::_5;
                    break;
                case '6':
                    out = ui::Key::_6;
                    break;
                case '7':
                    out = ui::Key::_7;
                    break;
                case '8':
                    out = ui::Key::_8;
                    break;
                case '9':
                    out = ui::Key::_9;
                    break;
                case ';':
                    out = ui::Key::Semicolon;
                    break;
                case '=':
                    out = ui::Key::Equal;
                    break;
                case 'a':
                    out = ui::Key::A;
                    break;
                case 'b':
                    out = ui::Key::B;
                    break;
                case 'c':
                    out = ui::Key::C;
                    break;
                case 'd':
                    out = ui::Key::D;
                    break;
                case 'e':
                    out = ui::Key::E;
                    break;
                case 'f':
                    out = ui::Key::F;
                    break;
                case 'g':
                    out = ui::Key::G;
                    break;
                case 'h':
                    out = ui::Key::H;
                    break;
                case 'i':
                    out = ui::Key::I;
                    break;
                case 'j':
                    out = ui::Key::J;
                    break;
                case 'k':
                    out = ui::Key::K;
                    break;
                case 'l':
                    out = ui::Key::L;
                    break;
                case 'm':
                    out = ui::Key::M;
                    break;
                case 'n':
                    out = ui::Key::N;
                    break;
                case 'o':
                    out = ui::Key::O;
                    break;
                case 'p':
                    out = ui::Key::P;
                    break;
                case 'q':
                    out = ui::Key::Q;
                    break;
                case 'r':
                    out = ui::Key::R;
                    break;
                case 's':
                    out = ui::Key::S;
                    break;
                case 't':
                    out = ui::Key::T;
                    break;
                case 'u':
                    out = ui::Key::U;
                    break;
                case 'v':
                    out = ui::Key::V;
                    break;
                case 'w':
                    out = ui::Key::W;
                    break;
                case 'x':
                    out = ui::Key::X;
                    break;
                case 'y':
                    out = ui::Key::Y;
                    break;
                case 'z':
                    out = ui::Key::Z;
                    break;
                case '[':
                    out = ui::Key::LeftBracket;
                    break;
                case '\\':
                    out = ui::Key::Backslash;
                    break;
                case ']':
                    out = ui::Key::RightBracket;
                    break;
                case 0xfe51: // @todo: is there a Fl_ shortcut for this?
                    out = ui::Key::GraveAccent;
                    break;
                case FL_Escape:
                    out = ui::Key::Escape;
                    break;
                case FL_Enter:
                    out = ui::Key::Enter;
                    break;
                case FL_Tab:
                    out = ui::Key::Tab;
                    break;
                case FL_BackSpace:
                    out = ui::Key::Backspace;
                    break;
                case FL_Insert:
                    out = ui::Key::Insert;
                    break;
                case FL_Delete:
                    out = ui::Key::Delete;
                    break;
                case FL_Right:
                    out = ui::Key::Right;
                    break;
                case FL_Left:
                    out = ui::Key::Left;
                    break;
                case FL_Down:
                    out = ui::Key::Down;
                    break;
                case FL_Up:
                    out = ui::Key::Up;
                    break;
                case FL_Page_Up:
                    out = ui::Key::PageUp;
                    break;
                case FL_Page_Down:
                    out = ui::Key::PageDown;
                    break;
                case FL_Home:
                    out = ui::Key::Home;
                    break;
                case FL_End:
                    out = ui::Key::End;
                    break;
                case FL_Caps_Lock:
                    out = ui::Key::CapsLock;
                    break;
                case FL_Scroll_Lock:
                    out = ui::Key::ScrollLock;
                    break;
                case FL_Num_Lock:
                    out = ui::Key::NumLock;
                    break;
                case FL_Print:
                    out = ui::Key::PrintScreen;
                    break;
                case FL_Pause:
                    out = ui::Key::Pause;
                    break;
                case FL_F + 1:
                    out = ui::Key::F1;
                    break;
                case FL_F + 2:
                    out = ui::Key::F2;
                    break;
                case FL_F + 3:
                    out = ui::Key::F3;
                    break;
                case FL_F + 4:
                    out = ui::Key::F4;
                    break;
                case FL_F + 5:
                    out = ui::Key::F5;
                    break;
                case FL_F + 6:
                    out = ui::Key::F6;
                    break;
                case FL_F + 7:
                    out = ui::Key::F7;
                    break;
                case FL_F + 8:
                    out = ui::Key::F8;
                    break;
                case FL_F + 9:
                    out = ui::Key::F9;
                    break;
                case FL_F + 10:
                    out = ui::Key::F10;
                    break;
                case FL_F + 11:
                    out = ui::Key::F11;
                    break;
                case FL_F + 12:
                    out = ui::Key::F12;
                    break;
                case FL_Shift_L:
                    out = ui::Key::LeftShift;
                    break;
                case FL_Control_L:
                    out = ui::Key::LeftControl;
                    break;
                case FL_Alt_L:
                    out = ui::Key::LeftAlt;
                    break;
                case FL_Meta_L:
                    out = ui::Key::LeftSuper;
                    break;
                case FL_Meta_R:
                    out = ui::Key::RightSuper;
                    break;
                }
                return out;
            }
        } // namespace

        void TimelineWidget::frameView()
        {
            TLRENDER_P();
            unsigned key = _changeKey(kFitScreen.hotkey());
            if (p.player)
            {
                auto innerPlayer = p.player->player();
                p.timeRange = innerPlayer->getTimeRange();
            }
            p.timelineWindow->key(fromFLTKKey(key), true, 0);
            bool send = App::ui->uiPrefs->SendTimeline->value();
            if (send)
            {
                Message message;
                message["command"] = "Timeline Fit";
                tcp->pushMessage(message);
            }
        }

        int TimelineWidget::keyPressEvent(unsigned key, const int modifiers)
        {
            TLRENDER_P();
            // First, check if it is one of the menu shortcuts
            int ret = p.ui->uiMenuBar->handle(FL_SHORTCUT);
            if (ret)
                return ret;
            if (kToggleEditMode.match(key))
            {
                p.ui->uiEdit->do_callback();
                return 1;
            }
            bool send = App::ui->uiPrefs->SendTimeline->value();
            if (send)
            {
                Message message;
                message["command"] = "Timeline Key Press";
                message["value"] = key;
                message["modifiers"] = modifiers;
                tcp->pushMessage(message);
            }

            key = _changeKey(key);
            p.timelineWindow->key(fromFLTKKey(key), true, modifiers);
            return 1;
        }

        int TimelineWidget::keyPressEvent()
        {
            TLRENDER_P();
            unsigned key = Fl::event_key();
            keyPressEvent(key, fromFLTKModifiers());
            return 1;
        }

        int TimelineWidget::keyReleaseEvent(unsigned key, const int modifiers)
        {
            TLRENDER_P();
            bool send = App::ui->uiPrefs->SendTimeline->value();
            if (send)
            {
                Message message;
                message["command"] = "Timeline Key Release";
                message["value"] = key;
                message["modifiers"] = modifiers;
                tcp->pushMessage(message);
            }
            key = _changeKey(key);
            p.timelineWindow->key(fromFLTKKey(key), false, modifiers);
            return 1;
        }

        int TimelineWidget::keyReleaseEvent()
        {
            TLRENDER_P();
            keyReleaseEvent(Fl::event_key(), fromFLTKModifiers());
            return 1;
        }

        void TimelineWidget::timerEvent_cb(void* d)
        {
            TimelineWidget* o = static_cast<TimelineWidget*>(d);
            o->timerEvent();
        }

        void TimelineWidget::_thumbnailEvent()
        {
            TLRENDER_P();

            if (p.thumbnailWindow && !p.thumbnailWindow->visible())
                return;

            if (p.thumbnail.request.future.valid() &&
                p.thumbnail.request.future.wait_for(std::chrono::seconds(0)) ==
                    std::future_status::ready)
            {
                if (auto image = p.thumbnail.request.future.get())
                {
                    if (image::PixelType::RGBA_U8 == image->getPixelType())
                    {
                        const int w = image->getWidth();
                        const int h = image->getHeight();
                        const int depth = 4;

                        uint8_t* pixelData = new uint8_t[w * h * depth];

                        auto rgbImage =
                            new Fl_RGB_Image(pixelData, w, h, depth);
                        rgbImage->alloc_array = true;

                        uint8_t* d = pixelData;
                        const uint8_t* s = image->getData();
                        std::memcpy(d, s, w * h * 4);
                        p.box->bind_image(rgbImage);
                        p.box->redraw();
                        repositionThumbnail();
                    }
                }
            }
        }

        void TimelineWidget::timerEvent()
        {
            TLRENDER_P();

            if (shown())
            {
                _tickEvent();

                _thumbnailEvent();

                if (_getSizeUpdate(p.timelineWindow))
                {
                    _sizeHintEvent();
                    _setGeometry();
                    _clipEvent();
                }

                if (_getDrawUpdate(p.timelineWindow))
                {
                    redraw();
                }
            }
            
            Fl::repeat_timeout(
                kTimeout, (Fl_Timeout_Handler)timerEvent_cb, this);
        }

        int TimelineWidget::handle(int event)
        {
            TLRENDER_P();
            switch (event)
            {
            case FL_FOCUS:
            case FL_UNFOCUS:
                return 1;
            case FL_ENTER:
                cursor(FL_CURSOR_DEFAULT);
                requestThumbnail(true);
                return enterEvent();
            case FL_LEAVE:
                if (p.ui->uiPrefs->uiPrefsTimelineThumbnails->value())
                {
                    _cancelThumbnailRequests();
                    hideThumbnail();
                }
                return leaveEvent();
            case FL_PUSH:
                if (p.ui->uiPrefs->uiPrefsTimelineThumbnails->value())
                    requestThumbnail(true);
                return mousePressEvent();
            case FL_DRAG:
                return mouseDragEvent(Fl::event_x(), Fl::event_y());
            case FL_RELEASE:
                panel::redrawThumbnails();
                return mouseReleaseEvent();
            case FL_MOVE:
                requestThumbnail(true);
                return mouseMoveEvent();
            case FL_MOUSEWHEEL:
                if (Fl::belowmouse() != this)
                    return 0;
                return wheelEvent();
            case FL_KEYDOWN:
            {
                return keyPressEvent();
            }
            case FL_KEYUP:
                return keyReleaseEvent();
            case FL_HIDE:
            {
                if (p.ui->uiPrefs->uiPrefsTimelineThumbnails->value())
                {
                    _cancelThumbnailRequests();
                    hideThumbnail();
                }
                return Fl_Vk_Window::handle(event);
            }
            }
            int out = Fl_Vk_Window::handle(event);
            return out;
        }

        const float TimelineWidget::pixelRatio() const
        {
            TLRENDER_P();
            const float devicePixelRatio = p.ui->uiView->pixels_per_unit();
            return devicePixelRatio;
        }

        int TimelineWidget::_toUI(int value) const
        {
            const float devicePixelRatio = pixelRatio();
            return value * devicePixelRatio;
        }

        math::Vector2i TimelineWidget::_toUI(const math::Vector2i& value) const
        {
            const float devicePixelRatio = pixelRatio();
            return value * devicePixelRatio;
        }

        int TimelineWidget::_fromUI(int value) const
        {
            const float devicePixelRatio = pixelRatio();
            return devicePixelRatio > 0.F ? (value / devicePixelRatio) : 0.F;
        }

        math::Vector2i
        TimelineWidget::_fromUI(const math::Vector2i& value) const
        {
            const float devicePixelRatio = pixelRatio();
            return devicePixelRatio > 0.F ? (value / devicePixelRatio)
                                          : math::Vector2i();
        }

        //! Routine to turn mrv2's hotkeys into Darby's shortcuts
        unsigned TimelineWidget::_changeKey(unsigned key)
        {
            if (key == kFitScreen.hotkey())
                key = '0'; // Darby uses 0 to frame view
            else if (key == kFitAll.hotkey())
                key = '0'; // Darby uses 0 to frame view
            return key;
        }

        void TimelineWidget::_styleUpdate() {}

        otime::RationalTime TimelineWidget::_posToTime(int value) noexcept
        {
            TLRENDER_P();

            otime::RationalTime out = time::invalidTime;
            if (p.player && p.timelineWidget)
            {
                _setGeometry(); // needed, as Linux could have issues when
                // dragging the window to the borders.
                const math::Box2i& geometry =
                    p.timelineWidget->getTimelineItemGeometry();
                const double normalized = (value - geometry.min.x) /
                                          static_cast<double>(geometry.w());
                out = (p.timeRange.start_time() +
                       otime::RationalTime(
                           p.timeRange.duration().value() * normalized,
                           p.timeRange.duration().rate()))
                          .round();
                out = math::clamp(
                    out, p.timeRange.start_time(),
                    p.timeRange.end_time_inclusive());
            }
            return out;
        }


        void TimelineWidget::setUnits(TimeUnits value)
        {
            TLRENDER_P();
            p.units = value;
            auto timeUnitsModel = _p->ui->app->timeUnitsModel();
            timeUnitsModel->setTimeUnits(
                static_cast<tl::timeline::TimeUnits>(value));
            TimelineClass* c = _p->ui->uiTimeWindow;
            c->uiStartFrame->setUnits(value);
            c->uiEndFrame->setUnits(value);
            c->uiFrame->setUnits(value);
            redraw();
        }

        void TimelineWidget::moveCallback(
            const std::vector<tl::timeline::MoveData>& moves)
        {
            TLRENDER_P();
            edit_store_undo(p.player, p.ui);
            edit_clear_redo(p.ui);
            edit_move_clip_annotations(moves, p.ui);
        }

        void TimelineWidget::_tickEvent()
        {
            TLRENDER_P();
            ui::TickEvent tickEvent(p.style, p.iconLibrary, p.fontSystem);
            _tickEvent(_p->timelineWindow, true, true, tickEvent);
        }

        void TimelineWidget::_tickEvent(
            const std::shared_ptr<ui::IWidget>& widget, bool visible,
            bool enabled, const ui::TickEvent& event)
        {
            TLRENDER_P();
            const bool parentsVisible = visible && widget->isVisible(false);
            const bool parentsEnabled = enabled && widget->isEnabled(false);
            for (const auto& child : widget->getChildren())
            {
                _tickEvent(child, parentsVisible, parentsEnabled, event);
            }
            widget->tickEvent(visible, enabled, event);
        }

        bool TimelineWidget::_getSizeUpdate(
            const std::shared_ptr<ui::IWidget>& widget) const
        {
            bool out = widget->getUpdates() & ui::Update::Size;
            if (out)
            {
                // std::cout << "Size update: " << widget->getObjectName() <<
                // std::endl;
            }
            else
            {
                for (const auto& child : widget->getChildren())
                {
                    out |= _getSizeUpdate(child);
                }
            }
            return out;
        }

        void TimelineWidget::_sizeHintEvent()
        {
            TLRENDER_P();
            const float devicePixelRatio = pixelRatio();
            ui::SizeHintEvent sizeHintEvent(
                p.style, p.iconLibrary, p.fontSystem, devicePixelRatio);
            _sizeHintEvent(p.timelineWindow, sizeHintEvent);
        }

        void TimelineWidget::_sizeHintEvent(
            const std::shared_ptr<ui::IWidget>& widget,
            const ui::SizeHintEvent& event)
        {
            for (const auto& child : widget->getChildren())
            {
                _sizeHintEvent(child, event);
            }
            widget->sizeHintEvent(event);
        }

        void TimelineWidget::_setGeometry()
        {
            TLRENDER_P();
            const math::Box2i geometry(0, 0, _toUI(w()), _toUI(h()));
            p.timelineWindow->setGeometry(geometry);
        }

        void TimelineWidget::_clipEvent()
        {
            TLRENDER_P();
            const math::Box2i geometry(0, 0, _toUI(w()), _toUI(h()));
            _clipEvent(p.timelineWindow, geometry, false);
        }

        void TimelineWidget::_clipEvent(
            const std::shared_ptr<ui::IWidget>& widget,
            const math::Box2i& clipRect, bool clipped)
        {
            const math::Box2i& g = widget->getGeometry();
            clipped |= !g.intersects(clipRect);
            clipped |= !widget->isVisible(false);
            const math::Box2i clipRect2 = g.intersect(clipRect);
            widget->clipEvent(clipRect2, clipped);
            const math::Box2i childrenClipRect =
                widget->getChildrenClipRect().intersect(clipRect2);
            for (const auto& child : widget->getChildren())
            {
                const math::Box2i& childGeometry = child->getGeometry();
                _clipEvent(
                    child, childGeometry.intersect(childrenClipRect), clipped);
            }
        }

        bool TimelineWidget::_getDrawUpdate(
            const std::shared_ptr<ui::IWidget>& widget) const
        {
            bool out = false;
            if (!widget->isClipped())
            {
                out = widget->getUpdates() & ui::Update::Draw;
                if (out)
                {
                    // std::cout << "Draw update: " << widget->getObjectName()
                    // << std::endl;
                }
                else
                {
                    for (const auto& child : widget->getChildren())
                    {
                        out |= _getDrawUpdate(child);
                    }
                }
            }
            return out;
        }

        void TimelineWidget::_drawEvent(
            const std::shared_ptr<ui::IWidget>& widget,
            const math::Box2i& drawRect, const ui::DrawEvent& event)
        {
            const math::Box2i& g = widget->getGeometry();
            if (!widget->isClipped() && g.w() > 0 && g.h() > 0)
            {
                event.render->setClipRect(drawRect);
                widget->drawEvent(drawRect, event);
                const math::Box2i childrenClipRect =
                    widget->getChildrenClipRect().intersect(drawRect);
                event.render->setClipRect(childrenClipRect);
                for (const auto& child : widget->getChildren())
                {
                    const math::Box2i& childGeometry = child->getGeometry();
                    if (childGeometry.intersects(childrenClipRect))
                    {
                        _drawEvent(
                            child, childGeometry.intersect(childrenClipRect),
                            event);
                    }
                }
                event.render->setClipRect(drawRect);
                widget->drawOverlayEvent(drawRect, event);
            }
        }

    } // namespace vulkan

} // namespace mrv
