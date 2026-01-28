// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once


#include "mrvVk/mrvVkDefines.h"

#include "mrvViewport/mrvTimelineViewport.h"

#ifdef TLRENDER_FFMPEG
#    include "mrvVoice/mrvAnnotation.h"
#endif

#include <tlVk/OffscreenBuffer.h>

#include <tlDraw/Annotation.h>

namespace mrv
{
    namespace vulkan
    {

        //
        // This class implements a viewport using Vulkan
        //
        class Viewport : public TimelineViewport
        {
            TLRENDER_NON_COPYABLE(Viewport);

        public:
            Viewport(int X, int Y, int W, int H, const char* L = 0);
            ~Viewport();

            //! Virtual log level method
            int log_level() const FL_OVERRIDE;

            //! Virual draw method
            void draw() FL_OVERRIDE;

            //! Virtual handle event method.
            int handle(int event) FL_OVERRIDE;

            //! Virtual hide method.  Destroys ALL resources.
            void hide() FL_OVERRIDE;

            //! Set the internal system context for the widget.
            void setContext(const std::weak_ptr<system::Context>& context);

            void init_colorspace() FL_OVERRIDE;
            std::vector<const char*> get_device_extensions() FL_OVERRIDE;
            
            void prepare() FL_OVERRIDE;
            void destroy() FL_OVERRIDE;

            std::shared_ptr<vlk::OffscreenBuffer> getVideoFBO();

            std::shared_ptr<vlk::OffscreenBuffer> getAnnotationFBO();

            void setSaveOverlay(const bool save);
            
        protected:            
            void _updateHDRMetadata();

            void _createCubicEnvironmentMap();

            void _createSphericalEnvironmentMap();

            math::Matrix4x4f _createEnvironmentMap();

            math::Matrix4x4f _createTexturedRectangle();

            void _calculateColorArea(mrv::area::Info& info);

            void _drawAnaglyph(int, int) const noexcept;

            void _drawCheckerboard(int, int) noexcept;

            void _drawColumns(int, int) noexcept;

            void _drawScanlines(int, int) noexcept;

            void _drawStereoVulkan() noexcept;

            void _drawStereo3D() noexcept;

            void _drawDataWindow() noexcept;

            void _drawDisplayWindow() noexcept;

            void _drawMissingFrame(const math::Size2i& renderSize) noexcept;

            //! Crop mask, data window and display window
            void _drawOverlays(const math::Size2i& renderSize) const noexcept;

            void _drawCropMask(const math::Size2i& renderSize) const noexcept;

            void _drawHUD(const std::vector<timeline::TextInfo>& textInfos,
                const float alpha) const noexcept;
            
            void _drawHUD(float alpha) const noexcept;

            void _drawCursor(const math::Matrix4x4f& mvp) noexcept;

#ifdef TLRENDER_FFMPEG
            void _drawAnnotations(
                const std::shared_ptr<tl::vlk::OffscreenBuffer>& overlay,
                const std::shared_ptr<tl::timeline_vlk::Render>& render,
                const math::Matrix4x4f& renderMVP,
                const otime::RationalTime& time,
                const std::vector<std::shared_ptr<draw::Annotation>>&
                    annotations,
                const std::vector<std::shared_ptr<voice::Annotation> >&
                voannotations,
                const math::Size2i& renderSize);
#else
            void _drawAnnotations(
                const std::shared_ptr<tl::vlk::OffscreenBuffer>& overlay,
                const std::shared_ptr<tl::timeline_vlk::Render>& render,
                const math::Matrix4x4f& renderMVP,
                const otime::RationalTime& time,
                const std::vector<std::shared_ptr<draw::Annotation>>&
                    annotations,
                const std::vector<bool>&
                voannotations,
                const math::Size2i& renderSize);
#endif
            void _pushAnnotationShape(const std::string& cmd) const override;

            void _readPixel(image::Color4f& rgba) override;

            void _drawHelpText() const noexcept;

            void _drawRectangleOutline(
                const std::string& pipelineName,
                const math::Matrix4x4f& mvp,
                const math::Box2i& box, const image::Color4f& color)
                const noexcept;
            
            void _appendText(std::vector<timeline::TextInfo>& textInfos,
                             const std::vector<std::shared_ptr<image::Glyph> >&,
                             math::Vector2i&, const int16_t lineHeight) const;
            
            void _appendText(std::vector<timeline::TextInfo>& textInfos,
                             const std::string& text,
                             const image::FontInfo& fontInfo,
                             math::Vector2i&, const int16_t lineHeight) const;
            void _drawText(const std::vector<timeline::TextInfo>& textInfos,
                           const math::Vector2i&, const image::Color4f&) const;

            void _drawAreaSelection() const noexcept;
            void _drawSafeAreas() noexcept;
            void _drawSafeAreas(
                const float percentX, const float percentY,
                const float pixelAspectRatio, const image::Color4f& color,
                const math::Matrix4x4f& mvp, const char* label = "") noexcept;

            void _drawShape(
                const std::shared_ptr<timeline_vlk::Render>& render,
                const std::shared_ptr< draw::Shape >& shape,
                const float alphamult = 1.F,
                const float resolutionMultiplier = 1.F) noexcept;

            void _calculateColorAreaFullValues(area::Info& info) noexcept;

            void _drawWindowArea(const std::string& pipelineName,
                                 const std::string&) noexcept;

            void _compositeAnnotations(
                VkCommandBuffer cmd,
                const std::shared_ptr<tl::vlk::OffscreenBuffer>&,
                const math::Matrix4x4f& orthoMatrix,
                const std::shared_ptr<vlk::Shader>& shader,
                const std::shared_ptr<vlk::VBO>& vbo,
                const math::Size2i& viewportSize);

            void _readOverlay(
                const std::shared_ptr<tl::vlk::OffscreenBuffer>& overlay);

            // Vulkan preparation functions
            void prepare_annotation_pipeline();
            void prepare_pipeline();
            void prepare_pipeline_layout();
            void prepare_annotation_pipeline_layout();
            void prepare_load_render_pass();
            void prepare_shaders();
            void prepare_render();

            void _mapBuffer();
            void _unmapBuffer();
            void _getMonitorNits(bool quiet = true);
            
        private:
            struct VKPrivate;
            std::unique_ptr<VKPrivate> _vk;
        };

    } // namespace vulkan

} // namespace mrv
