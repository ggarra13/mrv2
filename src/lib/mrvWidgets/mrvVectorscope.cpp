// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

// ─────────────────────────────────────────────────────────────────────────────
// Changes vs. original
// ─────────────────────────────────────────────────────────────────────────────
// • Added VectorscopeMethod enum (HSV | ITU601 | ITU709) 
// • Private now carries a `method` field (default: ITU709)
// • setMethod() / method() accessors – declare in mrvVectorscope.h
// • draw_pixel() branches on the active method:
//     HSV  – original polar-from-hue approach (kept for reference / comparison)
//     ITU601 – ITU-R BT.601 luma + chroma; plots Cb on X, Cr on Y (broadcast
//              standard vectorscope convention)
//     ITU709 – ITU-R BT.709 luma + chroma; plots Cb on X, Cr on Y (broadcast
//              standard vectorscope convention)
// • draw_grid() draws method-appropriate target boxes:
//     HSV  – original evenly-spaced 60° approach
//     ITU601 – boxes should be placed at the *actual* BT.601 Cb/Cr coordinates of the
//               six primary/secondary colours at 100 % saturation
//     ITU709 – boxes should be placed at the *actual* BT.709 Cb/Cr coordinates of the
//               six primary/secondary colours at 100 % saturation
// ─────────────────────────────────────────────────────────────────────────────

#include "mrvWidgets/mrvVectorscope.h"

#include "mrViewer.h"

#include "mrvCore/mrvColorSpaces.h"
#include "mrvCore/mrvColor.h"

#include <tlCore/Image.h>
#include <tlCore/Math.h>

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>

#include <cmath>

namespace mrv
{
    // ─────────────────────────────────────────────────────────────────────────
    // Six colour targets (100 % saturation, BT.601).
    // Cb/Cr are pre-normalised to [-1 … +1] (divided by 0.5 already).
    // ─────────────────────────────────────────────────────────────────────────
    struct ColorTarget
    {
        const char* name;
        float       r, g, b;       // source colour
        uint8_t     labelR, labelG, labelB;
    };

