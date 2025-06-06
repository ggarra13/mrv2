// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>

#include <Imath/ImathMatrix.h>
#include <Imath/ImathVec.h>

#include <tlCore/Image.h>
#include <tlCore/Math.h>

#include "mrvWidgets/mrvVectorscope.h"

#include "mrViewer.h"

#include "mrvCore/mrvColor.h"
#include "mrvCore/mrvI8N.h"

namespace mrv
{
    struct Vectorscope::Private
    {
        int diameter;
        math::Box2i box;
        image::PixelType pixelType;
        uint8_t* image = nullptr;
        size_t dataSize = 0;
        ViewerUI* ui;
    };

    Vectorscope::Vectorscope(int X, int Y, int W, int H, const char* L) :
        Fl_Group(X, Y, W, H, L),
        _p(new Private)
    {
        tooltip(_("Mark an area in the image with SHIFT + the left mouse "
                  "button"));
    }

    Vectorscope::~Vectorscope()
    {
        TLRENDER_P();

        free(p.image);
    }

    void Vectorscope::main(ViewerUI* m)
    {
        _p->ui = m;
    }

    ViewerUI* Vectorscope::main()
    {
        return _p->ui;
    }

    void Vectorscope::draw()
    {
        TLRENDER_P();
        fl_rectf(x(), y(), w(), h(), 0, 0, 0);

        p.diameter = h();
        if (w() < p.diameter)
            p.diameter = w();

        draw_grid();
        if (p.image)
        {
            draw_pixels();
        }
    }

    void Vectorscope::update(const area::Info& info)
    {
        TLRENDER_P();

        MyViewport* view = p.ui->uiView;
        const void* viewImage = view->image();
        p.box = info.box;
        p.pixelType = info.pixelType;

        if (!viewImage)
        {
            redraw();
            return;
        }

        const int channelCount = image::getChannelCount(p.pixelType);
        const int byteCount = image::getBitDepth(p.pixelType) / 8;
        
        const size_t dataSize = info.box.w() * info.box.h() * channelCount * byteCount;
        if (dataSize != p.dataSize)
        {
            p.dataSize = dataSize;
            free(p.image);
            p.image = reinterpret_cast<uint8_t*>(malloc(dataSize));
        }
        memcpy(p.image, viewImage, dataSize);

        redraw();
    }

    void Vectorscope::draw_pixel(image::Color4f& color) const noexcept
    {
        TLRENDER_P();

        using namespace Imath;

        if (color.r < 0)
            color.r = 0;
        if (color.g < 0)
            color.g = 0;
        if (color.b < 0)
            color.b = 0;

        if (color.r > 1.F)
            color.r = 1.F;
        if (color.g > 1.F)
            color.g = 1.F;
        if (color.b > 1.F)
            color.b = 1.F;

#ifdef VULKAN_BACKEND
        const uint8_t r = color.r * 255.F;
        const uint8_t g = color.g * 255.F;
        const uint8_t b = color.b * 255.F;
#elif OPENGL_BACKEND
        const uint8_t b = color.r * 255.F;
        const uint8_t g = color.g * 255.F;
        const uint8_t r = color.b * 255.F;
#else
        const uint8_t r = color.r * 255.F;
        const uint8_t g = color.g * 255.F;
        const uint8_t b = color.b * 255.F;
#endif

        image::Color4f hsv = color::rgb::to_hsv(color);

        int W = p.diameter / 2;
        int H = p.diameter / 2;

#ifdef __linux__
        M44f m;

        // Translate to center
        m.translate(V3f(x() + W, y() + H, 0));

        // Rotate base on hue
        m.rotate(V3f(0, 0, math::deg2rad(-15.0 - hsv.r * 360.0f)));

        // Scale based on saturation
        float s = hsv.g * 0.375f;
        m.scale(V3f(s, s, 1));

        V3f pos(0, p.diameter, 0);

        pos = pos * m;

        fl_rectf(pos.x, pos.y, 1, 1, r, g, b);
#else
        fl_push_matrix();

        // Position at center of circle
        fl_translate(x() + W, y() + H);

        // Rotate base on hue
        fl_rotate(15.0 + hsv.r * 360.0f);

        // Scale based on saturation
        float s = hsv.g * 0.375f;
        fl_scale(s, s);

        fl_color(r, g, b);
        fl_begin_points();
        fl_vertex(0, p.diameter);
        fl_end_points();

        fl_pop_matrix();
#endif
    }

