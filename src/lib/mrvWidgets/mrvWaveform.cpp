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
    struct Waveform::Private
    {
        ViewerUI*         ui                  = nullptr;
        math::Box2i       box;
        image::PixelType  pixelType;
        const uint8_t*    image               = nullptr;
        size_t            dataSize            = 0;
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
    }

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
            p.image = reinterpret_cast<const uint8_t*>(viewImage);
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
            p.image = reinterpret_cast<const uint8_t*>(viewImage);
        }
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

        const int H = h();
        
        X = x() + pct * w();
        int Y = y() + H - H * calculate_brightness(rgba, kAsLuminance);

        const float dr = math::clamp(rgba.r, 0.F, 1.F);
        const float dg = math::clamp(rgba.g, 0.F, 1.F);
        const float db = math::clamp(rgba.b, 0.F, 1.F);
        
        const uint8_t r8 = static_cast<uint8_t>(dr * 255.f);
        const uint8_t g8 = static_cast<uint8_t>(dg * 255.f);
        const uint8_t b8 = static_cast<uint8_t>(db * 255.f);
        
        fl_rectf(X, Y, 1, 1, r8, g8, b8);
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