    static constexpr ColorTarget kColorTargets[6] = {
        { "R",  1.f, 0.f, 0.f,  255, 80,  80  },
        { "Y",  1.f, 1.f, 0.f,  255, 255, 80  },
        { "G",  0.f, 1.f, 0.f,  80,  255, 80  },
        { "C",  0.f, 1.f, 1.f,  80,  255, 255 },
        { "B",  0.f, 0.f, 1.f,  80,  80,  255 },
        { "M",  1.f, 0.f, 1.f,  255, 80,  255 },
    };
    // Compute the max chroma vector length across all 6 targets
    // for the active method, so the outermost colour (G/M in BT.709)
    // always lands at exactly kScale * R regardless of standard.
    static float maxChromaRadius(VectorscopeMethod method)
    {
        float maxR = 0.f;
        for (const auto& ct : kColorTargets)
        {
            image::Color4f src(ct.r, ct.g, ct.b);
            image::Color4f ycbcr = (method == VectorscopeMethod::ITU709)
                                   ? color::rgb::to_ITU709(src)
                                   : color::rgb::to_ITU601(src);

            // ycbcr.g = Cb, ycbcr.b = Cr
            float r = std::sqrt(ycbcr.g * ycbcr.g + ycbcr.b * ycbcr.b);
            if (r > maxR) maxR = r;
        }
        return maxR;   // ≈ 0.500 for BT.601, ≈ 0.596 for BT.709
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Private
    // ─────────────────────────────────────────────────────────────────────────
    struct Vectorscope::Private
    {
        int               diameter   = 250;
        math::Box2i       box;
        image::PixelType  pixelType;
        uint8_t*          image      = nullptr;
        size_t            dataSize   = 0;
        ViewerUI*         ui         = nullptr;
        float             chromaNorm = 0.596f; // updated by setMethod
        VectorscopeMethod method     = VectorscopeMethod::ITU709;
    };

    // ─────────────────────────────────────────────────────────────────────────
    // Construction / destruction
    // ─────────────────────────────────────────────────────────────────────────
    Vectorscope::Vectorscope(int X, int Y, int W, int H, const char* L) :
        Fl_Group(X, Y, W, H, L),
        _p(new Private)
    {
        end();
        tooltip(_("Mark an area in the image with SHIFT + the left mouse "
                  "button"));
    }

    Vectorscope::~Vectorscope()
    {
        TLRENDER_P();
        free(p.image);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Method accessors
    // ─────────────────────────────────────────────────────────────────────────
    void Vectorscope::setMethod(VectorscopeMethod m)
    {
        _p->method = m;
        if (m != VectorscopeMethod::HSV)
            _p->chromaNorm = maxChromaRadius(m);
        redraw();
    }

    VectorscopeMethod Vectorscope::method() const
    {
        return _p->method;
    }

    // ─────────────────────────────────────────────────────────────────────────
    // UI plumbing
    // ─────────────────────────────────────────────────────────────────────────
    void Vectorscope::main(ViewerUI* m) { _p->ui = m; }
    ViewerUI* Vectorscope::main()       { return _p->ui; }

    void Vectorscope::resize(int X, int Y, int W, int H)
    {
        TLRENDER_P();

        W = std::min(W, H);
        if (W < 250) W = 250;
        H = W;

        p.diameter = W;
        Fl_Group::resize(X, Y, W, H);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // draw()
    // ─────────────────────────────────────────────────────────────────────────
    void Vectorscope::draw()
    {
        TLRENDER_P();
        fl_rectf(x(), y(), w(), h(), 0, 0, 0);

        draw_grid();
        if (p.image)
            draw_pixels();
    }

    // ─────────────────────────────────────────────────────────────────────────
    // update()
    // ─────────────────────────────────────────────────────────────────────────
    void Vectorscope::update(const area::Info& info)
    {
        TLRENDER_P();

        MyViewport* view   = p.ui->uiView;
        const void* viewImage = view->image();
        p.box       = info.box;
        p.pixelType = info.pixelType;

        if (!viewImage)
        {
            redraw();
            return;
        }

        const int channelCount = image::getChannelCount(p.pixelType);
        const int byteCount    = image::getBitDepth(p.pixelType) / 8;

        const size_t dataSize =
            info.box.w() * info.box.h() * channelCount * byteCount;

        if (dataSize != p.dataSize)
        {
            p.dataSize = dataSize;
            free(p.image);
            p.image = reinterpret_cast<uint8_t*>(malloc(dataSize));
        }
        memcpy(p.image, viewImage, dataSize);

        redraw();
    }

    // ─────────────────────────────────────────────────────────────────────────
    // draw_pixel() – dispatches to the active method
    // ─────────────────────────────────────────────────────────────────────────
    void Vectorscope::draw_pixel(image::Color4f& color) const noexcept
    {
        TLRENDER_P();

        const int cx = x() + p.diameter / 2;
        const int cy = y() + p.diameter / 2;
        const float R = p.diameter / 2.0f;

        // ── draw position (filled in by each branch) ──────────────────────
        float dx = 0.f, dy = 0.f;

        if (p.method == VectorscopeMethod::ITU601 ||
            p.method == VectorscopeMethod::ITU709)
        {
            
            image::Color4f ycbcr;

            switch(p.method)
            {
            case VectorscopeMethod::ITU709:
                ycbcr = color::rgb::to_ITU709(color);
                break;
            case VectorscopeMethod::ITU601:
            default:
                ycbcr = color::rgb::to_ITU601(color);
                break;
            } 

            // Normalise to [−1, +1]
            float cbNorm = ycbcr.g / p.chromaNorm;   // [−1 … +1]
            float crNorm = ycbcr.b / p.chromaNorm;   // [−1 … +1]

            // 0.85 keeps vivid colours comfortably inside the outer circle
            constexpr float kScale = 0.85f;

            dx =  cbNorm * R * kScale;   // Cb → X
            dy = -crNorm * R * kScale;   // Cr → Y  (negate: FLTK Y grows down)
        }
        else
        {
            // ── HSV (original behaviour, kept for comparison) ─────────────
            image::Color4f hsv = color::rgb::to_hsv(color);

            constexpr float kRedOffsetDegrees = -16.5f;
            constexpr float kFullTurn         = 360.0f;

            float hueDegrees   = hsv.r * kFullTurn;
            float angleDegrees = kRedOffsetDegrees - hueDegrees;
            float angleRad     = math::deg2rad(angleDegrees);

            float satRadius    = hsv.g * 0.85f * R;
            dx =  satRadius * std::sin(angleRad);
            dy = -satRadius * std::cos(angleRad);
        }

        const int posX = cx + static_cast<int>(dx);
        const int posY = cy + static_cast<int>(dy);

        const uint8_t r = static_cast<uint8_t>(
            math::clamp(color.r, 0.f, 1.f) * 255.f);
        const uint8_t g = static_cast<uint8_t>(
            math::clamp(color.g, 0.f, 1.f) * 255.f);
        const uint8_t b = static_cast<uint8_t>(
            math::clamp(color.b, 0.f, 1.f) * 255.f);

        const int pixelSize = p.diameter / 270;
        if (pixelSize <= 1)
        {
            fl_rectf(posX, posY, 1, 1, r, g, b);
        }
        else
        {
            fl_rectf(posX - pixelSize / 2, posY - pixelSize / 2,
                     pixelSize, pixelSize, r, g, b);
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // draw_pixels()
    // ─────────────────────────────────────────────────────────────────────────
    void Vectorscope::draw_pixels() const noexcept
    {
        TLRENDER_P();

        if (!p.box.isValid())
            return;

        int stepX = (p.box.max.x - p.box.min.x) / p.diameter;
        int stepY = (p.box.max.y - p.box.min.y) / p.diameter;
        if (stepX < 1) stepX = 1;
        if (stepY < 1) stepY = 1;

        const int channelCount = image::getChannelCount(p.pixelType);
        const int byteCount    = image::getBitDepth(p.pixelType) / 8;

        const uint32_t W = p.box.w();
        const uint32_t H = p.box.h();

        image::Color4f rgba;

        for (uint32_t Y = 0; Y < H; Y += stepY)
        {
            for (uint32_t X = 0; X < W; X += stepX)
            {
                const size_t offset =
                    (X + Y * W) * channelCount * byteCount;
                rgba = color::fromVoidPtr(p.image + offset, p.pixelType);
                draw_pixel(rgba);
            }
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // draw_grid()
    // ─────────────────────────────────────────────────────────────────────────
    void Vectorscope::draw_grid() noexcept
    {
        TLRENDER_P();

        const int   cx     = x() + p.diameter / 2;
        const int   cy     = y() + p.diameter / 2;
        const float R      = p.diameter / 2.0f;
        const int   boxSz  = std::max(12, p.diameter / 30);

        // ── Outer circle ─────────────────────────────────────────────────────
        fl_color(255, 255, 255);
        fl_arc(x(), y(), p.diameter, p.diameter, 0, 360);

        // ── Centre cross ─────────────────────────────────────────────────────
        fl_color(80, 80, 80);
        fl_line(cx - static_cast<int>(R), cy,
                cx + static_cast<int>(R), cy);
        fl_line(cx, cy - static_cast<int>(R),
                cx, cy + static_cast<int>(R));

        fl_line_style(FL_SOLID, 1);

        if (p.method == VectorscopeMethod::ITU601 ||
            p.method == VectorscopeMethod::ITU709)
        {
            constexpr float kScale    = 0.85f;   // must match draw_pixel
            constexpr float kNorm     = 0.5f;    // max |Cb| or |Cr|
            constexpr float kTarget75 = 0.75f;   // 75 % colour-bar level

            for (int i = 0; i < 6; ++i)
            {
                const ColorTarget& ct = kColorTargets[i];

                // ── 100 % target position ────────────────────────────────
                image::Color4f ycbcr = image::Color4f(ct.r, ct.g, ct.b);

                switch(p.method)
                {
                case VectorscopeMethod::ITU709:
                    ycbcr = color::rgb::to_ITU709(ycbcr);
                    break;
                case VectorscopeMethod::ITU601:
                default:
                    ycbcr = color::rgb::to_ITU601(ycbcr);
                }
                
                float cbN   = ycbcr.g / p.chromaNorm;
                float crN   = ycbcr.b / p.chromaNorm;

                float tx100 = cx + cbN * R * kScale;
                float ty100 = cy - crN * R * kScale;

                // ── 75 % target position ─────────────────────────────────
                // 75 % bars: scale source RGB by 0.75 before converting
                image::Color4f ycbcr75;
                ycbcr75 = image::Color4f(ct.r * kTarget75,
                                         ct.g * kTarget75,
                                         ct.b * kTarget75);
                switch(p.method)
                {
                case VectorscopeMethod::ITU709:
                    ycbcr75 = color::rgb::to_ITU709(ycbcr75);
                    break;
                case VectorscopeMethod::ITU601:
                default:
                    ycbcr75 = color::rgb::to_ITU601(ycbcr75);
                }
                float cb75N = ycbcr75.g / p.chromaNorm;
                float cr75N = ycbcr75.b / p.chromaNorm;

                float tx75  = cx + cb75N * R * kScale;
                float ty75  = cy - cr75N * R * kScale;

                // ── Radial guide line (100 % target → centre) ────────────
                fl_color(60, 60, 60);
                fl_line(cx, cy,
                        static_cast<int>(tx100),
                        static_cast<int>(ty100));

                // ── 75 % box (yellow, inner, broadcast standard) ──────────
                fl_color(255, 200, 0);
                fl_rect(static_cast<int>(tx75) - boxSz / 2,
                        static_cast<int>(ty75) - boxSz / 2,
                        boxSz, boxSz);

                // ── 100 % box (white) ─────────────────────────────────────
                fl_color(200, 200, 200);
                fl_rect(static_cast<int>(tx100) - boxSz / 2,
                        static_cast<int>(ty100) - boxSz / 2,
                        boxSz, boxSz);

                // ── Label (use the colour's own hue for readability) ──────
                int labelX = static_cast<int>(
                    cbN * R * kScale * 1.12f) + cx;
                int labelY = static_cast<int>(
                    -crN * R * kScale * 1.12f) + cy;

                fl_font(FL_HELVETICA_BOLD, 12);
                fl_color(ct.labelR, ct.labelG, ct.labelB);
                fl_draw(ct.name, labelX - 5, labelY + 5);
            }

            // ── Axis labels (Cb / Cr) ─────────────────────────────────────
            fl_font(FL_HELVETICA, 10);
            fl_color(160, 160, 160);
            fl_draw("+Cb",  cx + static_cast<int>(R * 0.88f) - 4,
                            cy + 12);
            fl_draw("-Cb",  cx - static_cast<int>(R * 0.88f) - 14,
                            cy + 12);
            fl_draw("+Cr",  cx - 14,
                            cy - static_cast<int>(R * 0.88f) + 10);
            fl_draw("-Cr",  cx - 14,
                            cy + static_cast<int>(R * 0.88f));
        }
        else
        {
            // ── HSV mode: original evenly-spaced 60° approach ─────────────
            static const char* names[] = { "B", "M", "R", "Y", "G", "C" };
            constexpr float kOffset = 103.5f;

            fl_color(255, 255, 255);
            for (int i = 0; i < 6; ++i)
            {
                float angle = math::deg2rad(kOffset - i * 60.f);
                int x1 = cx + static_cast<int>(std::sin(angle) * (R * 0.05f));
                int y1 = cy - static_cast<int>(std::cos(angle) * (R * 0.05f));
                int x2 = cx + static_cast<int>(std::sin(angle) * (R * 0.92f));
                int y2 = cy - static_cast<int>(std::cos(angle) * (R * 0.92f));
                fl_line(x1, y1, x2, y2);
            }

            fl_color(255, 255, 100);
            for (int i = 0; i < 6; ++i)
            {
                float angle = math::deg2rad(kOffset - i * 60.f);
                int tx = cx + static_cast<int>(std::sin(angle) * (R * 0.85f));
                int ty = cy - static_cast<int>(std::cos(angle) * (R * 0.85f));

                fl_rect(tx - boxSz / 2, ty - boxSz / 2, boxSz, boxSz);

                int labelOffset = static_cast<int>(R * 0.95f);
                int lx = cx + static_cast<int>(std::sin(angle) * labelOffset);
                int ly = cy - static_cast<int>(std::cos(angle) * labelOffset) + 5;

                fl_font(FL_HELVETICA, 12);
                fl_color(255, 255, 0);
                fl_draw(names[i], lx - 6, ly);
            }
        }

        fl_line_style(0);  // reset
    }

} // namespace mrv
