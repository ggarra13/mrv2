// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "PanoramaTimelineViewport.h"

#include <tlGL/Init.h>

#include <QMouseEvent>
#include <QSurfaceFormat>

namespace tl
{
    namespace examples
    {
        namespace panorama_qtwidget
        {
            PanoramaTimelineViewport::PanoramaTimelineViewport(
                const std::shared_ptr<system::Context>& context,
                QWidget* parent) :
                QOpenGLWidget(parent)
            {
                _context = context;

                QSurfaceFormat surfaceFormat;
                surfaceFormat.setMajorVersion(4);
                surfaceFormat.setMinorVersion(1);
                surfaceFormat.setProfile(QSurfaceFormat::CoreProfile);
                setFormat(surfaceFormat);
            }

            void PanoramaTimelineViewport::setOCIOOptions(
                const timeline::OCIOOptions& value)
            {
                if (value == _ocioOptions)
                    return;
                _ocioOptions = value;
                update();
            }

            void PanoramaTimelineViewport::setLUTOptions(
                const timeline::LUTOptions& value)
            {
                if (value == _lutOptions)
                    return;
                _lutOptions = value;
                update();
            }

            void PanoramaTimelineViewport::setImageOptions(
                const timeline::ImageOptions& value)
            {
                if (value == _imageOptions)
                    return;
                _imageOptions = value;
                update();
            }

            void PanoramaTimelineViewport::setPlayer(
                const QSharedPointer<qt::TimelinePlayer>& player)
            {
                if (_player)
                {
                    disconnect(
                        _player.get(),
                        SIGNAL(currentVideoChanged(
                            const std::vector<tl::timeline::VideoData>&)),
                        this,
                        SLOT(_currentVideoCallback(
                            const std::vector<tl::timeline::VideoData>&)));
                }
                _player = player;
                _videoData.clear();
                if (_player)
                {
                    const auto& ioInfo = _player->ioInfo();
                    _videoSize = !ioInfo.video.empty() ? ioInfo.video[0].size
                                                       : image::Size();
                    _videoData = _player->currentVideo();
                    connect(
                        _player.get(),
                        SIGNAL(currentVideoChanged(
                            const std::vector<tl::timeline::VideoData>&)),
                        SLOT(_currentVideoCallback(
                            const std::vector<tl::timeline::VideoData>&)));
                }
                update();
            }

            void PanoramaTimelineViewport::_currentVideoCallback(
                const std::vector<timeline::VideoData>& value)
            {
                _videoData = value;
                update();
            }

            void PanoramaTimelineViewport::initializeGL()
            {
                initializeOpenGLFunctions();
                gl::initGLAD();

                try
                {
                    // Create the sphere mesh.
                    _sphereMesh = geom::sphere(10.F, 100, 100);
                    auto vboData = convert(
                        _sphereMesh, gl::VBOType::Pos3_F32_UV_U16,
                        math::SizeTRange(0, _sphereMesh.triangles.size() - 1));
                    _sphereVBO = gl::VBO::create(
                        _sphereMesh.triangles.size() * 3,
                        gl::VBOType::Pos3_F32_UV_U16);
                    _sphereVBO->copy(vboData);
                    _sphereVAO = gl::VAO::create(
                        gl::VBOType::Pos3_F32_UV_U16, _sphereVBO->getID());

                    // Create the renderer.
                    if (auto context = _context.lock())
                    {
                        _render = timeline_gl::Render::create(context);
                    }

                    // Create the shader.
                    const std::string vertexSource =
                        "#version 410\n"
                        "\n"
                        "// Inputs\n"
                        "in vec3 vPos;\n"
                        "in vec2 vTexture;\n"
                        "\n"
                        "// Outputs\n"
                        "out vec2 fTexture;\n"
                        "\n"
                        "// Uniforms\n"
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
                        "// Inputs\n"
                        "in vec2 fTexture;\n"
                        "\n"
                        "// Outputs\n"
                        "out vec4 fColor;\n"
                        "\n"
                        "// Uniforms\n"
                        "uniform sampler2D textureSampler;\n"
                        "\n"
                        "void main()\n"
                        "{\n"
                        "    fColor = texture(textureSampler, fTexture);\n"
                        "}\n";
                    _shader = gl::Shader::create(vertexSource, fragmentSource);
                }
                catch (const std::exception& e)
                {
                    // Re-throw the exception to be caught in main().
                    throw e;
                }
            }

