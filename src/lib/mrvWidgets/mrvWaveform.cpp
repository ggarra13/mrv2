// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrViewer.h"

#include "mrvWidgets/mrvWaveform.h"

#include "mrvCore/mrvColor.h"
#include "mrvCore/mrvColorSpaces.h"

#include <FL/fl_draw.H>


namespace mrv
{
    // ─────────────────────────────────────────────────────────────────────────
    // Internal helpers
    // ─────────────────────────────────────────────────────────────────────────
    
    // Map a raw linear luminance value to a normalised [0, 1] waveform position
    // (0 = bottom, 1 = top) according to the current SDR/HDR mode.
    static float luminanceToNorm(float luma,
                                 bool  hdrMode,
                                 float hdrMaxValue,
                                 bool  hdrLogScale)
    {
        if (!hdrMode)
        {
            // Original SDR path – clamp to [0, 1].
            return math::clamp(luma, 0.f, 1.f);
        }
 
        if (hdrLogScale)
        {
            // Map [0, hdrMaxValue] onto [0, 1] in log2 space.
            // We add 1 before taking the log so that 0 maps cleanly to 0.
            const float logMax = std::log2(hdrMaxValue + 1.f);
            const float norm   = std::log2(std::max(luma, 0.f) + 1.f) / logMax;
            return math::clamp(norm, 0.f, 1.f);
        }
        else
        {
            // Linear HDR: simply divide by the ceiling value.
            return math::clamp(luma / hdrMaxValue, 0.f, 1.f);
        }
    }
    
    struct Waveform::Private
    {
        ViewerUI*         ui                  = nullptr;
        math::Box2i       box;
        image::PixelType  pixelType;
        uint8_t*          image               = nullptr;
        size_t            dataSize            = 0;
        
        // ── HDR display options ──────────────────────────────────────────────
        // When hdrMode is false the waveform behaves exactly as before:
        // luminance is assumed to live in [0, 1] and is mapped linearly to
        // widget height.
        //
        // When hdrMode is true values above 1.0 are legal.  The full
        // luminance range [0, hdrMaxValue] is mapped to widget height using
        // either a linear or a log2 scale (controlled by hdrLogScale).
        //
        // A dashed reference line is always drawn at the SDR white point
        // (luminance == 1.0) so the operator can see where broadcast-legal
        // levels end.
        bool              hdrMode             = true;
        float             hdrMaxValue         = 12.f; // e.g. 12 stops above 0
        bool              hdrLogScale         = true; // log2 feels more natural
                                                      // for HDR stop-based work
    };
    
    // ─────────────────────────────────────────────────────────────────────────
    // UI plumbing
    // ─────────────────────────────────────────────────────────────────────────
    void Waveform::main(ViewerUI* m) { _p->ui = m; }
    
    Waveform::Waveform( int X, int Y, int W, int H, const char* L) :
        Fl_Box( X, Y, W, Y, L ),
        _p(new Private)
    {
        color( FL_BLACK );
        tooltip( _("Mark an area in the image with SHIFT + the left mouse button") );
    }

    Waveform::~Waveform()
    {
        TLRENDER_P();
        free(p.image);
    }

    // ── HDR accessors ────────────────────────────────────────────────────────
    void Waveform::setHDRMode(bool enabled, float maxValue, bool logScale)
    {
        TLRENDER_P();
        p.hdrMode     = enabled;
        p.hdrMaxValue = (maxValue > 1.f) ? maxValue : 12.f;
        p.hdrLogScale = logScale;
        redraw();
    }
 
    bool  Waveform::isHDRMode()     const { return _p->hdrMode;     }
    float Waveform::hdrMaxValue()   const { return _p->hdrMaxValue; }
    bool  Waveform::isHDRLogScale() const { return _p->hdrLogScale; }
    
    // ─────────────────────────────────────────────────────────────────────────
    // update()
    // ─────────────────────────────────────────────────────────────────────────
    void Waveform::update(const area::Info& info)
    {
        TLRENDER_P();

        MyViewport* view      = p.ui->uiView;
        const void* viewImage = view->image();
        p.box       = info.box;
        p.pixelType = info.pixelType;

        if (!viewImage)
        {
            free(p.image);
            p.image = nullptr;
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
            p.image = reinterpret_cast<uint8_t*>(malloc(dataSize));
        }
        memcpy(p.image, viewImage, dataSize);
        
        redraw();
    }
    
    void Waveform::resize(int X, int Y, int W, int H)
    {
        if (W < 250) W = 250;
        H = W / 2;

        Fl_Box::resize(X, Y, W, H);
    }
    
