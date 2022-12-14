// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <memory>
#include <cmath>
#include <algorithm>

#include <FL/names.h>
#include <FL/Fl_Menu_Button.H>

#include "mrViewer.h"

#include "mrvCore/mrvHotkey.h"
#include "mrvCore/mrvSequence.h"
#include "mrvCore/mrvColorSpaces.h"

#include "mrvFl/mrvCallbacks.h"
#include "mrvFl/mrvToolsCallbacks.h"
#include "mrvFl/mrvTimelinePlayer.h"
#include "mrvFl/mrvCompareTool.h"

#include "mrvWidgets/mrvHorSlider.h"
#include "mrvWidgets/mrvMultilineInput.h"

#include "mrvGL/mrvGLShape.h"
#include "mrvGL/mrvTimelineViewport.h"
#include "mrvGL/mrvTimelineViewportPrivate.h"

#include "mrvApp/mrvSettingsObject.h"
#include "mrvCore/mrvUtil.h"
#include "mrvFl/mrvIO.h"

//#define DEBUG_EVENTS

namespace {
    const char* kModule = "view";
    const int kCrossSize = 10;
}

namespace mrv
{

    void TimelineViewport::redrawWindows()
    {
        _p->ui->uiView->redraw();
        if ( _p->ui->uiSecondary && _p->ui->uiSecondary->window()->visible())
        {
            Viewport* view = _p->ui->uiSecondary->viewport();
            view->redraw();
        }
    }


    void TimelineViewport::_handleCompareOverlay() noexcept
    {
        TLRENDER_P();

        if ( Fl::event_alt() )
        {
            float dx = p.event_x / (float)w();
            p.compareOptions.overlay = dx;
            if ( compareTool )  compareTool->overlay->value( dx );
        }
    }

    void TimelineViewport::_handleCompareWipe() noexcept
    {
        TLRENDER_P();

        if ( Fl::event_alt() )
        {
            float dx = p.event_x / (float)w();
            float dy = p.event_y / (float)h();
            p.compareOptions.wipeCenter.x = dx;
            p.compareOptions.wipeCenter.y = dy;
            if ( compareTool )
            {
                compareTool->wipeX->value( dx );
                compareTool->wipeY->value( dy );
            }
            redrawWindows();
        }
        else if ( Fl::event_shift() )
        {
            float dx = p.event_x / (float)w() * 360.F;
            p.compareOptions.wipeRotation = dx;
            if ( compareTool ) compareTool->wipeRotation->value( dx );
            redrawWindows();
        }
    }