    void Vectorscope::draw_pixels() const noexcept
    {
        TLRENDER_P();

        if (!p.box.isValid())
            return;

        int stepX = (p.box.max.x - p.box.min.x) / p.diameter;
        int stepY = (p.box.max.y - p.box.min.y) / p.diameter;
        if (stepX < 1)
            stepX = 1;
        if (stepY < 1)
            stepY = 1;

        const int channelCount = image::getChannelCount(p.pixelType);
        const int byteCount = image::getBitDepth(p.pixelType) / 8;

        const uint32_t W = p.box.w();
        const uint32_t H = p.box.h();
        
        image::Color4f rgba;

        // We don't use all pixels with dataSize as it is too slow.  We
        // use a step instead.
        for (uint32_t Y = 0; Y < H; Y += stepY)
        {
            for (uint32_t X = 0; X < W; X += stepX)
            {
                const size_t offset = (X + Y * W) * channelCount * byteCount;
                rgba = color::fromVoidPtr(p.image + offset, p.pixelType);
                draw_pixel(rgba);
            }
        }
    }

    void Vectorscope::draw_grid() noexcept
    {
        TLRENDER_P();

        fl_color(255, 255, 255);
        fl_arc(x(), y(), p.diameter, p.diameter, 0, 360);

        fl_line_style(0);

        int R = p.diameter / 2;
        int W = R;
        int H = R;

        float angle = 35;
        // Draw diagonal center lines
        for (int i = 0; i < 8; ++i, angle += 90)
        {
            fl_push_matrix();
            fl_translate(x() + W, y() + H);
            fl_rotate(angle);
            fl_begin_line();
            fl_vertex(0, 4);
            fl_vertex(0, R);
            fl_end_line();
            fl_pop_matrix();
        }

        // Draw cross
        fl_push_matrix();
        fl_translate(x(), y());
        fl_begin_line();
        fl_vertex(W, 0);
        fl_vertex(W, p.diameter);
        fl_end_line();
        fl_begin_line();
        fl_vertex(0, H);
        fl_vertex(p.diameter, H);
        fl_end_line();
        fl_pop_matrix();

        int RW = int(p.diameter * 0.05f);
        int RH = RW;

        fl_push_matrix();
        fl_translate(x() + W, y() + H);

        static const char* names[] = {
            "B", // B
            "Y", // C
            "C", // Y
            "R", // R
            "G", // G
            "M"  // M
        };

        int B = int(W * 0.75);

        int CX = x() + W;
        int CY = y() + H;

        const int coords[][2] = {
            {CX, CY + B},               // B
            {CX - 10, CY - B},          // Y
            {CX + B, CY + 10},          // C
            {CX - B, CY - 40},          // R
            {CX + B - 20, CY - B + 20}, // G
            {CX - B + 10, CY + B - 10}, // M
        };

        // Draw rectangles with letters near them
        angle = 15;
        for (int i = 0; i < 6; ++i, angle += 60)
        {
            fl_push_matrix();
            fl_rotate(angle);
            fl_translate(0, B);

            fl_color(255, 255, 255);
            fl_begin_line();
            fl_vertex(-RW, -RH);
            fl_vertex(RW, -RH);
            fl_vertex(RW, RH);
            fl_vertex(-RW, RH);
            fl_vertex(-RW, -RH);
            fl_end_line();
            fl_pop_matrix();

            fl_font(FL_HELVETICA, 12);
            fl_color(255, 255, 0);

            fl_draw(names[i], coords[i][0], coords[i][1]);
        }

        fl_pop_matrix();
    }

} // namespace mrv
