// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlQtWidget/TimelineViewport.h>

#include <tlUI/DrawUtil.h>

#include <tlTimelineGL/Render.h>

#include <tlGL/Init.h>
#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Shader.h>

#include <tlCore/Mesh.h>

#include <QGuiApplication>
#include <QMouseEvent>
#include <QSurfaceFormat>
#include <QWindow>

namespace tl
{
    namespace qtwidget
    {
        struct TimelineViewport::Private
        {
            std::weak_ptr<system::Context> context;
            timeline::OCIOOptions ocioOptions;
            timeline::LUTOptions lutOptions;
            std::vector<timeline::ImageOptions> imageOptions;
            std::vector<timeline::DisplayOptions> displayOptions;
            timeline::CompareOptions compareOptions;
            timeline::BackgroundOptions backgroundOptions;
            image::PixelType colorBuffer = image::PixelType::RGBA_U8;
            QSharedPointer<qt::TimelinePlayer> player;
            std::vector<timeline::VideoData> videoData;
            math::Vector2i viewPos;
            double viewZoom = 1.0;
            bool frameView = true;
            struct FpsData
            {
                double fps = 0.0;
                std::chrono::steady_clock::time_point timer;
                size_t frameCount = 0;
            };
            FpsData fpsData;
            struct DroppedFramesData
            {
                size_t dropped = 0;
                bool init = true;
                double frame = 0.0;
            };
            DroppedFramesData droppedFramesData;

            enum class MouseMode { None, View, Wipe };
            MouseMode mouseMode = MouseMode::kNone;
            math::Vector2i mousePos;
            math::Vector2i mousePress;
            math::Vector2i viewPosMousePress;

            bool doRender = false;
            std::shared_ptr<timeline::IRender> render;
            std::shared_ptr<tl::gl::Shader> shader;
            std::shared_ptr<tl::gl::OffscreenBuffer> buffer;
            std::shared_ptr<gl::VBO> vbo;
            std::shared_ptr<gl::VAO> vao;
        };

        TimelineViewport::TimelineViewport(
            const std::shared_ptr<system::Context>& context, QWidget* parent) :
            QOpenGLWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            setMouseTracking(true);
            setFocusPolicy(Qt::StrongFocus);

            p.context = context;
        }

        TimelineViewport::~TimelineViewport()
        {
            makeCurrent();
        }

        image::PixelType TimelineViewport::colorBuffer() const
        {
            return _p->colorBuffer;
        }

        const math::Vector2i& TimelineViewport::viewPos() const
        {
            return _p->viewPos;
        }

        double TimelineViewport::viewZoom() const
        {
            return _p->viewZoom;
        }

        bool TimelineViewport::hasFrameView() const
        {
            return _p->frameView;
        }

        double TimelineViewport::getFPS() const
        {
            return _p->fpsData.fps;
        }

        size_t TimelineViewport::getDroppedFrames() const
        {
            return _p->droppedFramesData.dropped;
        }

        void
        TimelineViewport::setOCIOOptions(const timeline::OCIOOptions& value)
        {
            TLRENDER_P();
            if (value == p.ocioOptions)
                return;
            p.ocioOptions = value;
            p.doRender = true;
            update();
        }

        void TimelineViewport::setLUTOptions(const timeline::LUTOptions& value)
        {
            TLRENDER_P();
            if (value == p.lutOptions)
                return;
            p.lutOptions = value;
            p.doRender = true;
            update();
        }

        void TimelineViewport::setImageOptions(
            const std::vector<timeline::ImageOptions>& value)
        {
            TLRENDER_P();
            if (value == p.imageOptions)
                return;
            p.imageOptions = value;
            p.doRender = true;
            update();
        }

        void TimelineViewport::setDisplayOptions(
            const std::vector<timeline::DisplayOptions>& value)
        {
            TLRENDER_P();
            if (value == p.displayOptions)
                return;
            p.displayOptions = value;
            p.doRender = true;
            update();
        }

        void TimelineViewport::setCompareOptions(
            const timeline::CompareOptions& value)
        {
            TLRENDER_P();
            if (value == p.compareOptions)
                return;
            p.compareOptions = value;
            p.doRender = true;
            update();
        }

        void TimelineViewport::setBackgroundOptions(
            const timeline::BackgroundOptions& value)
        {
            TLRENDER_P();
            if (value == p.backgroundOptions)
                return;
            p.backgroundOptions = value;
            p.doRender = true;
            update();
        }

