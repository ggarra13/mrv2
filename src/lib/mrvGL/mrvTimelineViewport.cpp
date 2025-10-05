// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

// Debug scaling of the window to image size.
// #define DEBUG_SCALING 1

#ifdef _WIN32
#include <winsock2.h>
#endif

#include "mrViewer.h"

#include <tlDevice/IOutput.h>

#include "mrvUI/mrvDesktop.h"

#include "mrvViewport/mrvTimelineViewport.h"

#include "mrvWidgets/mrvDragButton.h"

namespace
{
    inline uint32_t byteSwap32(uint32_t x)
    {
        return ((x >> 24) & 0x000000FF) |
            ((x >> 8)  & 0x0000FF00) |
            ((x << 8)  & 0x00FF0000) |
            ((x << 24) & 0xFF000000);
    }
    
    inline int normalizeAngle0to360(float angle)
    {
        int out = static_cast<int>(std::fmod(angle, 360.0f));
        if (out < 0)
        {
            out += 360;
        }
        return out;
    }

    void stop_playback_while_scrubbing_cb(mrv::opengl::TimelineViewport* view)
    {
        view->stopPlaybackWhileScrubbing();
    }

} // namespace

namespace mrv
{
    namespace opengl
    {        
        using namespace tl;

        TimelineViewport::TimelineViewport(
            int X, int Y, int W, int H, const char* L) :
            GLWindow(X, Y, W, H, L),
            _p(new Private)
        {
            TLRENDER_P();

            p.ui = App::ui;
            _init();
        }

        TimelineViewport::TimelineViewport(int W, int H, const char* L) :
            GLWindow(W, H, L),
            _p(new Private)
        {
            TLRENDER_P();

            p.ui = App::ui;
            _init();
        }
        
        void TimelineViewport::resize(int X, int Y, int W, int H)
        {
            GLWindow::resize(X, Y, W, H);
            if (hasFrameView())
            {
                _frameView();
            }
        }

        void TimelineViewport::_showPixelBar() const noexcept
        {
            TLRENDER_P();

            const int autoHide = p.ui->uiPrefs->uiPrefsAutoHidePixelBar->value();
            const bool hasPixelBar = p.ui->uiPrefs->uiPrefsPixelToolbar->value();
            const bool visiblePixelBar = p.ui->uiPixelBar->visible_r();

            if (!hasPixelBar || visiblePixelBar || !autoHide || p.presentation)
                return;

            toggle_pixel_bar(nullptr, p.ui);
        }

        void TimelineViewport::_hidePixelBar() const noexcept
        {
            TLRENDER_P();

            const int autoHide = p.ui->uiPrefs->uiPrefsAutoHidePixelBar->value();
            const bool visiblePixelBar = p.ui->uiPixelBar->visible_r();

            if (!visiblePixelBar || !autoHide)
                return;

            toggle_pixel_bar(nullptr, p.ui);
        }

        void TimelineViewport::_togglePixelBar() const noexcept
        {
            TLRENDER_P();

            const int autoHide = p.ui->uiPrefs->uiPrefsAutoHidePixelBar->value();
            const bool hasPixelBar = p.ui->uiPrefs->uiPrefsPixelToolbar->value();
            const bool visiblePixelBar = p.ui->uiPixelBar->visible_r();

            if (hasPixelBar && (!autoHide || p.presentation))
                return;

            // This is called *before* the togglePlayback begins, so we need
            // to check for Stop instead of playing.
            const auto playback = p.player->playback();
            if (playback == timeline::Playback::Stop)
            {
                if (visiblePixelBar)
                {
                    toggle_pixel_bar(nullptr, p.ui);
                }
            }
            else
            {
                if (!visiblePixelBar && hasPixelBar)
                {
                    toggle_pixel_bar(nullptr, p.ui);
                }
            }
        }

        float TimelineViewport::_getRotation() const noexcept
        {
            // \@note: FFmpeg's video rotation is counter-clockwise while
            //         our user's rotation are clockwise.  That's why we
            //         substract video rotation here.
            return normalizeAngle0to360(_p->rotation - _p->videoRotation);
        }

        math::Vector2i TimelineViewport::_getFocus(int X, int Y) const noexcept
        {
            TimelineViewport* self = const_cast< TimelineViewport* >(this);
            math::Vector2i pos;
            const float devicePixelRatio = self->pixels_per_unit();
            pos.x = X * devicePixelRatio;
            pos.y = (h() - 1 - Y) * devicePixelRatio;
            return pos;
        }

        math::Vector2f TimelineViewport::_getFocusf(int X, int Y) const noexcept
        {
            TimelineViewport* self = const_cast< TimelineViewport* >(this);
            math::Vector2f pos;
            const float devicePixelRatio = self->pixels_per_unit();
            pos.x = X * devicePixelRatio;
            pos.y = (h() - 1 - Y) * devicePixelRatio;
            return pos;
        }

    } // namespace opengl
    
} // namespace mrv
