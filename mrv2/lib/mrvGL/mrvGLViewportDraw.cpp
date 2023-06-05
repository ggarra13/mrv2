
#include "mrvGL/mrvGLViewport.h"
#include "mrvGL/mrvGLViewportPrivate.h"
#include "mrvGL/mrvTimelineViewportPrivate.h"
#include "mrvGL/mrvGLUtil.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrViewer.h"

namespace mrv
{
    void
    Viewport::_drawMissingFrame(const imaging::Size& renderSize) const noexcept
    {
        TLRENDER_P();
        MRV2_GL();

        gl.render->drawVideo(
            {p.lastVideoData},
            timeline::getBBoxes(p.compareOptions.mode, _getTimelineSizes()),
            p.imageOptions, p.displayOptions, p.compareOptions);

        if (p.missingFrameType == MissingFrameType::kScratchedFrame)
        {
            imaging::Color4f color(1, 0, 0, 0.8);
            drawLine(
                gl.render, math::Vector2i(0, 0),
                math::Vector2i(renderSize.w, renderSize.h), color, 4);
            drawLine(
                gl.render, math::Vector2i(0, renderSize.h),
                math::Vector2i(renderSize.w, 0), color, 4);
        }
    }

    void Viewport::_drawCursor(const math::Matrix4x4f& mvp) const noexcept
    {
        MRV2_GL();
        TLRENDER_P();
        if (p.actionMode != ActionMode::kScrub &&
            p.actionMode != ActionMode::kText &&
            p.actionMode != ActionMode::kSelection &&
            p.actionMode != ActionMode::kRotate && Fl::belowmouse() == this)
        {
            const imaging::Color4f color(1.F, 1.F, 1.F, 1.0F);
            std_any value;
            value = p.ui->app->settingsObject()->value(kPenSize);
            const float pen_size = std_any_cast<int>(value);
            p.mousePos = _getFocus();
            const auto& pos = _getRaster();
            gl.render->setTransform(mvp);
            drawCursor(gl.render, pos, pen_size / 2.0, color);
        }
    }

    void Viewport::_drawRectangleOutline(
        const math::BBox2i& box, const imaging::Color4f& color,
        const math::Matrix4x4f& mvp) const noexcept
    {
        MRV2_GL();
#if USE_ONE_PIXEL_LINES
        gl.outline.drawRect(box, color, mvp);
#else
        drawRectOutline(gl.render, box, color, 2.F, mvp);
#endif
    }

    math::Matrix4x4f Viewport::_drawTexturedRectangle()
    {
        TLRENDER_P();
        MRV2_GL();

        const auto& renderSize = getRenderSize();
        const auto& viewportSize = getViewportSize();

        const auto& mesh =
            geom::bbox(math::BBox2i(0, 0, renderSize.w, renderSize.h));
        if (!gl.vbo)
        {
            gl.vbo = gl::VBO::create(
                mesh.triangles.size() * 3, gl::VBOType::Pos2_F32_UV_U16);
        }
        if (gl.vbo)
        {
            gl.vbo->copy(convert(mesh, gl::VBOType::Pos2_F32_UV_U16));
        }

        if (!gl.vao && gl.vbo)
        {
            gl.vao =
                gl::VAO::create(gl::VBOType::Pos2_F32_UV_U16, gl.vbo->getID());
        }

        math::Matrix4x4f vm;
        vm =
            vm * math::translate(math::Vector3f(p.viewPos.x, p.viewPos.y, 0.F));
        vm = vm * math::scale(math::Vector3f(p.viewZoom, p.viewZoom, 1.F));
        const auto pm = math::ortho(
            0.F, static_cast<float>(viewportSize.w), 0.F,
            static_cast<float>(viewportSize.h), -1.F, 1.F);
        return pm * vm;
    }

} // namespace mrv
