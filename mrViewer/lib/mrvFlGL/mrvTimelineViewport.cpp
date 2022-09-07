// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <memory>

#include <tlGL/Mesh.h>
#include <tlGL/OffscreenBuffer.h>
#include <tlGL/Render.h>
#include <tlGL/Shader.h>
#include <tlGL/Util.h>

#include <tlCore/Mesh.h>



#include <tlGlad/gl.h>


#include <mrvCore/mrvUtil.h>
#include <mrvCore/mrvHotkey.h>
#include <mrvCore/mrvColorSpaces.h>

#include <mrvFl/mrvTimelinePlayer.h>

#include <mrvFlGL/mrvTimelineViewport.h>
#include <mrvFlGL/mrvTimelineViewportInline.h>

#include <mrViewer.h>

#include <glm/gtc/matrix_transform.hpp>


namespace mrv
{
    using namespace tl;

    TimelineViewport::TimelineViewport(
        int X, int Y, int W, int H, const char* L ) :
        Fl_Gl_Window( X, Y, W, H, L ),
        _p( new Private )
    {
        resizable(this);
    }

    TimelineViewport::~TimelineViewport()
    {
    }

    void TimelineViewport::main( ViewerUI* m )
    {
        std::cerr << "set ui to " << m << std::endl;
        _p->ui = m;
    }

    ViewerUI* TimelineViewport::main() const
    {
        return _p->ui;
    }

    void TimelineViewport::scrub()
    {
        TLRENDER_P();

        if (!p.timelinePlayers.empty())
        {
            const auto t = p.timelinePlayers[0]->currentTime();
            const int X = Fl::event_x() * pixels_per_unit();

            const float scale =
                p.ui->uiPrefs->uiPrefsScrubbingSensitivity->value();

            float dx = ( X - p.mousePress.x );
            dx /= scale;

            p.timelinePlayers[0]->seek(t + otime::RationalTime(dx, t.rate()));
            p.mousePress.x = X;
        }
    }

    void TimelineViewport::resize( int X, int Y, int W, int H )
    {
        Fl_Gl_Window::resize( X, Y, W, H );
        // if ( hasFrameView() )
        // {
        //     frameView();
        // }
    }