    void TimelineViewport::_handleDragLeftMouseButton() noexcept
    {
        TLRENDER_P();

        if ( p.compareOptions.mode == timeline::CompareMode::Wipe )
        {
            _handleCompareWipe();
        }
        else if ( p.compareOptions.mode ==
                  timeline::CompareMode::Overlay )
        {
            _handleCompareOverlay();
        }
        else
        {
            if ( Fl::event_shift() ||
                 p.actionMode == ActionMode::kSelection )
            {
                p.mousePos = _getFocus();
                math::Vector2i pos = _getRaster();
                if ( pos.x < 0 ) pos.x = 0;
                if ( pos.y < 0 ) pos.y = 0;
                const auto& renderSize = getRenderSize();
                if ( pos.x >= renderSize.w ) pos.x = renderSize.w - 1;
                if ( pos.y >= renderSize.h ) pos.y = renderSize.h - 1;
                p.selection.max = pos;
                redrawWindows();
                return;
            }
            else
            {
                draw::Point pnt( _getRaster() );

                auto player = getTimelinePlayer();
                if ( ! player ) return;

                auto annotation = player->getAnnotation();
                if ( p.actionMode != kScrub && ! annotation ) return;

                std::shared_ptr< draw::Shape > s;
                if ( annotation )
                    s = annotation->lastShape();

                switch( p.actionMode )
                {
                case ActionMode::kScrub:
                    scrub();
                    return;
                case ActionMode::kRectangle:
                {
                    auto shape = dynamic_cast< GLRectangleShape* >( s.get() );
                    if ( !shape ) return;

                    shape->pts[1].x = pnt.x;
                    shape->pts[2].x = pnt.x;
                    shape->pts[2].y = pnt.y;
                    shape->pts[3].y = pnt.y;
                    redrawWindows();
                    return;
                }
                case ActionMode::kDraw:
                {
                    auto shape = dynamic_cast< GLPathShape* >( s.get() );
                    if ( !shape ) return;

                    shape->pts.push_back( pnt );
                    redrawWindows();
                    return;
                }
                case ActionMode::kErase:
                {
                    auto shape = dynamic_cast< GLErasePathShape* >( s.get() );
                    if ( !shape ) return;

                    shape->pts.push_back( pnt );
                    redrawWindows();
                    return;
                }
                case ActionMode::kArrow:
                {
                    auto shape = dynamic_cast< GLArrowShape* >( s.get() );
                    if ( !shape ) return;

                    Imath::V2d p1 = shape->pts[0];
                    Imath::V2d lineVector = pnt - p1;
                    double lineLength = lineVector.length();

                    const auto& renderSize = getRenderSize();
                    const float theta = 45 * M_PI / 180;
                    const int nWidth = 35 * renderSize.w / 1024;

                    double tPointOnLine = nWidth / (2 * (tanf(theta) / 2) *
                                           lineLength);
                    Imath::V2d pointOnLine = pnt +
                                             -tPointOnLine * lineVector;

                    Imath::V2d normalVector( -lineVector.y, lineVector.x );

                    double tNormal = nWidth / (2 * lineLength );
                    Imath::V2d tmp = pointOnLine + tNormal * normalVector;

                    shape->pts[1] = pnt;
                    shape->pts[2] = tmp;
                    shape->pts[3] = pnt;
                    tmp = pointOnLine + -tNormal * normalVector;
                    shape->pts[4] = tmp;

                    redrawWindows();
                    return;
                }
                case ActionMode::kCircle:
                {
                    auto shape = dynamic_cast< GLCircleShape* >( s.get() );
                    if ( !shape ) return;

                    shape->radius = ( shape->center.x - pnt.x ) * pixels_per_unit();
                    redrawWindows();
                    return;
                }
                case ActionMode::kText:
                {
                    MultilineInput* w = getMultilineInput();
                    if ( w )
                    {
                        auto pos = math::Vector2i( p.event_x, p.event_y );
#ifdef USE_OPENGL2
                        w->pos = pos;
#else
                        w->Fl_Widget::position( pos.x, pos.y );
#endif
                        redrawWindows();
                    }
                }
                default:
                    return;
                }
            }
        }
    }

    MultilineInput* TimelineViewport::getMultilineInput() const noexcept
    {
        MultilineInput* w;
        for ( int i = children(); i--; )
        {
            w = dynamic_cast< MultilineInput* >( child(i) );
            if ( !w ) continue;
            return w;
        }
        return nullptr;
    }

