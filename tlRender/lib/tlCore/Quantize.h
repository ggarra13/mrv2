// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlCore/Util.h>

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>

namespace tl
{
    //! The most colors a quantized image has: what a GIF frame can hold.
    constexpr size_t quantizeColorsMax = 256;

    //! A palette: colors as 0xAARRGGBB.
    typedef std::array<uint32_t, quantizeColorsMax> QuantizePalette;

    //! Reduce an RGB image to a palette of at most 256 colors chosen for it,
    //! and each pixel's index into the palette.
    //!
    //! The palette is a median cut of the image's colors: the colors are
    //! divided where they are most numerous, so the palette goes to what the
    //! image is mostly made of. An image with no more colors than the palette
    //! holds keeps them.
    //!
    //! Dithering is ordered rather than diffused: the pattern is the same in
    //! every frame, so a still part of a moving picture does not shimmer, and
    //! it repeats, which the GIF compression makes use of. It is scaled to
    //! how far apart the palette's colors are where the pixel is, so flat
    //! color stays flat and a gradient is spread across its steps.
    //!
    //! \param rgb The pixels, three bytes each, red first.
    //! \param rgbStride The bytes from one row to the next; negative for an
    //! image stored bottom row first, with rgb at the top row.
    //! \param w The width.
    //! \param h The height.
    //! \param indices The palette index of each pixel, one byte each.
    //! \param indicesStride The bytes from one row of indices to the next.
    //! \param palette The colors, the alpha opaque; those past the count are
    //! opaque black.
    //! \param dither Whether to dither.
    //! \return The number of colors in the palette.
    size_t quantize(
        const uint8_t* rgb,
        ptrdiff_t rgbStride,
        int w,
        int h,
        uint8_t* indices,
        ptrdiff_t indicesStride,
        QuantizePalette& palette,
        bool dither = true);

    //! Reduce the frames of a moving picture to palettes, one after another.
    //!
    //! A palette chosen afresh for every frame moves a little with each one,
    //! and where it moves the steps of a gradient move with it: a sky that
    //! is still in the picture wobbles. So the colors of the palette that a
    //! frame uses are kept where they are, and colors the frame has that the
    //! palette lacks are given the places of colors it no longer uses. A
    //! palette is chosen afresh only for another picture: a cut.
    //!
    //! The palette holds 255 colors and its last entry is transparent, which
    //! no pixel is given. A GIF encoder that finds a transparent entry in a
    //! palette that has not changed writes only the pixels that have.
    class Quantizer
    {
        TLRENDER_NON_COPYABLE(Quantizer);

    public:
        Quantizer();

        ~Quantizer();

        //! Reduce the next frame. The arguments are those of quantize().
        //! A frame of another size than the last starts afresh.
        size_t quantize(
            const uint8_t* rgb,
            ptrdiff_t rgbStride,
            int w,
            int h,
            uint8_t* indices,
            ptrdiff_t indicesStride,
            QuantizePalette& palette,
            bool dither = true);

        //! Get how many times the palette has been chosen or changed: once
        //! for a moving picture whose colors stay the same.
        size_t getPaletteCount() const;

    private:
        TLRENDER_PRIVATE();
    };
}