            void PanoramaTimelineViewport::paintGL()
            {
                try
                {
                    // Create the offscreen buffer.
                    math::Size2i offscreenBufferSize(
                        _videoSize.w, _videoSize.h);
                    gl::OffscreenBufferOptions offscreenBufferOptions;
                    offscreenBufferOptions.colorType =
                        image::PixelType::RGBA_F32;
                    if (gl::doCreate(
                            _buffer, offscreenBufferSize,
                            offscreenBufferOptions))
                    {
                        _buffer = gl::OffscreenBuffer::create(
                            offscreenBufferSize, offscreenBufferOptions);
                    }

                    // Render the video data into the offscreen buffer.
                    if (_buffer)
                    {
                        gl::OffscreenBufferBinding binding(_buffer);
                        _render->begin(offscreenBufferSize);
                        _render->setOCIOOptions(_ocioOptions);
                        _render->setLUTOptions(_lutOptions);
                        _render->drawVideo(
                            {_videoData},
                            {math::Box2i(0, 0, _videoSize.w, _videoSize.h)},
                            {_imageOptions});
                        _render->end();
                    }
                }
                catch (const std::exception& e)
                {
                    // Re-throw the exception to be caught in main().
                    throw e;
                }

                // Render a sphere using the offscreen buffer as a texture.
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_SCISSOR_TEST);
                glDisable(GL_BLEND);
                const float devicePixelRatio = window()->devicePixelRatio();
                const math::Size2i windowSize(
                    width() * devicePixelRatio, height() * devicePixelRatio);
                glViewport(
                    0, 0, static_cast<GLsizei>(windowSize.w),
                    static_cast<GLsizei>(windowSize.h));
                glClearColor(0.F, 0.F, 0.F, 0.F);
                glClear(GL_COLOR_BUFFER_BIT);
                math::Matrix4x4f vm;
                vm = vm * math::translate(math::Vector3f(0.F, 0.F, 0.F));
                vm = vm * math::rotateX(_cameraRotation.x);
                vm = vm * math::rotateY(_cameraRotation.y);
                const auto pm = math::perspective(
                    _cameraFOV,
                    windowSize.w /
                        static_cast<float>(windowSize.h > 0 ? windowSize.h : 1),
                    .1F, 10000.F);
                _shader->bind();
                _shader->setUniform("transform.mvp", pm * vm);
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, _buffer->getColorID());
                _sphereVAO->bind();
                _sphereVAO->draw(
                    GL_TRIANGLES, 0, _sphereMesh.triangles.size() * 3);
            }

            void PanoramaTimelineViewport::mousePressEvent(QMouseEvent* event)
            {
                const float devicePixelRatio = window()->devicePixelRatio();
                _mousePosPrev.x = event->x() * devicePixelRatio;
                _mousePosPrev.y = event->y() * devicePixelRatio;
            }

            void PanoramaTimelineViewport::mouseReleaseEvent(QMouseEvent*) {}

            void PanoramaTimelineViewport::mouseMoveEvent(QMouseEvent* event)
            {
                const float devicePixelRatio = window()->devicePixelRatio();
                _cameraRotation.x +=
                    (event->y() * devicePixelRatio - _mousePosPrev.y) / 20.F *
                    -1.F;
                _cameraRotation.y +=
                    (event->x() * devicePixelRatio - _mousePosPrev.x) / 20.F *
                    -1.F;
                _mousePosPrev.x = event->x() * devicePixelRatio;
                _mousePosPrev.y = event->y() * devicePixelRatio;
            }
        } // namespace panorama_qtwidget
    } // namespace examples
} // namespace tl