    void TimelineViewport::start()
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            i->start();
        }
    }

    void TimelineViewport::framePrev()
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            i->framePrev();
        }
    }

    void TimelineViewport::playBackwards()
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            i->setPlayback( timeline::Playback::Reverse );
        }
    }

    void TimelineViewport::stop()
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            i->setPlayback( timeline::Playback::Stop );
        }
    }

    void TimelineViewport::playForwards()
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            i->setPlayback( timeline::Playback::Forward );
        }
    }

    void TimelineViewport::frameNext()
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            i->frameNext();
        }
    }

    void TimelineViewport::end()
    {
        TLRENDER_P();
        for (const auto& i : p.timelinePlayers)
        {
            i->end();
        }
    }

    int TimelineViewport::handle( int event )
    {
        TLRENDER_P();

        switch( event )
        {
        case FL_FOCUS:
            return 1;
        case FL_ENTER:
            p.mouseInside = true;
            return 1;
        case FL_LEAVE:
            p.mouseInside = false;
            return 1;
        case FL_PUSH:
        {
            if (!children()) take_focus();
            int button = Fl::event_button();
            const float devicePixelRatio = pixels_per_unit();
            p.mousePress.x = Fl::event_x() * devicePixelRatio;
            if ( button == FL_MIDDLE_MOUSE )
            {
                p.mousePress.y = h() * devicePixelRatio - 1 -
                                 Fl::event_y() * devicePixelRatio;
                p.viewPosMousePress = p.viewPos;
            }
            return 1;
        }
        case FL_RELEASE:
        {
            return 1;
        }
        case FL_MOVE:
        {
            if ( !p.ui->uiPixelBar->visible() ) return 0;

            const imaging::Size& r = _getRenderSize();
            const float devicePixelRatio = pixels_per_unit();

            math::Vector2i pos, posz;
            pos.x = Fl::event_x() * devicePixelRatio;
            pos.y = h() * devicePixelRatio - 1 -
                    Fl::event_y() * devicePixelRatio;

            posz.x = ( pos.x - p.viewPos.x ) / p.viewZoom;
            posz.y = ( pos.y - p.viewPos.y ) / p.viewZoom;

            float NaN = std::numeric_limits<float>::quiet_NaN();
            imaging::Color4f rgba( NaN, NaN, NaN, NaN );
            bool inside = true;
            if ( posz.x < 0 || posz.x >= r.w || posz.y < 0 || posz.y >= r.h )
                inside = false;

            if ( inside )
            {
                glPixelStorei(GL_PACK_ALIGNMENT, 1);

                const GLenum format = GL_RGBA;
                const GLenum type = GL_FLOAT;

                glReadPixels( pos.x, pos.y, 1, 1, format, type, &rgba );
            }

            switch( p.ui->uiAColorType->value() )
            {
            case kRGBA_Float:
                p.ui->uiPixelR->value( float_printf( rgba.r ).c_str() );
                p.ui->uiPixelG->value( float_printf( rgba.g ).c_str() );
                p.ui->uiPixelB->value( float_printf( rgba.b ).c_str() );
                p.ui->uiPixelA->value( float_printf( rgba.a ).c_str() );
                break;
            case kRGBA_Hex:
                p.ui->uiPixelR->value( hex_printf( rgba.r ).c_str() );
                p.ui->uiPixelG->value( hex_printf( rgba.g ).c_str() );
                p.ui->uiPixelB->value( hex_printf( rgba.b ).c_str() );
                p.ui->uiPixelA->value( hex_printf( rgba.a ).c_str() );
                break;
            case kRGBA_Decimal:
                p.ui->uiPixelR->value( dec_printf( rgba.r ).c_str() );
                p.ui->uiPixelG->value( dec_printf( rgba.g ).c_str() );
                p.ui->uiPixelB->value( dec_printf( rgba.b ).c_str() );
                p.ui->uiPixelA->value( dec_printf( rgba.a ).c_str() );
                break;
            }

            if ( rgba.r > 1.0f ) rgba.r = 1.0f;
            else if ( rgba.r < 0.0f ) rgba.r = 0.0f;
            if ( rgba.g > 1.0f ) rgba.g = 1.0f;
            else if ( rgba.g < 0.0f ) rgba.g = 0.0f;
            if ( rgba.b > 1.0f ) rgba.b = 1.0f;
            else if ( rgba.b < 0.0f ) rgba.b = 0.0f;

            uint8_t col[3];
            col[0] = uint8_t(rgba.r * 255.f);
            col[1] = uint8_t(rgba.g * 255.f);
            col[2] = uint8_t(rgba.b * 255.f);

            Fl_Color c( fl_rgb_color( col[0], col[1], col[2] ) );

            // @bug: in fltk color lookup? (0 != Fl_BLACK)
            if ( c == 0 )
                p.ui->uiPixelView->color( FL_BLACK );
            else
                p.ui->uiPixelView->color( c );
            p.ui->uiPixelView->redraw();

            imaging::Color4f hsv;

            int cspace = p.ui->uiBColorType->value() + 1;

            switch( cspace )
            {
            case color::kHSV:
                hsv = color::rgb::to_hsv( rgba );
                break;
            case color::kHSL:
                hsv = color::rgb::to_hsl( rgba );
                break;
            case color::kCIE_XYZ:
                hsv = color::rgb::to_xyz( rgba );
                break;
            case color::kCIE_xyY:
                hsv = color::rgb::to_xyY( rgba );
                break;
            case color::kCIE_Lab:
                hsv = color::rgb::to_lab( rgba );
                break;
            case color::kCIE_Luv:
                hsv = color::rgb::to_luv( rgba );
                break;
            case color::kYUV:
                hsv = color::rgb::to_yuv( rgba );
                break;
            case color::kYDbDr:
                hsv = color::rgb::to_YDbDr( rgba );
                break;
            case color::kYIQ:
                hsv = color::rgb::to_yiq( rgba );
                break;
            case color::kITU_601:
                hsv = color::rgb::to_ITU601( rgba );
                break;
            case color::kITU_709:
                hsv = color::rgb::to_ITU709( rgba );
                break;
            case color::kRGB:
            default:
                hsv = rgba;
                break;
            }


            p.ui->uiPixelH->value( float_printf( hsv.r ).c_str() );
            p.ui->uiPixelS->value( float_printf( hsv.g ).c_str() );
            p.ui->uiPixelV->value( float_printf( hsv.b ).c_str() );
            mrv::BrightnessType brightness_type = (mrv::BrightnessType)
                                                  p.ui->uiLType->value();
            hsv.a = calculate_brightness( rgba, brightness_type );
            p.ui->uiPixelL->value( float_printf( hsv.a ).c_str() );
            return 1;
        }
        case FL_DRAG:
        {
            int button = Fl::event_button();
            const float devicePixelRatio = pixels_per_unit();
            if ( button == FL_MIDDLE_MOUSE )
            {
                p.mousePos.x = Fl::event_x() * devicePixelRatio;
                p.mousePos.y = h() * devicePixelRatio - 1 -
                               Fl::event_y() * devicePixelRatio;
                p.viewPos.x = p.viewPosMousePress.x +
                              (p.mousePos.x - p.mousePress.x);
                p.viewPos.y = p.viewPosMousePress.y +
                              (p.mousePos.y - p.mousePress.y);
                p.frameView = false;
            }
            else if ( button == FL_LEFT_MOUSE )
            {
                scrub();
            }
            redraw();
            return 1;
        }
        case FL_MOUSEWHEEL:
        {
            float dy = Fl::event_dy();
            int idx = p.ui->uiPrefs->uiPrefsZoomSpeed->value();
            const float devicePixelRatio = pixels_per_unit();
            p.mousePos.x = Fl::event_x() * devicePixelRatio;
            p.mousePos.y = h() * devicePixelRatio - 1 -
                           Fl::event_y() * devicePixelRatio;
            const float speedValues[] = { 0.1f, 0.25f, 0.5f };
            float speed = speedValues[idx];
            float change = 1.0f;
            if ( dy > 0 )
            {
                change += dy * speed;
                change = 1.0f / change;
            }
            else
            {
                change -= dy * speed;
            }
            setViewZoom( viewZoom() * change, p.mousePos );
            return 1;
        }
        case FL_KEYBOARD:
        {
            unsigned key = Fl::event_key();
            switch( key )
            {
            case 'f':
            {
                _frameView();
                return 1;
                break;
            }
            case ' ':
            {
                using timeline::Playback;
                Playback playback = p.timelinePlayers[0]->playback();

                for (const auto& i : p.timelinePlayers)
                {
                    i->setPlayback(
                        Playback::Stop == playback ?
                        Playback::Forward :
                        Playback::Stop );
                }
                return 1;
                break;
            }
            case FL_Home:
                start();
                return 1;
                break;
            case FL_End:
                end();
                return 1;
                break;
            case FL_Down:
            {
                playForwards();
                return 1;
                break;
            }
            case FL_Up:
            {
                playBackwards();
                return 1;
                break;
            }
            case FL_Right:
            {
                frameNext();
                return 1;
                break;
            }
            case FL_Left:
            {
                framePrev();
                return 1;
                break;
            }
            }
        }
        }
        return Fl_Gl_Window::handle( event );
    }

    void TimelineViewport::setColorConfig(const imaging::ColorConfig& value)
    {
        TLRENDER_P();
        if (value == p.colorConfig)
            return;
        p.colorConfig = value;
        redraw();
    }

    void TimelineViewport::setImageOptions(
        const std::vector<timeline::ImageOptions>& value)
    {
        TLRENDER_P();
        if (value == p.imageOptions)
            return;
        p.imageOptions = value;
        redraw();
    }

    void TimelineViewport::setDisplayOptions(
        const std::vector<timeline::DisplayOptions>& value)
    {
        TLRENDER_P();
        if (value == p.displayOptions)
            return;
        p.displayOptions = value;
        redraw();
    }

    void TimelineViewport::setCompareOptions(const timeline::CompareOptions& value)
    {
        TLRENDER_P();
        if (value == p.compareOptions)
            return;
        p.compareOptions = value;
        redraw();
    }

    void TimelineViewport::setTimelinePlayers(
        const std::vector<TimelinePlayer*>& value)
    {
        TLRENDER_P();
        p.videoData.clear();
        p.timelinePlayers = value;
        for (const auto& i : p.timelinePlayers)
        {
            _p->videoData.push_back(i->video());
        }
        if (p.frameView)
        {
            _frameView();
        }
        redraw();
    }

    TimelinePlayer*
    TimelineViewport::getTimelinePlayer(const int index) const
    {
        return _p->timelinePlayers[index];
    }

    const math::Vector2i& TimelineViewport::viewPos() const
    {
        return _p->viewPos;
    }

    float TimelineViewport::viewZoom() const
    {
        return _p->viewZoom;
    }

    bool TimelineViewport::hasFrameView() const
    {
        return _p->frameView;
    }

    void TimelineViewport::setViewPosAndZoom(const math::Vector2i& pos,
                                             float zoom)
    {
        TLRENDER_P();
        if (pos == p.viewPos && zoom == p.viewZoom)
            return;
        p.viewPos = pos;
        p.viewZoom = zoom;
        redraw();
    }

    void TimelineViewport::setViewZoom(float zoom,
                                       const math::Vector2i& focus)
    {
        TLRENDER_P();
        math::Vector2i pos;
        pos.x = focus.x + (p.viewPos.x - focus.x) * (zoom / p.viewZoom);
        pos.y = focus.y + (p.viewPos.y - focus.y) * (zoom / p.viewZoom);
        setViewPosAndZoom(pos, zoom);
    }

    void TimelineViewport::frameView()
    {
        TLRENDER_P();
        _frameView();
        redraw();
    }

    void TimelineViewport::viewZoom1To1()
    {
        TLRENDER_P();
        setViewZoom(1.F, p.mouseInside ? p.mousePos : _getViewportCenter());
    }

    void TimelineViewport::viewZoomIn()
    {
        TLRENDER_P();
        setViewZoom(p.viewZoom * 2.F, p.mouseInside ? p.mousePos : _getViewportCenter());
    }

    void TimelineViewport::viewZoomOut()
    {
        TLRENDER_P();
        setViewZoom(p.viewZoom / 2.F, p.mouseInside ? p.mousePos : _getViewportCenter());
    }

    void TimelineViewport::videoCallback(const timeline::VideoData& value,
                                         const TimelinePlayer* sender )
    {
        TLRENDER_P();
        const auto i = std::find(p.timelinePlayers.begin(),
                                 p.timelinePlayers.end(), sender);
        if (i != p.timelinePlayers.end())
        {
            const size_t index = i - p.timelinePlayers.begin();
            p.videoData[index] = value;
            p.ui->uiTimeline->redraw();
            redraw();
        }
    }


    imaging::Size TimelineViewport::_getViewportSize() const
    {
        TimelineViewport* t =
            const_cast< TimelineViewport* >( this );
        return imaging::Size( t->pixel_w(), t->pixel_h() );
    }

    std::vector<imaging::Size> TimelineViewport::_getTimelineSizes() const
    {
        TLRENDER_P();
        std::vector<imaging::Size> sizes;
        for (const auto& i : p.timelinePlayers)
        {
            const auto& ioInfo = i->ioInfo();
            if (!ioInfo.video.empty())
            {
                sizes.push_back(ioInfo.video[0].size);
            }
        }
        return sizes;
    }

    imaging::Size TimelineViewport::_getRenderSize() const
    {
        return timeline::getRenderSize(_p->compareOptions.mode,
                                       _getTimelineSizes());
    }

    math::Vector2i TimelineViewport::_getViewportCenter() const
    {
        const auto viewportSize = _getViewportSize();
        return math::Vector2i(viewportSize.w / 2, viewportSize.h / 2);
    }

    void TimelineViewport::_frameView()
    {
        TLRENDER_P();
        const auto viewportSize = _getViewportSize();
        const auto renderSize = _getRenderSize();
        float zoom = viewportSize.w / static_cast<float>(renderSize.w);
        if (zoom * renderSize.h > viewportSize.h)
        {
            zoom = viewportSize.h / static_cast<float>(renderSize.h);
        }
        const math::Vector2i c(renderSize.w / 2, renderSize.h / 2);
        p.viewPos.x = viewportSize.w / 2.F - c.x * zoom;
        p.viewPos.y = viewportSize.h / 2.F - c.y * zoom;
        p.viewZoom = zoom;
    }

    void TimelineViewport::resizeWindow()
    {
        TLRENDER_P();
        auto renderSize = _getRenderSize();
        if ( !renderSize.isValid() ) return;

        int W = renderSize.w;
        int H = renderSize.h;


        Fl_Double_Window* mw = p.ui->uiMain;
        int screen = mw->screen_num();
        float scale = Fl::screen_scale( screen );
        int minx, miny, maxW, maxH, posX, posY;
        Fl::screen_work_area( minx, miny, maxW, maxH, screen );

        PreferencesUI* uiPrefs = p.ui->uiPrefs;
        if ( uiPrefs->uiWindowFixedPosition->value() )
        {
            posX = (int) uiPrefs->uiWindowXPosition->value();
            posY = (int) uiPrefs->uiWindowYPosition->value();
        }
        else
        {
            posX = minx;
            posY = miny;
        }



        int decW = mw->decorated_w();
        int decH = mw->decorated_h();

        int dW = decW - mw->w();
        int dH = decH - mw->h();

        maxW -= dW;
        maxH -= dH;
        posX += dW / 2;
#ifdef _WIN32
        posY += dH - dW / 2;
#else
        posY += dH;
#endif

        int maxX = posX + maxW;
        int maxY = posY + maxH;

        bool fit = false;

        if ( maxX > maxW ) {
            fit = true;
            W = maxW;
        }
        if ( maxY > maxH ) {
            fit = true;
            H = maxH;
        }


        if ( uiPrefs && uiPrefs->uiWindowFixedSize->value() )
        {
            W = (int) uiPrefs->uiWindowXSize->value();
            H =  (int) uiPrefs->uiWindowYSize->value();
        }

        maxW = (int) (maxW / scale);
        if ( W < 690 )  W = 690;
        else if ( W > maxW )
        {
            W = maxW;
        }

        maxH =  (int) (maxH / scale);
        if ( H < 565 )  H =  565;
        else if ( H > maxH )
        {
            H = maxH;
        }

        int X = posX;
        int Y = posY;
        std::cerr << X << ", " << Y << std::endl;
        std::cerr << W << "x" << H << std::endl;
        mw->resize( X, Y, W, H );
        Fl::flush();

        std::cerr << "==============================" << std::endl;
        _frameView();
        std::cerr << "******************************" << std::endl;
    }

}
