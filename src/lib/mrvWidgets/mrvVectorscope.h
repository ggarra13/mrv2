// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Box.H>

#include <tlCore/Util.h>
#include <tlCore/Color.h>
#include <tlCore/Image.h>

#include "mrvCore/mrvColorAreaInfo.h"

class ViewerUI;

namespace mrv
{
    using namespace tl;

    // VectorscopeMethod selects the YCbCr colour standard used for both pixel
    // plotting and target-box placement.
    //
    //   ITU601  – ITU-R BT.601  (SD broadcast, Kr=0.299  Kb=0.114)
    //   ITU709  – ITU-R BT.709  (HD broadcast, Kr=0.2126 Kb=0.0722)
    //   Rec2020 – ITU-R BT.2020 (UHD / HDR,    Kr=0.2627 Kb=0.0593)
    //
    // HDR note (Rec2020 + HDR swapchain)
    // ─────────────────────────────────
    // In a PQ / ST.2084 swapchain the scene-linear RGB values are normalised
    // to 10 000 nits = 1.0.  Because Cb and Cr scale linearly with absolute
    // RGB, a reference-white red (~203 nits ≈ 0.0203 linear) produces a
    // chroma vector only 2 % as long as a 10 000-nit red, so all dots cluster
    // near the centre.
    //
    // setReferenceWhiteNits() supplies the luminance level (nits) that should
    // land on the target boxes.  The scope divides each pixel's RGB by
    // (referenceWhiteNits / 10 000) before the YCbCr transform so that
    // reference-white primaries map to ±0.5 Cb/Cr — exactly on the targets.
    // BT.2408 recommends 203 nits for HLG and HDR10 programme material.
    // Set to 10 000 (or call setReferenceWhiteNits(10000.f)) to disable
    // normalisation (SDR swapchain behaviour).
    //
    // Dot colours use a per-channel Reinhard tone-map applied to the original
    // (un-normalised) values so that HDR specular highlights remain vividly
    // hued rather than washing out to white.
    enum class VectorscopeMethod { ITU601, ITU709, Rec2020 };

    class Vectorscope : public Fl_Box
    {
    public:
        Vectorscope(int X, int Y, int W, int H, const char* L = 0);
        ~Vectorscope();

        virtual void draw() override;

        void update(const area::Info& info);

        void resize(int X, int Y, int W, int H) override;
        
        void main(ViewerUI* m);
        ViewerUI* main();

        void              setMethod(VectorscopeMethod m);
        VectorscopeMethod method() const;

        // Set the luminance level (nits) that maps to the target-box ring.
        // Only meaningful for Rec2020 on an HDR (PQ) swapchain.
        // Default: 203.f  (BT.2408 reference white for HDR10 / HLG).
        // Pass 10000.f to match SDR-swapchain behaviour (no normalisation).
        void  setReferenceWhiteNits(float nits);
        float referenceWhiteNits() const;
        
    protected:
        void draw_grid() noexcept;
        void draw_pixel(image::Color4f& rgb) const noexcept;
        void draw_pixels() const noexcept;

        TLRENDER_PRIVATE();
    };

} // namespace mrv