        void TimelineViewport::setColorBuffer(image::PixelType value)
        {
            TLRENDER_P();
            if (value == p.colorBuffer)
                return;
            p.colorBuffer = value;
            p.doRender = true;
            update();
        }

        void TimelineViewport::setPlayer(
            const QSharedPointer<qt::TimelinePlayer>& value)
        {
            TLRENDER_P();

            p.fpsData.fps = 0.0;
            p.fpsData.timer = std::chrono::steady_clock::now();
            p.fpsData.frameCount = 0;
            Q_EMIT fpsChanged(p.fpsData.fps);
            p.droppedFramesData.init = true;
            Q_EMIT droppedFramesChanged(p.droppedFramesData.dropped);

            if (value)
            {
                disconnect(
                    value.get(),
                    SIGNAL(playbackChanged(tl::timeline::Playback)), this,
                    SLOT(_playbackUpdate(tl::timeline::Playback)));
                disconnect(
                    value.get(),
                    SIGNAL(currentVideoChanged(
                        const std::vector<tl::timeline::VideoData>&)),
                    this,
                    SLOT(_videoDataUpdate(
                        const std::vector<tl::timeline::VideoData>&)));
            }

            p.player = value;

            p.videoData.clear();
            if (p.player)
            {
                p.videoData = p.player->currentVideo();
            }
            p.doRender = true;
            update();

            if (p.player)
            {
                connect(
                    p.player.get(),
                    SIGNAL(playbackChanged(tl::timeline::Playback)),
                    SLOT(_playbackUpdate(tl::timeline::Playback)));
                connect(
                    p.player.get(),
                    SIGNAL(currentVideoChanged(
                        const std::vector<tl::timeline::VideoData>&)),
                    SLOT(_videoDataUpdate(
                        const std::vector<tl::timeline::VideoData>&)));
            }
        }

        void TimelineViewport::setViewPosAndZoom(
            const math::Vector2i& pos, double zoom)
        {
            TLRENDER_P();
            if (pos == p.viewPos && zoom == p.viewZoom)
                return;
            p.viewPos = pos;
            p.viewZoom = zoom;
            p.doRender = true;
            update();
            Q_EMIT viewPosAndZoomChanged(p.viewPos, p.viewZoom);
            setFrameView(false);
        }

        void
        TimelineViewport::setViewZoom(double zoom, const math::Vector2i& focus)
        {
            TLRENDER_P();
            math::Vector2i pos;
            pos.x = focus.x + (p.viewPos.x - focus.x) * (zoom / p.viewZoom);
            pos.y = focus.y + (p.viewPos.y - focus.y) * (zoom / p.viewZoom);
            setViewPosAndZoom(pos, zoom);
        }

        void TimelineViewport::setFrameView(bool value)
        {
            TLRENDER_P();
            if (value == p.frameView)
                return;
            p.frameView = value;
            p.doRender = true;
            update();
            Q_EMIT frameViewChanged(p.frameView);
        }

        void TimelineViewport::viewZoom1To1()
        {
            TLRENDER_P();
            setViewZoom(1.F, _viewportCenter());
        }

        void TimelineViewport::viewZoomIn()
        {
            TLRENDER_P();
            setViewZoom(p.viewZoom * 2.F, _viewportCenter());
        }

        void TimelineViewport::viewZoomOut()
        {
            TLRENDER_P();
            setViewZoom(p.viewZoom / 2.F, _viewportCenter());
        }

        void TimelineViewport::_playbackUpdate(timeline::Playback value)
        {
            TLRENDER_P();
            switch (value)
            {
            case timeline::Playback::Forward:
            case timeline::Playback::Reverse:
                p.fpsData.timer = std::chrono::steady_clock::now();
                p.fpsData.frameCount = 0;
                p.droppedFramesData.init = true;
                break;
            default:
                break;
            }
        }

        void TimelineViewport::_videoDataUpdate(
            const std::vector<timeline::VideoData>& value)
        {
            TLRENDER_P();

            p.videoData = value;

            p.fpsData.frameCount = p.fpsData.frameCount + 1;
            const auto now = std::chrono::steady_clock::now();
            const std::chrono::duration<double> diff = now - p.fpsData.timer;
            if (diff.count() > 1.0)
            {
                const double fps = p.fpsData.frameCount / diff.count();
                // std::cout << "FPS: " << fps << std::endl;
                if (fps != p.fpsData.fps)
                {
                    p.fpsData.fps = fps;
                    Q_EMIT fpsChanged(fps);
                }
                p.fpsData.timer = now;
                p.fpsData.frameCount = 0;
            }

            p.doRender = true;
            update();
        }

