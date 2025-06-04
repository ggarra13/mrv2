// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlQtQuick/GLFramebufferObject.h>

#include <tlQtQuick/Init.h>

#include <tlQt/TimelinePlayer.h>

#include <tlTimelineGL/Render.h>

#include <tlGL/Init.h>

#include <QOpenGLFramebufferObject>

#include <QQuickWindow>

namespace tl
{
    namespace qtquick
    {
        namespace
        {
            class Renderer : public QQuickFramebufferObject::Renderer
            {
            public:
                Renderer(const GLFramebufferObject* framebufferObject) :
                    _framebufferObject(framebufferObject)
                {
                }

                virtual ~Renderer() {}

                QOpenGLFramebufferObject*
                createFramebufferObject(const QSize& size) override
                {
                    return QQuickFramebufferObject::Renderer::
                        createFramebufferObject(size);
                }

                void render() override
                {
                    if (!_init)
                    {
                        _init = true;
                        gl::initGLAD();
                        _render =
                            timeline_gl::Render::create(qtquick::getContext());
                    }

                    QOpenGLFramebufferObject* fbo = framebufferObject();
                    const math::Size2i size(fbo->width(), fbo->height());
                    _render->begin(size);
                    _render->drawVideo(
                        {_videoData}, {math::Box2i(0, 0, size.w, size.h)});
                    _render->end();

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
                    _framebufferObject->window()->resetOpenGLState();
#endif // QT_VERSION
                }

                void synchronize(QQuickFramebufferObject*) override
                {
                    _videoData = _framebufferObject->video();
                }

            private:
                const GLFramebufferObject* _framebufferObject = nullptr;
                bool _init = false;
                timeline::VideoData _videoData;
                std::shared_ptr<timeline::IRender> _render;
            };
        } // namespace

        struct GLFramebufferObject::Private
        {
            timeline::VideoData videoData;
        };

        GLFramebufferObject::GLFramebufferObject(QQuickItem* parent) :
            QQuickFramebufferObject(parent),
            _p(new Private)
        {
            setMirrorVertically(true);
        }

        GLFramebufferObject::~GLFramebufferObject() {}

        const timeline::VideoData& GLFramebufferObject::video() const
        {
            return _p->videoData;
        }

        QQuickFramebufferObject::Renderer*
        GLFramebufferObject::createRenderer() const
        {
            return new qtquick::Renderer(this);
        }

        void GLFramebufferObject::setVideo(const timeline::VideoData& value)
        {
            _p->videoData = value;
            update();
        }
    } // namespace qtquick
} // namespace tl