    int TimelineViewport::acceptMultilineInput() noexcept
    {
        TLRENDER_P();


        MultilineInput* w = getMultilineInput();
        if ( ! w ) return 0;


        int ret = 0;
        const char* text = w->value();
        if ( text && strlen(text) > 0 )
        {
            auto player = getTimelinePlayer();
            if (! player ) return 0;

            auto annotation = player->getAnnotation();
            if ( ! annotation ) return 0;

            uint8_t r, g, b;
            int fltk_color = p.ui->uiPenColor->color();
            Fl::get_color( (Fl_Color) fltk_color, r, g, b );
            const imaging::Color4f color(r / 255.F, g / 255.F,
                                         b / 255.F, 1.F);
            fl_font( w->textfont(), w->textsize() );

#ifdef USE_OPENGL2
            auto shape = std::make_shared< GL2TextShape >();
#else
            auto shape = std::make_shared< GLTextShape >( p.fontSystem );
#endif

            draw::Point pnt( _getRaster() );
            shape->pts.push_back( pnt ); // needed
            annotation->push_back( shape );
            // Calculate offset from corner due to cross and the bottom of
            // the font.
            math::Vector2i offset( kCrossSize + 2,
                                   kCrossSize + fl_height() - fl_descent() );

            shape->text  = text;
            shape->color = color;

            float pixels_unit = pixels_per_unit();

#ifdef USE_OPENGL2
            shape->font = w->textfont();
            shape->fontSize = w->textsize() / p.viewZoom;
            auto pos = math::Vector2i( w->x() + offset.x,
                                       h() - w->y() - offset.y );

            // This works!
            pos.x = (pos.x - p.viewPos.x / pixels_unit) / p.viewZoom;
            pos.y = (pos.y - p.viewPos.y / pixels_unit) / p.viewZoom;

            shape->pts[0].x = pos.x;
            shape->pts[0].y = pos.y;
#else
            shape->fontFamily = w->fontFamily;
            shape->fontSize = w->textsize() / p.viewZoom * pixels_per_unit();

            shape->pts[0].x += offset.x;
            shape->pts[0].y -= offset.y;
            shape->pts[0].y = -shape->pts[0].y;
#endif
            p.ui->uiUndoDraw->activate();
            ret = 1;
        }
        // Safely delete the winget.  This call removes the
        // widget from the opengl canvas too.
        Fl::delete_widget( w );
        redrawWindows();
        return ret;
    }