        void TimelineViewport::initializeGL()
        {
            TLRENDER_P();

            initializeOpenGLFunctions();
            gl::initGLAD();

            try
            {
                if (auto context = p.context.lock())
                {
                    p.render = timeline_gl::Render::create(context);
                }

                const std::string vertexSource =
                    "#version 410\n"
                    "\n"
                    "in vec3 vPos;\n"
                    "in vec2 vTexture;\n"
                    "out vec2 fTexture;\n"
                    "\n"
                    "uniform struct Transform\n"
                    "{\n"
                    "    mat4 mvp;\n"
                    "} transform;\n"
                    "\n"
                    "void main()\n"
                    "{\n"
                    "    gl_Position = transform.mvp * vec4(vPos, 1.0);\n"
                    "    fTexture = vTexture;\n"
                    "}\n";
                const std::string fragmentSource =
                    "#version 410\n"
                    "\n"
                    "in vec2 fTexture;\n"
                    "out vec4 fColor;\n"
                    "\n"
                    "uniform sampler2D textureSampler;\n"
                    "\n"
                    "void main()\n"
                    "{\n"
                    "    fColor = texture(textureSampler, fTexture);\n"
                    "}\n";
                p.shader = gl::Shader::create(vertexSource, fragmentSource);
            }
            catch (const std::exception& e)
            {
                if (auto context = p.context.lock())
                {
                    context->log(
                        "tl::qtwidget::TimelineViewport", e.what(),
                        log::Type::Error);
                }
            }
        }

        void TimelineViewport::resizeGL(int w, int h)
        {
            TLRENDER_P();
            p.doRender = true;
            p.vao.reset();
            p.vbo.reset();
        }

        void TimelineViewport::paintGL()
        {
            TLRENDER_P();

            if (p.frameView)
            {
                _frameView();
            }

            const auto viewportSize = _viewportSize();
            if (p.doRender)
            {
                p.doRender = false;
                try
                {
                    if (viewportSize.isValid())
                    {
                        gl::OffscreenBufferOptions offscreenBufferOptions;
                        offscreenBufferOptions.colorType = p.colorBuffer;
                        if (!p.displayOptions.empty())
                        {
                            offscreenBufferOptions.colorFilters =
                                p.displayOptions[0].imageFilters;
                        }
                        offscreenBufferOptions.depth = gl::OffscreenDepth::_24;
                        offscreenBufferOptions.stencil =
                            gl::OffscreenStencil::_8;
                        if (gl::doCreate(
                                p.buffer, viewportSize, offscreenBufferOptions))
                        {
                            p.buffer = gl::OffscreenBuffer::create(
                                viewportSize, offscreenBufferOptions);
                        }
                    }
                    else
                    {
                        p.buffer.reset();
                    }

                    if (p.buffer)
                    {
                        gl::OffscreenBufferBinding binding(p.buffer);
                        timeline::RenderOptions renderOptions;
                        renderOptions.colorBuffer = p.colorBuffer;
                        p.render->begin(viewportSize, renderOptions);
                        p.render->setOCIOOptions(p.ocioOptions);
                        p.render->setLUTOptions(p.lutOptions);
                        if (!p.videoData.empty())
                        {
                            math::Matrix4x4f vm;
                            vm = vm * math::translate(math::Vector3f(
                                          p.viewPos.x, p.viewPos.y, 0.F));
                            vm = vm * math::scale(math::Vector3f(
                                          p.viewZoom, p.viewZoom, 1.F));
                            const auto pm = math::ortho(
                                0.F, static_cast<float>(viewportSize.w), 0.F,
                                static_cast<float>(viewportSize.h), -1.F, 1.F);
                            p.render->setTransform(pm * vm);
                            p.render->drawVideo(
                                p.videoData,
                                timeline::getBoxes(
                                    p.compareOptions.mode, p.videoData),
                                p.imageOptions, p.displayOptions,
                                p.compareOptions, p.backgroundOptions);

                            _droppedFramesUpdate(p.videoData[0].time);
                        }
                        p.render->end();
                    }
                }
                catch (const std::exception& e)
                {
                    if (auto context = p.context.lock())
                    {
                        context->log(
                            "tl::qtwidget::TimelineViewport", e.what(),
                            log::Type::Error);
                    }
                }
            }

            glViewport(0, 0, GLsizei(viewportSize.w), GLsizei(viewportSize.h));
            glClearColor(0.F, 0.F, 0.F, 0.F);
            glClear(GL_COLOR_BUFFER_BIT);

            if (p.buffer)
            {
                p.shader->bind();
                p.shader->setUniform(
                    "transform.mvp",
                    math::ortho(
                        0.F, static_cast<float>(viewportSize.w),
                        static_cast<float>(viewportSize.h), 0.F, -1.F, 1.F));

                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, p.buffer->getColorID());

                const auto mesh = geom::box(
                    math::Box2i(0, 0, viewportSize.w, viewportSize.h));
                if (!p.vbo)
                {
                    p.vbo = gl::VBO::create(
                        mesh.triangles.size() * 3,
                        gl::VBOType::Pos2_F32_UV_U16);
                }
                if (p.vbo)
                {
                    p.vbo->copy(convert(mesh, gl::VBOType::Pos2_F32_UV_U16));
                }

                if (!p.vao && p.vbo)
                {
                    p.vao = gl::VAO::create(
                        gl::VBOType::Pos2_F32_UV_U16, p.vbo->getID());
                }
                if (p.vao && p.vbo)
                {
                    p.vao->bind();
                    p.vao->draw(GL_TRIANGLES, 0, p.vbo->getSize());
                }
            }
        }

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        void TimelineViewport::enterEvent(QEvent* event)
#else
        void TimelineViewport::enterEvent(QEnterEvent* event)
