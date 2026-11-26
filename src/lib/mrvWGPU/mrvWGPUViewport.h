// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once


#include "mrvViewport/mrvTimelineViewport.h"

#include "mrvWGPU/mrvWGPUDefines.h"

#ifdef TLRENDER_FFMPEG
#    include "mrvVoice/mrvAnnotation.h"
#endif

// #include <tlWGPU/OffscreenBuffer.h>

#include <tlDraw/Annotation.h>

namespace mrv
{
    namespace webgpu
    {
        //
        // This class implements a viewport using OpenWGPU
        //
        class Viewport : public TimelineViewport
        {
            TLRENDER_NON_COPYABLE(Viewport);

        public:
            Viewport(int X, int Y, int W, int H, const char* L = 0);
            ~Viewport();

            //! Virual draw method.
            void draw() override;

            //! Virtual handle event method.
            int handle(int event) override;

            //! Virtual prepare method.
            void prepare() override;

            //! Virtual prepare method.
            void destroy() override;

            //! Set the internal system context for the widget.
            void setContext(const std::weak_ptr<system::Context>& context);

            //! Refresh window by clearing the associated resources.
            void refresh() override;

            //! Poll the color picker in timeout.
            void pollPick();

            //! Return Windows' fontsystem.
            std::shared_ptr<image::FontSystem > getFontSystem();

        protected:
            void prepare_annotation_pipeline();
            void prepare_pipeline();
            void prepare_shaders();

            void _createPBOs(const math::Size2i& renderSize);
            void _createOverlayPBO(const math::Size2i& renderSize);

            void _createCubicEnvironmentMap();

            void _createSphericalEnvironmentMap();

            math::Matrix4x4f _createEnvironmentMap();

            math::Matrix4x4f _createTexturedRectangle();

            void _calculateColorArea(mrv::area::Info& info);

            void _drawAnawgpuyph(int, int) const noexcept;

            void _drawCheckerboard(int, int) const noexcept;

            void _drawColumns(int, int) const noexcept;

            void _drawScanlines(int, int) const noexcept;

            void _drawStereoOpenWGPU() noexcept;

            void _drawStereo3D() const noexcept;

            void _drawDataWindow() const noexcept;

            void _drawDisplayWindow() const noexcept;

            void _drawMissingFrame(const math::Size2i& renderSize) const noexcept;

            //! Crop mask, data window and display window
            void _drawOverlays(const math::Size2i& renderSize) const noexcept;

            void _drawCropMask(const math::Size2i& renderSize) const noexcept;

            void _drawHUD(
                const std::vector<timeline::TextInfo>& textInfos,
                const float alpha) const noexcept;

            void _drawHUD(float alpha) const noexcept;

            void _drawCursor(const math::Matrix4x4f& mvp) const noexcept;

            void _pushAnnotationShape(const std::string& cmd) const override;

            void _readPixel(image::Color4f& rgba) override;

            void _drawRectanwgpueOutline(
                const math::Box2i& box, const image::Color4f& color,
                const math::Matrix4x4f& mvp) const noexcept;
            void _appendText(std::vector<timeline::TextInfo>& textInfos,
                             const std::vector<std::shared_ptr<image::Glyph> >&,
                             math::Vector2i&, const int16_t lineHeight) const;

            void _appendText(std::vector<timeline::TextInfo>& textInfos,
                             const std::string& text,
                             const image::FontInfo& fontInfo,
                             math::Vector2i&, const int16_t lineHeight) const;
            void _drawText(const std::vector<timeline::TextInfo>& textInfos,
                           const math::Vector2i&, const image::Color4f&) const;

            void _drawSafeAreas() const noexcept;
            void _drawSafeAreas(
                const float percentX, const float percentY,
                const float pixelAspectRatio, const image::Color4f& color,
                const math::Matrix4x4f& mvp, const char* label = "") const noexcept;

            void _mapBuffer() const noexcept;
            void _unmapBuffer() const noexcept;

            void _drawShape(
                const std::shared_ptr< draw::Shape >& shape,
                const float alphamult = 1.F,
                const float resolutionMultiplier = 1.F) noexcept;

            void _calculateColorAreaFullValues(area::Info& info) noexcept;

            void _drawWindowArea(const std::string&) const noexcept;

            void _compositeAnnotations(
                const math::Matrix4x4f& shaderMatrix,
                const math::Size2i& viewportSize);

#if 0   // \@wmrv2

#ifdef TLRENDER_FFMPEG
            void _drawAnnotations(
                const std::shared_ptr<tl::wgpu::OffscreenBuffer>& overlay,
                const math::Matrix4x4f& renderMVP,
                const OTIO_NS::RationalTime& time,
                const std::vector<std::shared_ptr<draw::Annotation>>& annotations,
                const std::vector<std::shared_ptr<voice::Annotation>>& voannotations,
                const math::Size2i& renderSize);
#else
            void _drawAnnotations(
                const std::shared_ptr<tl::wgpu::OffscreenBuffer>& overlay,
                const math::Matrix4x4f& renderMVP,
                const OTIO_NS::RationalTime& time,
                const std::vector<std::shared_ptr<draw::Annotation>>& annotations,
                const std::vector<bool>& voannotations,
                const math::Size2i& renderSize);
#endif
            void _compositeAnnotations(
                const std::shared_ptr<tl::wgpu::OffscreenBuffer>&,
                const math::Matrix4x4f& orthoMatrix,
                const math::Size2i& viewportSize);

            void _compositeOverlay(const std::shared_ptr<tl::wgpu::OffscreenBuffer>&,
                                   const math::Matrix4x4f& identity,
                                   const math::Size2i& viewportSize);
#endif

        private:
            struct WGPUPrivate;
            std::unique_ptr<WGPUPrivate> _wgpu;
        };

    } // namespace webgpu

} // namespace mrv
