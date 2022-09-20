// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#include <FL/Fl_Menu_Button.H>
#include <FL/names.h>  // for debugging events


#include <memory>
#include <cmath>

#include <mrvCore/mrvUtil.h>
#include <mrvCore/mrvHotkey.h>
#include <mrvCore/mrvColorSpaces.h>

#include <mrvFl/mrvCallbacks.h>
#include <mrvFl/mrvTimelinePlayer.h>
#include <mrvFl/mrvIO.h>

#include <mrvGL/mrvTimelineViewport.h>
#include <mrvGL/mrvTimelineViewportPrivate.h>

#include <mrViewer.h>

#include <glm/gtc/matrix_transform.hpp>


namespace {
    const char* kModule = "view";
}

namespace mrv
{

    int TimelineViewport::handle( int event )
    {
        TLRENDER_P();

        p.event_x = Fl::event_x();
        p.event_y = Fl::event_y();

        switch( event )
        {
        case FL_FOCUS:
            return 1;
            break;
        case FL_ENTER:
            window()->cursor( FL_CURSOR_CROSS );
            return 1;
            break;
        case FL_LEAVE:
            window()->cursor( FL_CURSOR_DEFAULT );
            return 1;
            break;
        case FL_PUSH:
        {
            if (!children()) take_focus();
            int button = Fl::event_button();
            p.mousePress = _getFocus();
            if ( button == FL_MIDDLE_MOUSE )
            {
                p.viewPosMousePress = p.viewPos;
            }
            else if ( button == FL_RIGHT_MOUSE )
            {
                p.ui->uiMain->fill_menu( p.popupMenu.get() );
                p.popupMenu->popup();
            }
            return 1;
        }
        case FL_RELEASE:
        {
            return 1;
        }
        case FL_MOVE:
        {
            _updatePixelBar();
            _updateCoords();
            return 1;
        }
        case FL_DRAG:
        {
            int button = Fl::event_button();
            p.mousePos = _getFocus();
            if ( button == FL_MIDDLE_MOUSE )
            {
                p.viewPos.x = p.viewPosMousePress.x +
                              (p.mousePos.x - p.mousePress.x);
                p.viewPos.y = p.viewPosMousePress.y +
                              (p.mousePos.y - p.mousePress.y);
            }
            else if ( button == FL_LEFT_MOUSE )
            {
                scrub();
            }
            _updatePixelBar();
            _updateCoords();
            redraw();
            return 1;
        }
        case FL_MOUSEWHEEL:
        {
            float dy = Fl::event_dy();
            int idx = p.ui->uiPrefs->uiPrefsZoomSpeed->value();
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
            setViewZoom( viewZoom() * change, _getFocus() );
            return 1;
        }
        case FL_KEYBOARD:
        {
            unsigned rawkey = Fl::event_key();
            if ( kResetChanges.match( rawkey ) )
            {
                p.ui->uiGamma->value( 1.0 );
                p.ui->uiGain->value( 1.0 );
                updateDisplayOptions();
                _refresh();
                _updatePixelBar();
                return 1;
            }
            else if ( kFitScreen.match( rawkey ) )
            {
                frameView();
                return 1;
            }
            else if ( kCenterImage.match( rawkey ) )
            {
                centerView();
                return 1;
            }
            else if ( kPlayDirection.match( rawkey ) )
            {
                using timeline::Playback;
                Playback playback = p.timelinePlayers[0]->playback();

                for (const auto& i : p.timelinePlayers)
                {
                    i->togglePlayback();
                }
                return 1;
            }
            else if ( kPlayFwd.match( rawkey ) )
            {
                playForwards();
                return 1;
            }
            else if ( kPlayBack.match( rawkey ) )
            {
                playBackwards();
                return 1;
            }
            else if ( kFrameStepFwd.match( rawkey ) )
            {
                frameNext();
                return 1;
            }
            else if ( kFrameStepBack.match( rawkey ) )
            {
                framePrev();
                return 1;
            }
            else if ( kFirstFrame.match( rawkey ) )
            {
                startFrame();
                return 1;
            }
            else if ( kLastFrame.match( rawkey ) )
            {
                endFrame();
                return 1;
            }
            else if ( kToggleToolBar.match( rawkey ) )
            {
                toggle_ui_bar( p.ui, p.ui->uiToolsGroup, 45, 433 );
                save_ui_state( p.ui );
                return 1;
            }
            else if ( kTogglePresentation.match( rawkey ) )
            {

                Fl_Window* w= p.ui->uiMain;

                if ( p.presentation )
                {
                    restore_ui_state( p.ui );
                    if ( w->fullscreen_active() ) w->fullscreen_off();
                    restore_ui_state( p.ui );
                    p.presentation = false;
                }
                else
                {
                    save_ui_state( p.ui );
#ifdef __linux__
                    // Not sure why we need this on linux, but we do
                    hide_ui_state( p.ui );
#endif
                    w->fullscreen();
                    hide_ui_state( p.ui );

                    p.presentation = true;
                }
                return 1;
            }
            else if ( kFullScreen.match( rawkey ) )
            {
                if ( p.fullScreen || p.presentation )
                {
                    p.ui->uiMain->fullscreen_off();
                    restore_ui_state( p.ui );
                    p.fullScreen = p.presentation = false;
                }
                else
                {
                    save_ui_state( p.ui );
                    p.ui->uiMain->fullscreen();
                    p.fullScreen = true;
                }
                return 1;
            }
            else if ( kToggleMenuBar.match( rawkey ) )
            {
                toggle_ui_bar( p.ui, p.ui->uiMenuGroup, 25 );
                if ( p.ui->uiMenuGroup->visible() )
                    p.ui->uiMain->fill_menu( p.ui->uiMenuBar );
                save_ui_state( p.ui );
                return 1;
            }
            else if ( kToggleTopBar.match( rawkey ) )
            {
                toggle_ui_bar( p.ui, p.ui->uiTopBar, 28 );
                save_ui_state( p.ui );
                return 1;
            }
            else if ( kTogglePixelBar.match( rawkey ) )
            {
                toggle_ui_bar( p.ui, p.ui->uiPixelBar, 30 );
                save_ui_state( p.ui );
                return 1;
            }
            else if ( kToggleTimeline.match( rawkey ) )
            {
                toggle_ui_bar( p.ui, p.ui->uiBottomBar, 49 );
                save_ui_state( p.ui );
                return 1;
            }
            else if ( rawkey >= kZoomMin.key && rawkey <= kZoomMax.key )
            {
                if ( rawkey == kZoomMin.key )
                {
                    viewZoom1To1();
                }
                else
                {
                    float z = (float) (rawkey - kZoomMin.key);
                    if ( Fl::event_state( FL_CTRL ) )
                        z = 1.0f / z;
                    setViewZoom( z, _getFocus() );
                }
                return 1;
            }
        }
        default:
            break;
        }

        return Fl_SuperClass::handle( event );
    }
}