#endif // QT_VERSION
        {
            TLRENDER_P();
            event->accept();
            p.mouseMode = Private::MouseMode::kNone;
        }

        void TimelineViewport::leaveEvent(QEvent* event)
        {
            TLRENDER_P();
            event->accept();
            p.mouseMode = Private::MouseMode::kNone;
        }

        void TimelineViewport::mousePressEvent(QMouseEvent* event)
        {
            TLRENDER_P();
            if (Qt::LeftButton == event->button() &&
                event->modifiers() & Qt::ControlModifier)
            {
                const float devicePixelRatio = window()->devicePixelRatio();
                p.mouseMode = Private::MouseMode::View;
                p.mousePress.x = event->x() * devicePixelRatio;
                p.mousePress.y = event->y() * devicePixelRatio;
                p.viewPosMousePress = p.viewPos;
            }
            else if (
                Qt::LeftButton == event->button() &&
                event->modifiers() & Qt::AltModifier)
            {
                const float devicePixelRatio = window()->devicePixelRatio();
                p.mouseMode = Private::MouseMode::Wipe;
                p.mousePress.x = event->x() * devicePixelRatio;
                p.mousePress.y = event->y() * devicePixelRatio;
            }
        }

        void TimelineViewport::mouseReleaseEvent(QMouseEvent*)
        {
            TLRENDER_P();
            p.mouseMode = Private::MouseMode::kNone;
        }

        void TimelineViewport::mouseMoveEvent(QMouseEvent* event)
        {
            TLRENDER_P();
            const float devicePixelRatio = window()->devicePixelRatio();
            p.mousePos.x = event->x() * devicePixelRatio;
            p.mousePos.y = event->y() * devicePixelRatio;
            switch (p.mouseMode)
            {
            case Private::MouseMode::View:
                p.viewPos.x =
                    p.viewPosMousePress.x + (p.mousePos.x - p.mousePress.x);
                p.viewPos.y =
                    p.viewPosMousePress.y + (p.mousePos.y - p.mousePress.y);
                p.doRender = true;
                update();
                Q_EMIT viewPosAndZoomChanged(p.viewPos, p.viewZoom);
                setFrameView(false);
                break;
            case Private::MouseMode::Wipe:
                if (p.player)
                {
                    const auto& ioInfo = p.player->ioInfo();
                    if (!ioInfo.video.empty())
                    {
                        const auto& imageInfo = ioInfo.video[0];
                        p.compareOptions.wipeCenter.x =
                            (p.mousePos.x - p.viewPos.x) / p.viewZoom /
                            static_cast<float>(
                                imageInfo.size.w *
                                imageInfo.size.pixelAspectRatio);
                        p.compareOptions.wipeCenter.y =
                            (p.mousePos.y - p.viewPos.y) / p.viewZoom /
                            static_cast<float>(imageInfo.size.h);
                        p.doRender = true;
                        update();
                        Q_EMIT compareOptionsChanged(p.compareOptions);
                    }
                }
                break;
            default:
                break;
            }
        }

        void TimelineViewport::wheelEvent(QWheelEvent* event)
        {
            TLRENDER_P();
            if (Qt::NoModifier == event->modifiers())
            {
                event->accept();
                const float delta = event->angleDelta().y() / 8.F / 15.F;
                const double mult = 1.1;
                const double zoom = delta < 0 ? p.viewZoom / (-delta * mult)
                                              : p.viewZoom * (delta * mult);
                setViewZoom(zoom, p.mousePos);
            }
            else if (event->modifiers() & Qt::ControlModifier)
            {
                event->accept();
                if (p.player)
                {
                    const auto t = p.player->currentTime();
                    const float delta = event->angleDelta().y() / 8.F / 15.F;
                    p.player->seek(t + otime::RationalTime(delta, t.rate()));
                }
            }
        }

        void TimelineViewport::keyPressEvent(QKeyEvent* event)
        {
            TLRENDER_P();
            switch (event->key())
            {
            case Qt::Key::Key_0:
                event->accept();
                setViewZoom(1.F, p.mousePos);
                break;
            case Qt::Key::Key_Minus:
                event->accept();
                setViewZoom(p.viewZoom / 2.F, p.mousePos);
                break;
            case Qt::Key::Key_Equal:
            case Qt::Key::Key_Plus:
                event->accept();
                setViewZoom(p.viewZoom * 2.F, p.mousePos);
                break;
            case Qt::Key::Key_Backspace:
                event->accept();
                setFrameView(true);
                break;
            default:
                QOpenGLWidget::keyPressEvent(event);
                break;
            }
        }

        math::Size2i TimelineViewport::_viewportSize() const
        {
            const float devicePixelRatio = window()->devicePixelRatio();
            return math::Size2i(
                width() * devicePixelRatio, height() * devicePixelRatio);
        }

        math::Vector2i TimelineViewport::_viewportCenter() const
        {
            const math::Size2i viewportSize = _viewportSize();
            return math::Vector2i(viewportSize.w / 2, viewportSize.h / 2);
        }

        math::Size2i TimelineViewport::_renderSize() const
        {
            TLRENDER_P();
            return timeline::getRenderSize(p.compareOptions.mode, p.videoData);
        }

        void TimelineViewport::_frameView()
        {
            TLRENDER_P();
            const math::Size2i viewportSize = _viewportSize();
            const math::Size2i renderSize = _renderSize();
            double zoom = 1.0;
            if (renderSize.w > 0)
            {
                zoom = viewportSize.w / static_cast<double>(renderSize.w);
                if (renderSize.h > 0 && zoom * renderSize.h > viewportSize.h)
                {
                    zoom = viewportSize.h / static_cast<double>(renderSize.h);
                }
            }
            const math::Vector2i c(renderSize.w / 2, renderSize.h / 2);
            const math::Vector2i viewPos(
                viewportSize.w / 2.0 - c.x * zoom,
                viewportSize.h / 2.0 - c.y * zoom);
            if (viewPos != p.viewPos || zoom != p.viewZoom)
            {
                p.viewPos = viewPos;
                p.viewZoom = zoom;
                Q_EMIT viewPosAndZoomChanged(p.viewPos, p.viewZoom);
            }
        }

        void
        TimelineViewport::_droppedFramesUpdate(const otime::RationalTime& value)
        {
            TLRENDER_P();
            if (value != time::invalidTime && p.droppedFramesData.init)
            {
                p.droppedFramesData.init = false;
                p.droppedFramesData.dropped = 0;
                Q_EMIT droppedFramesChanged(p.droppedFramesData.dropped);
            }
            else
            {
                const double frameDiff =
                    value.value() - p.droppedFramesData.frame;
                if (std::abs(frameDiff) > 1.0)
                {
                    ++(p.droppedFramesData.dropped);
                    Q_EMIT droppedFramesChanged(p.droppedFramesData.dropped);
                }
            }
            p.droppedFramesData.frame = value.value();
        }
    } // namespace qtwidget
} // namespace tl
