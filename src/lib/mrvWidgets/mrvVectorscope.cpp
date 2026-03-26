// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.



#include "mrvWidgets/mrvVectorscope.h"

#include "mrViewer.h"

#include "mrvCore/mrvColorSpaces.h"
#include "mrvCore/mrvColor.h"

#include <tlCore/Image.h>
#include <tlCore/Math.h>

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>


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
    
    void Vectorscope::resize(int X, int Y, int W, int H)
    {
        TLRENDER_P();
        
        W = std::min(W, H);
        if (W < 250) W = 250;
        H = W;
        
        p.diameter = W;

        Fl_Group::resize(X, Y, W, H);
    }


    void Vectorscope::draw()
    {
        TLRENDER_P();
        fl_rectf(x(), y(), w(), h(), 0, 0, 0);

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

        image::Color4f hsv = color::rgb::to_hsv(color);

        int centerX = x() + p.diameter / 2;
        int centerY = y() + p.diameter / 2;

        // Standard vectorscope hue: 0° = Red at ~11 o'clock (counter-clockwise)
        // Many scopes use ~103.7°-120° for red depending on YIQ/YPbPr reference.
        // This value (103.5°) puts pure red near the top-leftish (standard "top" position).
        constexpr float kRedOffsetDegrees = 103.5f;   // Adjust slightly if you prefer exact 11 o'clock
        constexpr float kFullTurn = 360.0f;

        float hueDegrees = hsv.r * kFullTurn;                     // 0..360
        float angleDegrees = kRedOffsetDegrees - hueDegrees;      // Counter-clockwise from red
        float angleRadians = math::deg2rad(angleDegrees);

        // Saturation scaling - tuned so vivid colors reach near the edge
        // 0.42f works well for most content (you can expose as a preference later)
        float saturationRadius = hsv.g * 0.42f * (p.diameter / 2.0f);

        // Polar to cartesian
        float dx = saturationRadius * std::sin(angleRadians);
        float dy = -saturationRadius * std::cos(angleRadians);   // Negative because FLTK y increases downward

        int posX = centerX + static_cast<int>(dx);
        int posY = centerY + static_cast<int>(dy);

        const uint8_t r = static_cast<uint8_t>(color.r * 255.0f);
        const uint8_t g = static_cast<uint8_t>(color.g * 255.0f);
        const uint8_t b = static_cast<uint8_t>(color.b * 255.0f);

        int pixel_size = p.diameter / 270;
        if (pixel_size <= 1)
        {
            fl_rectf(posX, posY, 1, 1, r, g, b);
        }
        else
        {
            fl_rectf(posX - pixel_size / 2, posY - pixel_size / 2,
                     pixel_size, pixel_size, r, g, b);
        }
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

        int cx = x() + p.diameter / 2;
        int cy = y() + p.diameter / 2;
        int radius = p.diameter / 2;

        // Outer circle (white)
        fl_color(255, 255, 255);
        fl_arc(x(), y(), p.diameter, p.diameter, 0, 360);

        // Center cross (subtle)
        fl_color(200, 200, 200);
        fl_line(cx - radius, cy, cx + radius, cy);
        fl_line(cx, cy - radius, cx, cy + radius);

        // Radial lines to the six color targets (every 60°)
        fl_color(255, 255, 255);
        fl_line_style(FL_SOLID, 1);

        for (int i = 0; i < 6; ++i)
        {
            float angle = math::deg2rad(103.5f - i * 60.0f);   // Same offset as plotting
            int x1 = cx + static_cast<int>(std::sin(angle) * (radius * 0.05f));
            int y1 = cy - static_cast<int>(std::cos(angle) * (radius * 0.05f));
            int x2 = cx + static_cast<int>(std::sin(angle) * (radius * 0.92f));
            int y2 = cy - static_cast<int>(std::cos(angle) * (radius * 0.92f));

            fl_line(x1, y1, x2, y2);
        }

        // Small boxes at color target positions (approximating 75-100% targets)
        int boxSize = std::max(16, p.diameter / 60);
        fl_color(255, 255, 100);   // yellowish for visibility

        static const char* names[] = { "R", "M", "B", "C", "G", "Y" };

        for (int i = 0; i < 6; ++i)
        {
            float angle = math::deg2rad(103.5f - i * 60.0f);
            int tx = cx + static_cast<int>(std::sin(angle) * (radius * 0.85f));
            int ty = cy - static_cast<int>(std::cos(angle) * (radius * 0.85f));

            // Small target box
            fl_rect(tx - boxSize/2, ty - boxSize/2, boxSize, boxSize);

            // Labels - positioned a bit outside the circle
            int labelOffset = radius * 0.95f;
            int lx = cx + static_cast<int>(std::sin(angle) * labelOffset);
            int ly = cy - static_cast<int>(std::cos(angle) * labelOffset) + 5;  // slight vertical tweak

            fl_font(FL_HELVETICA, 12);
            fl_color(255, 255, 0);
            fl_draw(names[i], lx - 6, ly);   // centered roughly
        }

        fl_line_style(0);  // reset
    }

} // namespace mrv
