// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

// ─────────────────────────────────────────────────────────────────────────────
// Changes vs. original
// ─────────────────────────────────────────────────────────────────────────────
// • Added VectorscopeMethod enum (ITU601 | ITU709 | Rec2020)
// • Private now carries a `method` field (default: ITU709)
// • setMethod() / method() accessors – declared in mrvVectorscope.h
// • draw_pixel() branches on the active method:
//     ITU601  – ITU-R BT.601  luma + chroma; plots Cb on X, Cr on Y
//     ITU709  – ITU-R BT.709  luma + chroma; plots Cb on X, Cr on Y
//     Rec2020 – ITU-R BT.2020 luma + chroma; plots Cb on X, Cr on Y
//               HDR pixels (values > 1.0) are Reinhard-tone-mapped before
//               their dot colour is drawn, preserving hue information.
// • draw_grid() draws method-appropriate target boxes at the actual Cb/Cr
//   coordinates of the six primary/secondary colours at 100 % saturation.
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
    // Simple per-channel Reinhard tone-map: maps [0, ∞) → [0, 1).
    // Applied to HDR dot colours so they stay vividly hued on screen.
    // ─────────────────────────────────────────────────────────────────────────
    static inline float reinhardTM(float v) noexcept
    {
        return v / (1.f + v);
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Six colour targets (100 % saturation).
    // ─────────────────────────────────────────────────────────────────────────
    struct ColorTarget
    {
        const char* name;
        float       r, g, b;
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

    // ─────────────────────────────────────────────────────────────────────────
    // Dispatch helper: convert an RGB colour to YCbCr using the active method.
    // ─────────────────────────────────────────────────────────────────────────
    static image::Color4f toYCbCr(const image::Color4f& src,
                                  VectorscopeMethod     method) noexcept
    {
        switch (method)
        {
        case VectorscopeMethod::ITU709:
            return color::rgb::to_ITU709(src);
        case VectorscopeMethod::Rec2020:
            return color::rgb::to_Rec2020(src);
        case VectorscopeMethod::ITU601:
        default:
            return color::rgb::to_ITU601(src);
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Compute the maximum chroma vector length across all six targets for the
    // active method, so the outermost colour always lands at exactly
    // kScale * R regardless of standard.
    //
    //   BT.601  – max ≈ 0.500  (all targets nearly equidistant)
    //   BT.709  – max ≈ 0.596  (Green / Magenta reach furthest)
    //   BT.2020 – max ≈ 0.584  (Green / Magenta reach furthest)
    // ─────────────────────────────────────────────────────────────────────────
    static float maxChromaRadius(VectorscopeMethod method)
    {
        float maxR = 0.f;
        for (const auto& ct : kColorTargets)
        {
            const image::Color4f ycbcr =
                toYCbCr(image::Color4f(ct.r, ct.g, ct.b), method);

            // ycbcr.g = Cb, ycbcr.b = Cr
            const float r = std::sqrt(ycbcr.g * ycbcr.g + ycbcr.b * ycbcr.b);
            if (r > maxR) maxR = r;
        }
        return maxR;
    }

    // ─────────────────────────────────────────────────────────────────────────
    // Private
    // ─────────────────────────────────────────────────────────────────────────
    struct Vectorscope::Private
    {
        int               diameter            = 250;
        math::Box2i       box;
        image::PixelType  pixelType;
        uint8_t*          image               = nullptr;
        size_t            dataSize            = 0;
        ViewerUI*         ui                  = nullptr;
        float             chromaNorm          = 0.596f;  // updated by setMethod()
        VectorscopeMethod method              = VectorscopeMethod::ITU709;

        // HDR swapchain normalisation (Rec2020 only).
        // referenceWhiteLinear = referenceWhiteNits / 10000.f
        // Dividing input RGB by this value maps reference-white primaries to
        // ±0.5 Cb/Cr, which is exactly where the 100 % target boxes sit.
        float             referenceWhiteNits   = 203.f;
        float             referenceWhiteLinear = 203.f / 10000.f;
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
        _p->method     = m;
        _p->chromaNorm = maxChromaRadius(m);
        redraw();
    }

    VectorscopeMethod Vectorscope::method() const
    {
        return _p->method;
    }

    // ─────────────────────────────────────────────────────────────────────────
    // HDR reference-white accessors
    // ─────────────────────────────────────────────────────────────────────────
    void Vectorscope::setReferenceWhiteNits(float nits)
    {
        if (nits <= 0.f) nits = 203.f;     // guard against bad input
        _p->referenceWhiteNits   = nits;
        _p->referenceWhiteLinear = nits / 10000.f;
        redraw();
    }

    float Vectorscope::referenceWhiteNits() const
    {
        return _p->referenceWhiteNits;
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
        {
            fl_push_clip(x(), y(), w(), h());
            draw_pixels();
            fl_pop_clip();
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // update()
    // ─────────────────────────────────────────────────────────────────────────
    void Vectorscope::update(const area::Info& info)
    {
        TLRENDER_P();

        MyViewport* view      = p.ui->uiView;
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
    // draw_pixel() – dispatches to the active method.
    //
    // Dot position:
    //   All three methods map Cb → X and Cr → Y (Cr negated because FLTK Y
    //   grows downward).  The chroma vector is normalised by chromaNorm so the
    //   most-saturated target always lands at kScale * R.
    //
    // Dot colour:
    //   SDR methods (ITU601, ITU709) clamp to [0, 1] as before.
    //   Rec2020 applies a per-channel Reinhard tone-map so that HDR pixels
    //   (values > 1.0) keep their hue rather than washing out to white.
    // ─────────────────────────────────────────────────────────────────────────
    void Vectorscope::draw_pixel(image::Color4f& color) const noexcept
    {
        TLRENDER_P();

        const int   cx = x() + p.diameter / 2;
        const int   cy = y() + p.diameter / 2;
        const float R  = p.diameter / 2.0f;

        // ── Chroma position ───────────────────────────────────────────────
        // For Rec2020 on an HDR (PQ) swapchain, scene-linear RGB is normalised
        // to 10 000 nits = 1.0.  Because Cb/Cr scale linearly with absolute
        // RGB values, a reference-white primary (≈ 203 nits ≈ 0.0203 linear)
        // would produce a chroma vector only ~2 % as long as a 10 000-nit
        // primary — i.e. all dots cluster near the centre.
        //
        // We normalise the RGB values by referenceWhiteLinear before the
        // YCbCr transform so that reference-white primaries land exactly on
        // the 100 % target boxes.  SDR methods leave color unchanged.
        image::Color4f chromaInput = color;
            
        if (p.method == VectorscopeMethod::Rec2020 &&
            p.referenceWhiteLinear > 0.f)
        {
            // Convert from PQ to Linear (1.0 = 10,000 nits)
            chromaInput.r = color::inverse_st2084_eotf(chromaInput.r);
            chromaInput.g = color::inverse_st2084_eotf(chromaInput.g);
            chromaInput.b = color::inverse_st2084_eotf(chromaInput.b);
        
            const float inv = 1.f / p.referenceWhiteLinear;
            chromaInput.r *= inv;
            chromaInput.g *= inv;
            chromaInput.b *= inv;
        }
        const image::Color4f ycbcr = toYCbCr(chromaInput, p.method);

        const float cbNorm = ycbcr.g / p.chromaNorm;   // [−1 … +1]
        const float crNorm = ycbcr.b / p.chromaNorm;   // [−1 … +1]

        constexpr float kScale = 0.85f;   // keeps targets comfortably inside

        const int posX = cx + static_cast<int>( cbNorm * R * kScale);
        const int posY = cy + static_cast<int>(-crNorm * R * kScale);  // Y↑

        // ── Dot colour ────────────────────────────────────────────────────
        // For Rec2020 HDR content, luminance values can greatly exceed 1.0.
        // Apply a per-channel Reinhard tone-map so the plotted dot retains its
        // chromatic identity (a bright red HDR pixel still draws red, not
        // clipped white).  For SDR methods the tone-map is equivalent to the
        // original clamp because SDR values stay well below 1.0 in practice,
        // but we keep the explicit branch for clarity.
        float dr = color.r, dg = color.g, db = color.b;
        if (p.method == VectorscopeMethod::Rec2020)
        {
           // Use the already-scaled chromaInput (where 203 nits = 1.0).
           // Apply a 2.2 gamma correction so the dots look vibrant on an SDR UI.
            // We use a simple power function instead of Reinhard to keep 
            // reference white at full brightness.
            dr = std::pow(std::max(0.f, chromaInput.r), 1.f / 2.2f);
            dg = std::pow(std::max(0.f, chromaInput.g), 1.f / 2.2f);
            db = std::pow(std::max(0.f, chromaInput.b), 1.f / 2.2f);
            
            // Clamp for the 8-bit blit, but highlights (> 203 nits) will correctly 
            // bloom toward white.
            dr = std::min(dr, 1.0f);
            dg = std::min(dg, 1.0f);
            db = std::min(db, 1.0f);
        }
        else
        {
            dr = math::clamp(dr, 0.f, 1.f);
            dg = math::clamp(dg, 0.f, 1.f);
            db = math::clamp(db, 0.f, 1.f);
        }

        const uint8_t r8 = static_cast<uint8_t>(dr * 255.f);
        const uint8_t g8 = static_cast<uint8_t>(dg * 255.f);
        const uint8_t b8 = static_cast<uint8_t>(db * 255.f);

        // ── Blit ──────────────────────────────────────────────────────────
        const int pixelSize = p.diameter / 270;
        if (pixelSize <= 1)
        {
            fl_rectf(posX, posY, 1, 1, r8, g8, b8);
        }
        else
        {
            fl_rectf(posX - pixelSize / 2, posY - pixelSize / 2,
                     pixelSize, pixelSize, r8, g8, b8);
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
#ifdef VULKAN_BACKEND
                const size_t offset =
                    (X + Y * W) * channelCount * byteCount;
                rgba = color::fromVoidPtr(p.image + offset, p.pixelType);
#endif

#ifdef OPENGL_BACKEND
                const size_t offset =
                    (X + Y * W) * channelCount * sizeof(float);
                const uint8_t* ptr = p.image + offset;
                rgba = *(reinterpret_cast<const image::Color4f*>(ptr));
                std::swap(rgba.r, rgba.b);  // BGRA → RGBA
#endif
                draw_pixel(rgba);
            }
        }
    }

    // ─────────────────────────────────────────────────────────────────────────
    // draw_grid()
    //
    // Outer ring, centre cross, radial guides, 75 % / 100 % target boxes, and
    // axis labels are drawn for the active colour standard.
    //
    // Rec2020 target-box positions are derived from the BT.2020 matrix, so
    // they will differ slightly from BT.709 — Green / Magenta are a little
    // closer to the centre (chromaNorm ≈ 0.584 vs. ≈ 0.596 for BT.709).
    // ─────────────────────────────────────────────────────────────────────────
    void Vectorscope::draw_grid() noexcept
    {
        TLRENDER_P();

        const int   cx    = x() + p.diameter / 2;
        const int   cy    = y() + p.diameter / 2;
        const float R     = p.diameter / 2.0f;
        const int   boxSz = std::max(12, p.diameter / 30);

        // ── Outer circle ──────────────────────────────────────────────────
        fl_color(255, 255, 255);
        fl_arc(x(), y(), p.diameter, p.diameter, 0, 360);

        // ── Centre cross ──────────────────────────────────────────────────
        fl_color(80, 80, 80);
        fl_line(cx - static_cast<int>(R), cy,
                cx + static_cast<int>(R), cy);
        fl_line(cx, cy - static_cast<int>(R),
                cx, cy + static_cast<int>(R));

        fl_line_style(FL_SOLID, 1);

        constexpr float kScale    = 0.85f;
        constexpr float kTarget75 = 0.75f;

        for (int i = 0; i < 6; ++i)
        {
            const ColorTarget& ct = kColorTargets[i];

            // ── 100 % target ──────────────────────────────────────────────
            const image::Color4f src100(ct.r, ct.g, ct.b);
            const image::Color4f ycbcr100 = toYCbCr(src100, p.method);

            const float cbN = ycbcr100.g / p.chromaNorm;
            const float crN = ycbcr100.b / p.chromaNorm;

            const float tx100 = cx + cbN * R * kScale;
            const float ty100 = cy - crN * R * kScale;

            // ── 75 % target ───────────────────────────────────────────────
            // Scale source RGB by 0.75 before converting (broadcast 75 % bars).
            const image::Color4f src75(ct.r * kTarget75,
                                       ct.g * kTarget75,
                                       ct.b * kTarget75);
            const image::Color4f ycbcr75 = toYCbCr(src75, p.method);

            const float cb75N = ycbcr75.g / p.chromaNorm;
            const float cr75N = ycbcr75.b / p.chromaNorm;

            const float tx75 = cx + cb75N * R * kScale;
            const float ty75 = cy - cr75N * R * kScale;

            // ── Radial guide line ─────────────────────────────────────────
            fl_color(60, 60, 60);
            fl_line(cx, cy,
                    static_cast<int>(tx100),
                    static_cast<int>(ty100));

            // ── 75 % box (yellow) ─────────────────────────────────────────
            fl_color(255, 200, 0);
            fl_rect(static_cast<int>(tx75) - boxSz / 2,
                    static_cast<int>(ty75) - boxSz / 2,
                    boxSz, boxSz);

            // ── 100 % box (white) ─────────────────────────────────────────
            fl_color(200, 200, 200);
            fl_rect(static_cast<int>(tx100) - boxSz / 2,
                    static_cast<int>(ty100) - boxSz / 2,
                    boxSz, boxSz);

            // ── Label ─────────────────────────────────────────────────────
            const int labelX = cx + static_cast<int>(cbN * R * kScale * 1.12f);
            const int labelY = cy + static_cast<int>(-crN * R * kScale * 1.12f);

            fl_font(FL_HELVETICA_BOLD, 12);
            fl_color(ct.labelR, ct.labelG, ct.labelB);
            fl_draw(ct.name, labelX - 5, labelY + 5);
        }

        // ── Axis labels ───────────────────────────────────────────────────
        // Show the standard name next to the Cb axis so the operator always
        // knows which colour space the scope is calibrated to.
        const char* standardTag =
            (p.method == VectorscopeMethod::ITU601)  ? "BT.601"  :
            (p.method == VectorscopeMethod::ITU709)  ? "BT.709"  :
                                                       "BT.2020";

        fl_font(FL_HELVETICA, 10);
        fl_color(160, 160, 160);
        fl_draw("+Cb",       cx + static_cast<int>(R * 0.88f) - 4, cy + 12);
        fl_draw("-Cb",       cx - static_cast<int>(R * 0.88f) - 14, cy + 12);
        fl_draw("+Cr",       cx - 14, cy - static_cast<int>(R * 0.88f) + 10);
        fl_draw("-Cr",       cx - 14, cy + static_cast<int>(R * 0.88f));
        fl_draw(standardTag, cx - static_cast<int>(R * 0.88f),
                             cy + static_cast<int>(R * 0.88f));

        fl_line_style(0);  // reset
    }

} // namespace mrv