    void Waveform::draw_grid()
    {        
        TLRENDER_P();
 
        fl_line_style(FL_DASH, 1);
 
        const int wx = x(), wy = y(), ww = w(), wh = h();
 
        auto drawHLine = [&](float normY, uint8_t r, uint8_t g, uint8_t b)
        {
            // normY is in [0,1], 0 = bottom, 1 = top.
            const int py = wy + wh - static_cast<int>(normY * wh);
            fl_color(fl_rgb_color(r, g, b));
            fl_line(wx, py, wx + ww, py);
        };
 
        if (!p.hdrMode)
        {
            // SDR: draw 0 % (black) and 100 % (white) reference lines.
            drawHLine(0.f,  80,  80,  80);   // black point  – dark grey
            drawHLine(1.f, 200, 200, 200);   // white point  – light grey
        }
        else
        {
            // HDR: always draw the SDR white-point (luma == 1.0) prominently.
            const float sdrNorm = luminanceToNorm(
                1.f, p.hdrMode, p.hdrMaxValue, p.hdrLogScale);
            drawHLine(sdrNorm, 220, 180,  50);  // amber – SDR legal limit
 
            // Draw a line for every full stop above 1.0 (i.e. 2, 4, 8, …)
            // as long as it falls within hdrMaxValue.
            for (float stops = 2.f; stops <= p.hdrMaxValue; stops *= 2.f)
            {
                const float norm = luminanceToNorm(
                    stops, p.hdrMode, p.hdrMaxValue, p.hdrLogScale);
                // Dimmer lines for the upper stops.
                drawHLine(norm, 100, 100, 100);
            }
 
            // HDR ceiling.
            drawHLine(1.f, 200, 200, 200);
        }
 
        fl_line_style(0);   // reset to solid
    }

    void Waveform::draw()
    {
        TLRENDER_P();
        
        fl_rectf(x(), y(), w(), h(), 0, 0, 0);
        
        draw_grid();
        
        if (p.image)
            draw_pixels();
    }

    void Waveform::draw_pixel(int X, const image::Color4f& rgba)
    {
        TLRENDER_P();
        
        const uint32_t W = p.box.w();
        const float pct = static_cast<float>(X) / W;

        const int wh = h();
        
        X = x() + static_cast<int>(pct * w());

        // ── Waveform Y position ──────────────────────────────────────────────
        const float luma = calculate_brightness(rgba, kAsLuminance);
        const float norm = luminanceToNorm(
            luma, p.hdrMode, p.hdrMaxValue, p.hdrLogScale);
 
        // norm == 0 → bottom of widget, norm == 1 → top
        const int Y = y() + wh - static_cast<int>(norm * wh);

        // ── Display colour ───────────────────────────────────────────────────
        float dr = rgba.r;
        float dg = rgba.g;
        float db = rgba.b;

        if (p.hdrMode)
        {
            // Blend super-white channels toward white so the operator can
            // clearly see which columns contain HDR data.  Values in [0,1]
            // are unaffected; values above 1 move toward (1,1,1).
            const float blend = math::clamp(luma - 1.f, 0.f, 1.f);
            dr = math::clamp(dr, 0.f, 1.f);
            dg = math::clamp(dg, 0.f, 1.f);
            db = math::clamp(db, 0.f, 1.f);
            dr = dr + (1.f - dr) * blend;
            dg = dg + (1.f - dg) * blend;
            db = db + (1.f - db) * blend;
        }
        else
        {
            // Original SDR path – hard clamp.
            dr = math::clamp(dr, 0.f, 1.F);
            dg = math::clamp(dg, 0.f, 1.F);
            db = math::clamp(db, 0.f, 1.F);
        }
                
        const uint8_t r8 = static_cast<uint8_t>(dr * 255.f);
        const uint8_t g8 = static_cast<uint8_t>(dg * 255.f);
        const uint8_t b8 = static_cast<uint8_t>(db * 255.f);

        fl_color(r8, g8, b8);
        fl_point(X, Y);
    }

    void Waveform::draw_pixels()
    {
        TLRENDER_P();
        
        if (!p.box.isValid())
            return;
        
        int stepX = (p.box.max.x - p.box.min.x) / w();
        int stepY = (p.box.max.y - p.box.min.y) / w();
        if (stepX < 1) stepX = 1;
        if (stepY < 1) stepY = 1;
        
        const int channelCount = image::getChannelCount(p.pixelType);
        const int byteCount    = image::getBitDepth(p.pixelType) / 8;

        const uint32_t W = p.box.w();
        const uint32_t H = p.box.h();

        image::Color4f rgba;

        for (uint32_t X = 0; X < W; X += stepX)
        {
            for (uint32_t Y = 0; Y < H; Y += stepY)
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
                draw_pixel(X, rgba);
            }
        }
    }

}
