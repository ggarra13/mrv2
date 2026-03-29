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

#include "mrvCore/mrvBackend.h"
#include "mrvCore/mrvColorSpaces.h"
#include "mrvCore/mrvColor.h"

#include <tlCore/Image.h>
#include <tlCore/Math.h>

#include <FL/Enumerations.H>
#include <FL/fl_draw.H>

#ifdef VULKAN_BACKEND
#include <FL/vk_enum_string_helper.h>
#endif

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
        const uint8_t*    image               = nullptr;
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
        Fl_Box(X, Y, W, H, L),
        _p(new Private)
    {
        tooltip(_("Mark an area in the image with SHIFT + the left mouse "
                  "button"));
    }

    Vectorscope::~Vectorscope()
    {
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
        Fl_Box::resize(X, Y, W, H);
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
            p.image = reinterpret_cast<const uint8_t*>(viewImage);
            return;
        }

        const int channelCount = image::getChannelCount(p.pixelType);
        const int byteCount    = image::getBitDepth(p.pixelType) / 8;

        const size_t dataSize =
            info.box.w() * info.box.h() * channelCount * byteCount;

        if (dataSize != p.dataSize)
        {
            p.dataSize = dataSize;
            p.image = reinterpret_cast<const uint8_t*>(viewImage);
        }

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
    //   Rec2020 does not clamp.
    // ─────────────────────────────────────────────────────────────────────────
    void Vectorscope::draw_pixel(image::Color4f& color) const noexcept
    {
        TLRENDER_P();

        const int   cx = x() + p.diameter / 2;
        const int   cy = y() + p.diameter / 2;
        const float R  = p.diameter / 2.0f;

        // ── Chroma position ───────────────────────────────────────────────
        // For Rec2020 on an HDR (PQ) swapchain, we normalize to scene-linear
        // first so that to 10 000 nits = 1.0.
        // Because Cb/Cr scale linearly with absolute
        // RGB values, a reference-white primary (≈ 203 nits ≈ 0.0203 linear)
        // would produce a chroma vector only ~2 % as long as a 10 000-nit
        // primary — i.e. all dots cluster near the centre.
        //
        // We normalise the RGB values by referenceWhiteLinear before the
        // YCbCr transform so that reference-white primaries land exactly on
        // the 100 % target boxes.  SDR methods leave color unchanged.
        image::Color4f chromaInput = color;
        bool hdrSDRConverted = false;   // set when PQ->BT.709 is done in-place
        
#ifdef VULKAN_BACKEND
        Fl_Vk_Context& ctx = p.ui->uiView->getContext();
        if (ctx.colorSpace == VK_COLOR_SPACE_HDR10_ST2084_EXT &&
            p.referenceWhiteLinear > 0.f)
        {
            // ── Step 1: PQ EOTF → linear BT.2020 (0 = 0 nits, 1 = 10 000 nits) ─
            chromaInput.r = color::inverse_st2084_eotf(chromaInput.r);
            chromaInput.g = color::inverse_st2084_eotf(chromaInput.g);
            chromaInput.b = color::inverse_st2084_eotf(chromaInput.b);

            if (p.method == VectorscopeMethod::Rec2020)
            {
                const float inv = 1.f / p.referenceWhiteLinear;
                chromaInput.r *= inv;
                chromaInput.g *= inv;
                chromaInput.b *= inv;
            }
            else // ITU709 or ITU601 — source content is SDR BT.709
            {
                // ── Step 2: BT.2020 linear primaries → BT.709 linear primaries ──
                // M = XYZ_to_709 × 2020_to_XYZ  (both at D65).
                // Each row sums to 1.0 so D65 white is preserved.
                // BT.2020 linear → BT.709 linear  (D65 white, exact chromaticities)
                // Use double for the multiply, then store back to float.
                static constexpr double M[3][3] =
                    {
                        {  1.66049100, -0.58764114, -0.07284987 },
                        { -0.12455047,  1.13288989, -0.00833942 },
                        { -0.01815076, -0.10057889,  1.11872966 }
                    };
                const double r = M[0][0]*chromaInput.r + M[0][1]*chromaInput.g + M[0][2]*chromaInput.b;
                const double g = M[1][0]*chromaInput.r + M[1][1]*chromaInput.g + M[1][2]*chromaInput.b;
                const double b = M[2][0]*chromaInput.r + M[2][1]*chromaInput.g + M[2][2]*chromaInput.b;
                chromaInput.r = static_cast<float>(std::max(0.0, r));
                chromaInput.g = static_cast<float>(std::max(0.0, g));
                chromaInput.b = static_cast<float>(std::max(0.0, b));

                // ── Step 3: Normalise – SDR reference white (default 203 nits) → 1.0 ─
                // libplacebo maps SDR 1.0 → ~203 nits in HDR mode by default.
                // Adjust referenceWhiteNits if you changed pl_color_map_params.peak_detect
                // or are using a different reference (e.g. 100 nits).
                const float inv = 1.f / p.referenceWhiteLinear;
                chromaInput.r = math::clamp(chromaInput.r * inv, 0.f, 1.f);
                chromaInput.g = math::clamp(chromaInput.g * inv, 0.f, 1.f);
                chromaInput.b = math::clamp(chromaInput.b * inv, 0.f, 1.f);

                // ── Step 4: BT.709 OETF – YCbCr matrix is defined for R'G'B', not linear ─
                auto oetf709 = [](float L) -> float
                    {                        
                        return (L < 0.018053968510807f)
                            ? 4.5f * L
                            : 1.0993f * std::pow(L, 0.45f) - 0.0993f;
                    };
                chromaInput.r = oetf709(chromaInput.r);
                chromaInput.g = oetf709(chromaInput.g);
                chromaInput.b = oetf709(chromaInput.b);

                hdrSDRConverted = true; // chromaInput is now BT.709 gamma R'G'B' ∈ [0,1]
            }
        }
        else if (ctx.colorSpace == VK_COLOR_SPACE_DISPLAY_P3_NONLINEAR_EXT)
        {
            // Display P3 NonLinear: P3-D65 primaries + sRGB transfer function.
            // No HDR nit-level normalisation is required (SDR, values in [0,1]).

            // ── Step 1: sRGB EOTF → linear Display P3 ────────────────────────────
            // IEC 61966-2-1 electro-optical transfer function (same as sRGB).
            auto eotfSRGB = [](float V) -> float
                {
                    return (V <= 0.04045f)
                        ? V / 12.92f
                        : std::pow((V + 0.055f) / 1.055f, 2.4f);
                };
            chromaInput.r = eotfSRGB(chromaInput.r);
            chromaInput.g = eotfSRGB(chromaInput.g);
            chromaInput.b = eotfSRGB(chromaInput.b);

            if (p.method == VectorscopeMethod::Rec2020)
            {
                // ── Step 2: Display P3 linear → BT.2020 linear ───────────────────
                // M = XYZ_to_BT2020 × P3_to_XYZ  (both at D65 white point).
                // Derived from the ICC primaries for P3-D65 and BT.2020.
                // White (1,1,1) maps to (1,1,1) – rows sum to 1.0.
                static constexpr double M[3][3] =
                    {
                        {  0.75383303,  0.19859737,  0.04756960 },
                        {  0.04574385,  0.94177722,  0.01247893 },
                        { -0.00121034,  0.01760243,  0.98360900 }
                    };
                const double r = M[0][0]*chromaInput.r + M[0][1]*chromaInput.g + M[0][2]*chromaInput.b;
                const double g = M[1][0]*chromaInput.r + M[1][1]*chromaInput.g + M[1][2]*chromaInput.b;
                const double b = M[2][0]*chromaInput.r + M[2][1]*chromaInput.g + M[2][2]*chromaInput.b;
                chromaInput.r = static_cast<float>(std::max(0.0, r));
                chromaInput.g = static_cast<float>(std::max(0.0, g));
                chromaInput.b = static_cast<float>(std::max(0.0, b));

                // Leave hdrSDRConverted = false.
                // chromaInput is now linear BT.2020, consistent with the HDR10+Rec2020
                // path.  The Rec2020 dot-colour branch (pow 1/2.2) will handle display
                // gamma — it is checked before the hdrSDRConverted branch.
            }
            else // ITU709 or ITU601 — content encoded with P3 primaries, read as BT.709
            {
                // ── Step 2: Display P3 linear → BT.709 linear ────────────────────
                // M = XYZ_to_BT709 × P3_to_XYZ  (both at D65 white point).
                // The R and G rows have a zero in the B column because P3-D65 and
                // BT.709 share the same blue primary (0.150, 0.060).
                static constexpr double M[3][3] =
                    {
                        {  1.22494018, -0.22494018,  0.00000000 },
                        { -0.04205695,  1.04205695,  0.00000000 },
                        { -0.01963755, -0.07863605,  1.09827360 }
                    };
                const double r = M[0][0]*chromaInput.r + M[0][1]*chromaInput.g + M[0][2]*chromaInput.b;
                const double g = M[1][0]*chromaInput.r + M[1][1]*chromaInput.g + M[1][2]*chromaInput.b;
                const double b = M[2][0]*chromaInput.r + M[2][1]*chromaInput.g + M[2][2]*chromaInput.b;
                chromaInput.r = static_cast<float>(std::max(0.0, r));
                chromaInput.g = static_cast<float>(std::max(0.0, g));
                chromaInput.b = static_cast<float>(std::max(0.0, b));

                // ── Step 3: BT.709 OETF → R′G′B′ ────────────────────────────────
                // The ITU-R BT.709 / BT.601 YCbCr matrices are defined for
                // gamma-encoded signals, so we must re-apply the OETF here.
                auto oetf709 = [](float L) -> float
                    {
                        return (L < 0.018053968510807f)
                            ? 4.5f * L
                            : 1.0993f * std::pow(L, 0.45f) - 0.0993f;
                    };
                chromaInput.r = oetf709(chromaInput.r);
                chromaInput.g = oetf709(chromaInput.g);
                chromaInput.b = oetf709(chromaInput.b);

                hdrSDRConverted = true; // chromaInput is now BT.709 R′G′B′ ∈ [0, 1]
            }
        }
#endif
        
        const image::Color4f ycbcr = toYCbCr(chromaInput, p.method);

        const float cbNorm = ycbcr.g / p.chromaNorm;   // [−1 … +1]
        const float crNorm = ycbcr.b / p.chromaNorm;   // [−1 … +1]

        constexpr float kScale = 0.85f;   // keeps targets comfortably inside

        const int posX = cx + static_cast<int>( cbNorm * R * kScale);
        const int posY = cy + static_cast<int>(-crNorm * R * kScale);  // Y↑

        // ── Dot colour ────────────────────────────────────────────────────
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
        else if (hdrSDRConverted)
        {
            // chromaInput already holds BT.709-gamma R'G'B' ∈ [0,1].
            // Use it directly – no additional gamma needed.
            dr = chromaInput.r;
            dg = chromaInput.g;
            db = chromaInput.b;
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