    void TimelineViewport::_handlePushLeftMouseButton() noexcept
    {
        TLRENDER_P();

        if ( p.compareOptions.mode == timeline::CompareMode::Wipe )
        {
            _handleCompareWipe();
        }
        else if ( p.compareOptions.mode ==
                  timeline::CompareMode::Overlay )
        {
            _handleCompareOverlay();
        }
        else
        {
            if ( Fl::event_shift() || p.actionMode == ActionMode::kSelection )
            {
                p.mousePos = _getFocus();
                math::Vector2i pos = _getRaster();
                if ( pos.x < 0 ) pos.x = 0;
                if ( pos.y < 0 ) pos.y = 0;
                const auto& renderSize = getRenderSize();
                if ( pos.x >= renderSize.w ) pos.x = renderSize.w - 1;
                if ( pos.y >= renderSize.h ) pos.y = renderSize.h - 1;
                p.selection.min = pos;
                p.selection.max = p.selection.min;
                redrawWindows();
            }
            else
            {
                if( p.actionMode == ActionMode::kScrub )
                    return;

                uint8_t r, g, b;
                SettingsObject* settingsObject = p.ui->app->settingsObject();
                int fltk_color = p.ui->uiPenColor->color();
                Fl::get_color( (Fl_Color)fltk_color, r, g, b );
                const imaging::Color4f color(r / 255.F, g / 255.F,
                                             b / 255.F, 1.F);
                std_any value;
                value = settingsObject->value( kPenSize );
                int pen_size = std_any_cast<int>(value);

                value = settingsObject->value( kTextFont );
                Fl_Font font = std_any_cast<int>(value);

                draw::Point pnt( _getRaster() );

                auto player = getTimelinePlayer();
                if ( ! player ) return;

                auto annotation = player->getAnnotation();
                bool all_frames = false;
                value = p.ui->app->settingsObject()->value( kAllFrames );
                all_frames = std_any_cast<int>( value );
                if ( !annotation )
                {
                    annotation = player->createAnnotation( all_frames );
                    if ( !annotation ) return;
                }
                else
                {
                    if ( annotation->allFrames() != all_frames )
                    {
                        std::string error;
                        if ( all_frames )
                        {
                            error += _("Cannot create an annotation here for all frames.  "
                                       "A current frame annotation already exists.");
                        }
                        else
                        {
                            error += _("Cannot create an annotation here for current frame.  "
                                       "An all frames annotation already exists." );
                        }
                        LOG_ERROR( error );
                        return;
                    }
                }


                switch( p.actionMode )
                {
                case ActionMode::kDraw:
                {
                    auto shape = std::make_shared< GLPathShape >();
                    shape->pen_size = pen_size;
                    shape->color  = color;
                    shape->pts.push_back( pnt );

                    annotation->push_back( shape );
                    break;
                }
                case ActionMode::kErase:
                {
                    auto shape = std::make_shared< GLErasePathShape >();
                    shape->pen_size = pen_size;
                    shape->color  = color;
                    shape->pts.push_back( pnt );

                    annotation->push_back( shape );
                    break;
                }
                case ActionMode::kArrow:
                {
                    auto shape = std::make_shared< GLArrowShape >();
                    shape->pen_size = pen_size;
                    shape->color  = color;
                    shape->pts.push_back( pnt );
                    shape->pts.push_back( pnt );
                    shape->pts.push_back( pnt );
                    shape->pts.push_back( pnt );
                    shape->pts.push_back( pnt );

                    annotation->push_back( shape );
                    break;
                }
                case ActionMode::kCircle:
                {
                    auto shape = std::make_shared< GLCircleShape >();
                    shape->pen_size = pen_size;
                    shape->color  = color;
                    shape->center = _getRaster();
                    shape->radius = 0;

                    annotation->push_back( shape );
                    break;
                }
                case ActionMode::kRectangle:
                {
                    auto shape = std::make_shared< GLRectangleShape >();
                    shape->pen_size = pen_size;
                    shape->color  = color;
                    shape->pts.push_back( pnt );
                    shape->pts.push_back( pnt );
                    shape->pts.push_back( pnt );
                    shape->pts.push_back( pnt );
                    shape->pts.push_back( pnt );

                    annotation->push_back( shape );
                    break;
                }
                case ActionMode::kText:
                {
                    const auto& viewportSize = getViewportSize();
                    float pct = viewportSize.w / 1024.F;
                    auto w = getMultilineInput();

                    value = settingsObject->value( kFontSize );
                    int font_size = std_any_cast<int>(value);
                    double fontSize = font_size * pct * p.viewZoom;
                    math::Vector2i pos( p.event_x, p.event_y );
                    if ( w )
                    {
                        w->take_focus();
                        w->pos = pos;
#ifdef USE_OPENGL2
                        w->textfont( (Fl_Font ) font );
#else
                        w->Fl_Widget::position( pos.x, pos.y );
#endif
                        w->textsize( fontSize );
                        redrawWindows();
                        return;
                    }


                    w = new MultilineInput( pos.x, pos.y, 20, 30 * pct * p.viewZoom );
                    w->take_focus();
#ifdef USE_OPENGL2
                    w->textfont( (Fl_Font ) font );
#endif
                    w->textsize( fontSize );
                    w->textcolor( fltk_color );
                    w->viewPos = p.viewPos;
                    w->viewZoom = p.viewZoom;
                    w->redraw();

                    this->add( w );

                    redrawWindows();
                    return;
                }
                default:
                    return;
                }
                p.ui->uiUndoDraw->activate();
            }
        }
    }

    bool  TimelineViewport::_shouldUpdatePixelBar() const noexcept
    {
        TLRENDER_P();
        // Don't update the pixel bar here if we are playing the movie,
        // as we will update it in the draw() routine.
        bool update = false;
        if ( !p.timelinePlayers.empty() )
        {
            auto player = p.timelinePlayers[0];
            update = ( player->playback() == timeline::Playback::Stop );

            // However, if the movie is a single frame long, we need to
            // update it
            if ( player->inOutRange().duration().to_frames() == 1 )
                update = true;
        }
        return update;
    }

    void TimelineViewport::_updatePixelBar() const noexcept
    {
        TLRENDER_P();

        bool update = _shouldUpdatePixelBar();

        if ( update )
        {
            updatePixelBar();
        }
    }

