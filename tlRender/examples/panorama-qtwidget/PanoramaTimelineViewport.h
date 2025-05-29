// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimelinePlayer.h>

#include <tlTimelineGL/Render.h>

#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Shader.h>

#include <tlCore/Mesh.h>

#include <QOpenGLWidget>
#include <QOpenGLFunctions_4_1_Core>

namespace tl
{
    namespace examples
    {
        //! Example panorama timeline rendering.
        namespace panorama_qtwidget
        {
            //! Panorama timeline viewport.
            class PanoramaTimelineViewport : public QOpenGLWidget,
                                             protected QOpenGLFunctions_4_1_Core
            {
                Q_OBJECT

            public:
                PanoramaTimelineViewport(
                    const std::shared_ptr<system::Context>&,
                    QWidget* parent = nullptr);

                //! Set the OpenColorIO options.
                void setOCIOOptions(const timeline::OCIOOptions&);

                //! Set the LUT options.
                void setLUTOptions(const timeline::LUTOptions&);

                //! Set the image options.
                void setImageOptions(const timeline::ImageOptions&);

                //! Set the timeline player.
                void setPlayer(const QSharedPointer<qt::TimelinePlayer>&);

            private Q_SLOTS:
                void _currentVideoCallback(
                    const std::vector<tl::timeline::VideoData>&);

            protected:
                void initializeGL() override;
                void paintGL() override;
                void mousePressEvent(QMouseEvent*) override;
                void mouseReleaseEvent(QMouseEvent*) override;
                void mouseMoveEvent(QMouseEvent*) override;

            private:
                std::weak_ptr<system::Context> _context;
                timeline::OCIOOptions _ocioOptions;
                timeline::LUTOptions _lutOptions;
                timeline::ImageOptions _imageOptions;
                QSharedPointer<qt::TimelinePlayer> _player;
                image::Size _videoSize;
                std::vector<timeline::VideoData> _videoData;
                math::Vector2f _cameraRotation;
                float _cameraFOV = 45.F;
                geom::TriangleMesh3 _sphereMesh;
                std::shared_ptr<gl::VBO> _sphereVBO;
                std::shared_ptr<gl::VAO> _sphereVAO;
                std::shared_ptr<gl::Shader> _shader;
                std::shared_ptr<gl::OffscreenBuffer> _buffer;
                std::shared_ptr<timeline_gl::Render> _render;
                math::Vector2i _mousePosPrev;
            };
        } // namespace panorama_qtwidget
    } // namespace examples
} // namespace tl