    void TimelineViewport::_updateCursor() const noexcept
    {
        TLRENDER_P();
        if ( p.actionMode == ActionMode::kScrub ||
             p.actionMode == ActionMode::kSelection )
            cursor( FL_CURSOR_CROSS );
        else if ( p.actionMode == ActionMode::kText )
            cursor( FL_CURSOR_INSERT );
        else
            cursor( FL_CURSOR_NONE );
    }

    int TimelineViewport::handle( int event )
    {
        TLRENDER_P();

#ifdef DEBUG_EVENTS
        bool primary = true;
        if ( p.ui->uiSecondary && p.ui->uiSecondary->viewport() == this )
            primary = false;

        if ( event != FL_MOVE )
        {
            if ( primary ) std::cerr << "PRIMARY ";
            else           std::cerr << "SECONDARY ";
            DBGM0( "EVENT=" << fl_eventnames[event] );
            DBGM0( "FOCUS=" << Fl::focus() );
        }
#endif

        int ret = Fl_SuperClass::handle( event );
        if ( ( event == FL_KEYDOWN ||
               event == FL_KEYUP ) && Fl::focus() != this )
        {
            return ret;
        }

        p.event_x = Fl::event_x();
        p.event_y = Fl::event_y();

        switch( event )
        {
        case FL_FOCUS:
            return 1;
        case FL_ENTER:
        {
            //if (!children()) take_focus();
            // p.ui->uiTimeWindow->uiTimeline->hideThumbnail(); // needed
            _updateCursor();
            updatePixelBar();
            _updateCoords();
            redraw();
            return 1;
            break;
        }
        case FL_LEAVE:
        {
            cursor( FL_CURSOR_DEFAULT );
            constexpr float NaN = std::numeric_limits<float>::quiet_NaN();
            imaging::Color4f rgba( NaN, NaN, NaN, NaN );
            _updatePixelBar( rgba );
            redraw();  // to clear the drawing cursor
            return 1;
            break;
        }
        case FL_UNFOCUS:
            return 1;
            break;
        case FL_PUSH:
        {
            take_focus();
            p.mousePress = _getFocus();
            if ( Fl::event_button1() )
            {
                _handlePushLeftMouseButton();
            }
            else if ( Fl::event_button2() )
            {
                p.viewPosMousePress = p.viewPos;
            }
            else if ( Fl::event_button3() )
            {
                if ( Fl::event_alt() ) {
                    p.viewPosMousePress = p.mousePress;
                    return 1;
                }

                // begin();
                Fl_Group::current(0);
                p.popupMenu = new Fl_Menu_Button( 0, 0, 0, 0 );
                // end();

                p.popupMenu->type( Fl_Menu_Button::POPUP3 );

                p.ui->uiMain->fill_menu( p.popupMenu );
                p.popupMenu->popup();

                p.popupMenu = nullptr;
            }
            return 1;
        }
        case FL_MOVE:
        {
            _updateCoords();
            // If we are drawing or erasing, draw the cursor
            if ( p.actionMode != ActionMode::kScrub &&
                 p.actionMode != ActionMode::kSelection &&
                 p.actionMode != ActionMode::kText )
            {
                cursor( FL_CURSOR_NONE );
                redrawWindows();
            }
            _updatePixelBar();
            return 1;
        }
        case FL_RELEASE:
        {
            if ( p.actionMode == ActionMode::kScrub )
            {
                if ( filesTool )   filesTool->redraw();
                if ( compareTool ) compareTool->redraw();
            }
            _updateCursor();
            return 1;
        }
        case FL_DRAG:
        {
            p.mousePos = _getFocus();
            if ( Fl::event_button2() )
            {
                p.viewPos.x = p.viewPosMousePress.x +
                              (p.mousePos.x - p.mousePress.x);
                p.viewPos.y = p.viewPosMousePress.y +
                              (p.mousePos.y - p.mousePress.y);
                cursor( FL_CURSOR_MOVE );
            }
            else if ( Fl::event_button1() )
            {
                _handleDragLeftMouseButton();
                _updatePixelBar();
            }
            else if ( Fl::event_button3() )
            {
                if ( Fl::event_alt() )
                {
                    float dx = p.mousePos.x - p.mousePress.x;
                    setViewZoom( viewZoom() + dx * viewZoom() / 500.0f,
                                 p.viewPosMousePress );
                    p.mousePress = p.mousePos;
                }
            }
            _updateCoords();
            redrawWindows();
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

            // If we have a text widget, don't swallow key presses
            unsigned rawkey = Fl::event_key();
#if defined(FLTK_USE_WAYLAND)
            if ( rawkey >= 'A' && rawkey <= 'Z' )
            {
                rawkey = tolower( rawkey );
            }
#endif
            if ( kResetChanges.match( rawkey ) )
            {
                p.ui->uiGamma->value( 1.0 );
                p.ui->uiGain->value( 1.0 );
                p.ui->uiGammaInput->value( 1.0 );
                p.ui->uiGainInput->value( 1.0 );
                updateDisplayOptions();
                _refresh();
                return 1;
            }
            else if ( kToggleToolBar.match( rawkey ) )
            {
                toggle_action_tool_bar( nullptr, p.ui );
                save_ui_state( p.ui, p.ui->uiToolsGroup );
                return 1;
            }
            else if ( kShapeFrameStepFwd.match( rawkey ) )
            {
                next_annotation_cb( nullptr, p.ui );
                return 1;
            }
            else if ( kShapeFrameStepBack.match( rawkey ) )
            {
                previous_annotation_cb( nullptr, p.ui );
                return 1;
            }
            else if ( kFitScreen.match( rawkey ) )
            {
                frameView();
                return 1;
            }
            else if ( kResizeMainWindow.match( rawkey ) )
            {
                resizeWindow();
                return 1;
            }
            else if ( kCenterImage.match( rawkey ) )
            {
                centerView();
                return 1;
            }
            else if ( kTextMode.match( rawkey ) )
            {
                setActionMode( ActionMode::kText );
                return 1;
            }
            else if ( kScrubMode.match( rawkey ) )
            {
                setActionMode( ActionMode::kScrub );
                return 1;
            }
            else if ( kDrawMode.match( rawkey ) )
            {
                setActionMode( ActionMode::kDraw );
                return 1;
            }
            else if ( kEraseMode.match( rawkey ) )
            {
                setActionMode( ActionMode::kErase );
                return 1;
            }
            else if ( kArrowMode.match( rawkey ) )
            {
                setActionMode( ActionMode::kArrow );
                return 1;
            }
            else if ( kCircleMode.match( rawkey ) )
            {
                setActionMode( ActionMode::kCircle );
                return 1;
            }
            else if ( kRectangleMode.match( rawkey ) )
            {
                setActionMode( ActionMode::kRectangle );
                return 1;
            }
            else if ( kAreaMode.match( rawkey ) )
            {
                setActionMode( ActionMode::kSelection );
                return 1;
            }
            else if ( kPlayDirection.match( rawkey ) )
            {
                togglePlayback();
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
            else if ( kSafeAreas.match( rawkey ) )
            {
                p.safeAreas ^= 1;
                p.ui->uiMain->fill_menu( p.ui->uiMenuBar );
                redrawWindows();
                return 1;
            }
            else if ( kTogglePresentation.match( rawkey ) )
            {
                setPresentationMode( !p.presentation );
                return 1;
            }
            else if ( kFullScreen.match( rawkey ) )
            {
                setFullScreenMode( !p.fullScreen );
                return 1;
            }
            else if ( kToggleMenuBar.match( rawkey ) )
            {
                toggle_ui_bar( p.ui, p.ui->uiMenuGroup, 25 );
                if ( p.ui->uiMenuGroup->visible() )
                    p.ui->uiMain->fill_menu( p.ui->uiMenuBar );
                save_ui_state( p.ui, p.ui->uiMenuGroup );
                return 1;
            }
            else if ( kToggleTopBar.match( rawkey ) )
            {
                toggle_ui_bar( p.ui, p.ui->uiTopBar, 28 );
                save_ui_state( p.ui, p.ui->uiTopBar );
                return 1;
            }
            else if ( kTogglePixelBar.match( rawkey ) )
            {
                toggle_ui_bar( p.ui, p.ui->uiPixelBar, 30 );
                save_ui_state( p.ui, p.ui->uiPixelBar );
                return 1;
            }
            else if ( kToggleTimeline.match( rawkey ) )
            {
                toggle_ui_bar( p.ui, p.ui->uiBottomBar, 49 );
                save_ui_state( p.ui, p.ui->uiBottomBar );
                return 1;
            }
            else if ( kSetInPoint.match( rawkey ) )
            {
                TimelineClass* c = p.ui->uiTimeWindow;
                c->uiStartButton->value( ! c->uiStartButton->value() );
                c->uiStartButton->do_callback();
                return 1;
            }
            else if ( kSetOutPoint.match( rawkey ) )
            {
                TimelineClass* c = p.ui->uiTimeWindow;
                c->uiEndButton->value( ! c->uiEndButton->value() );
                c->uiEndButton->do_callback();
                return 1;
            }
            else if ( kExposureMore.match( rawkey ) )
            {
                p.ui->uiExposureMore->do_callback();
                return 1;
            }
            else if ( kExposureLess.match( rawkey ) )
            {
                p.ui->uiExposureLess->do_callback();
                return 1;
            }
            else if ( kGammaMore.match( rawkey ) )
            {
                float gamma = p.ui->uiGamma->value();
                p.ui->uiGamma->value( gamma + 0.1f );
                p.ui->uiGamma->do_callback();
                return 1;
            }
            else if ( kGammaLess.match( rawkey ) )
            {
                float gamma = p.ui->uiGamma->value();
                p.ui->uiGamma->value( gamma - 0.1f );
                p.ui->uiGamma->do_callback();
                return 1;
            }
            else if ( kUndoDraw.match( rawkey ) )
            {
                p.ui->uiUndoDraw->do_callback();
                redrawWindows();
                return 1;
            }
            else if ( kRedoDraw.match( rawkey ) )
            {
                p.ui->uiRedoDraw->do_callback();
                redrawWindows();
                return 1;
            }
            else if ( kZoomIn.match( rawkey ) )
            {
                setViewZoom( viewZoom() * 2, _getFocus() );
                return 1;
            }
            else if ( kZoomOut.match( rawkey ) )
            {
                setViewZoom( viewZoom() * 0.5, _getFocus() );
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
            return 0;
            break;
        }
        case FL_DND_ENTER:
        case FL_DND_LEAVE:
        case FL_DND_DRAG:
        case FL_DND_RELEASE:
        {
            return 1;
        }
        case FL_PASTE:
        {
            std::string text;
            if ( Fl::event_text() ) text = Fl::event_text();
            dragAndDrop( text );
            return 1;
        }
        default:
            break;
        }

        return ret;
    }

    void TimelineViewport::dragAndDrop( const std::string& text ) noexcept
    {
        TLRENDER_P();

        stringArray tmpFiles, loadFiles;
        mrv::split_string( tmpFiles, text, "\n" );

        for ( auto file : tmpFiles )
        {
            if ( file.substr(0, 7) == "file://" )
                file = file.substr( 7, file.size() );

            if ( file.empty() ) continue;

            if ( mrv::is_directory( file.c_str() ) )
            {
                stringArray movies, sequences, audios;
                parse_directory( file, movies, sequences, audios );
                loadFiles.insert( loadFiles.end(), movies.begin(),
                                  movies.end() );
                loadFiles.insert( loadFiles.end(), sequences.begin(),
                                  sequences.end() );
                loadFiles.insert( loadFiles.end(), audios.begin(),
                                  audios.end() );
                continue;
            }
            else
            {
                loadFiles.push_back( file );
            }
        }

        open_files_cb( loadFiles, p.ui );
    }




}
