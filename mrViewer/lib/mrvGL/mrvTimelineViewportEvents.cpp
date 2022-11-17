
#include <memory>
#include <cmath>
#include <algorithm>

#include "FL/Fl_Menu_Button.H"
#include "FL/names.h"  // for debugging events

#include "mrvCore/mrvHotkey.h"
#include "mrvCore/mrvSequence.h"
#include "mrvCore/mrvColorSpaces.h"

#include "mrvFl/mrvCallbacks.h"
#include "mrvFl/mrvToolsCallbacks.h"
#include "mrvFl/mrvTimelinePlayer.h"
#include "mrvFl/mrvCompareTool.h"

#include "mrvGL/mrvGLShape.h"
#include "mrvGL/mrvTimelineViewport.h"
#include "mrvGL/mrvTimelineViewportPrivate.h"

#include "mrViewer.h"

#include "mrvCore/mrvUtil.h"
#include "mrvFl/mrvIO.h"


namespace {
    const char* kModule = "view";
}

namespace mrv
{
    std::vector< std::shared_ptr< tl::draw::Annotation > >
    TimelineViewport::_getAnnotationsForFrame(
        const int64_t frame,
        const int previous,
        const int next )
    {
        TLRENDER_P();
        
        std::vector< std::shared_ptr< tl::draw::Annotation > > annotations;
        
        draw::AnnotationList::iterator found = p.annotations.begin();

        while ( found != p.annotations.end() )
        {
            found =
                std::find_if( found, p.annotations.end(),
                              [frame, previous, next]( const auto& a ) {
                                  if ( a->allFrames() ) return true;
                                  int start = a->frame() - previous;
                                  int end   = a->frame() + next;
                                  return ( frame >= start && frame <= end );
                              } );
            
            if ( found != p.annotations.end() )
            {
                annotations.push_back( *found );
                ++found;
            }
        }
        return annotations;
    }

    
    void TimelineViewport::redraw_windows()
    {
        _p->ui->uiView->redraw();
        if ( _p->ui->uiSecondary ) _p->ui->uiSecondary->viewport()->redraw();
    }

    std::shared_ptr< tl::draw::Annotation >
    TimelineViewport::_getAnnotationForFrame( const int64_t frame,
        const bool create )
    {
        TLRENDER_P();

        //! Don't allow annotations while playing
        if ( !p.timelinePlayers.empty() &&
             p.timelinePlayers[0]->playback() != timeline::Playback::Stop )
            return nullptr;
 
        
        const draw::AnnotationList::iterator& found =
            std::find_if( p.annotations.begin(),
                          p.annotations.end(),
                          [frame]( const auto& a ) {
                              return a->frame() == frame;
                          } );
        if ( found == p.annotations.end() )
        {
            if ( create )
            {
                bool all_frames = p.ui->uiAllFrames->value();
                auto annotation =
                    std::make_shared< draw::Annotation >(frame, all_frames);
                p.annotations.push_back( annotation );
                return annotation;
            }
            return nullptr;
        }
        else
        {
            return *found;
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
            redraw();
        }
        else if ( Fl::event_shift() )
        {
            float dx = p.event_x / (float)w() * 360.F;
            p.compareOptions.wipeRotation = dx;
            if ( compareTool ) compareTool->wipeRotation->value( dx );
            redraw();
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
            if ( Fl::event_shift() || p.actionMode == ActionMode::kSelection )
            {
                math::Vector2i pos;
                p.mousePos = _getFocus();
                pos.x = ( p.mousePos.x - p.viewPos.x ) / p.viewZoom;
                pos.y = ( p.mousePos.y - p.viewPos.y ) / p.viewZoom;
                if ( pos.x < 0 ) pos.x = 0;
                if ( pos.y < 0 ) pos.y = 0;
                const auto& renderSize = _getRenderSize();
                if ( pos.x >= renderSize.w ) pos.x = renderSize.w - 1;
                if ( pos.y >= renderSize.h ) pos.y = renderSize.h - 1;
                p.selection.max = pos;
                redraw_windows();
            }
            else
            {
                draw::Point pnt( _getRaster() );
                
                switch( p.actionMode )
                {
                case ActionMode::kScrub:
                    scrub();
                    return;
                case ActionMode::kDraw:
                {
                    int64_t frame = p.ui->uiTimeline->value();
                    auto annotation = _getAnnotationForFrame( frame );
                    if ( ! annotation.get() ) return;
                    
                    auto s = annotation->lastShape();
                    auto shape = dynamic_cast< GLPathShape* >( s.get() );
                    if ( !shape ) return;
                    
                    shape->pts.push_back( pnt );
                    redraw_windows();
                    return;
                }
                case ActionMode::kErase:
                {
                    int64_t frame = p.ui->uiTimeline->value();
                    auto annotation = _getAnnotationForFrame( frame );
                    if ( ! annotation.get() ) return;
                    
                    auto s = annotation->lastShape();
                    auto shape = dynamic_cast< GLErasePathShape* >( s.get() );
                    if ( !shape ) return;
                    
                    shape->pts.push_back( pnt );
                    redraw_windows();
                    return;
                }
                case ActionMode::kArrow:
                {
                    int64_t frame = p.ui->uiTimeline->value();
                    auto annotation = _getAnnotationForFrame( frame );
                    if ( ! annotation.get() ) return;
                    
                    auto s = annotation->lastShape();
                    auto shape = dynamic_cast< GLArrowShape* >( s.get() );
                    if ( !shape ) return;
                    
                    Imath::V2d p1 = shape->pts[0];
                    Imath::V2d lineVector = pnt - p1;
                    double lineLength = lineVector.length();

                    const float theta = 45 * M_PI / 180;
                    const int nWidth = 35;

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
                        
                    redraw_windows();
                    return;
                }
                case ActionMode::kCircle:
                {
                    int64_t frame = p.ui->uiTimeline->value();
                    auto annotation = _getAnnotationForFrame( frame );
                    if ( ! annotation.get() ) return;
                    
                    auto s = annotation->lastShape();
                    auto shape = dynamic_cast< GLCircleShape* >( s.get() );
                    if ( !shape ) return;
                    
                    shape->radius = shape->center.x - pnt.x;
                    redraw_windows();
                    return;
                }
                default:
                    return;
                }
            }
        }
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
                math::Vector2i pos;
                p.mousePos = _getFocus();
                pos.x = ( p.mousePos.x - p.viewPos.x ) / p.viewZoom;
                pos.y = ( p.mousePos.y - p.viewPos.y ) / p.viewZoom;
                if ( pos.x < 0 ) pos.x = 0;
                if ( pos.y < 0 ) pos.y = 0;
                const auto& renderSize = _getRenderSize();
                if ( pos.x > renderSize.w ) pos.x = renderSize.w - 1;
                if ( pos.y > renderSize.h ) pos.y = renderSize.h - 1;
                p.selection.min = pos;
                p.selection.max = p.selection.min;
                        
                redraw_windows();
            }
            else
            {
                uint8_t r, g, b;
                Fl::get_color( p.ui->uiPenColor->color(), r, g, b );
                const imaging::Color4f color(r / 255.F, g / 255.F,
                                             b / 255.F, 1.F);
                const float pen_size = p.ui->uiPenSize->value();
                    
                draw::Point pnt( _getRaster() );

                switch( p.actionMode )
                {
                case ActionMode::kDraw:
                {
                    int64_t frame = p.ui->uiTimeline->value();
                    auto annotation = _getAnnotationForFrame( frame, true );
                    if ( ! annotation ) return;

                    auto shape = std::make_shared< GLPathShape >();
                    shape->pen_size = pen_size;
                    shape->color  = color;
                    shape->pts.push_back( pnt );
                    
                    annotation->push_back( shape );
                    break;
                }
                case ActionMode::kErase:
                {
                    int64_t frame = p.ui->uiTimeline->value();
                    auto annotation = _getAnnotationForFrame( frame, true );
                    if ( ! annotation ) return;

                    auto shape = std::make_shared< GLErasePathShape >();
                    shape->pen_size = pen_size;
                    shape->color  = color;
                    shape->pts.push_back( pnt );
                    
                    annotation->push_back( shape );
                    break;
                }
                case ActionMode::kArrow:
                {
                    int64_t frame = p.ui->uiTimeline->value();
                    auto annotation = _getAnnotationForFrame( frame, true );
                    if ( ! annotation ) return;
                    
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
                    int64_t frame = p.ui->uiTimeline->value();
                    auto annotation = _getAnnotationForFrame( frame, true );
                    if ( ! annotation.get() ) return;
                    
                    auto shape = std::make_shared< GLCircleShape >();
                    shape->pen_size = pen_size;
                    shape->color  = color;
                    shape->center = _getRaster();
                    shape->radius = 0;
                    
                    annotation->push_back( shape );
                    break;
                }
                default:
                    return;
                }
                p.ui->uiUndoDraw->activate();
            }
        }
    }
    
    int TimelineViewport::handle( int event )
    {
        TLRENDER_P();

        p.event_x = Fl::event_x();
        p.event_y = Fl::event_y();

        switch( event )
        {
        case FL_FOCUS:
        case FL_ENTER:
            window()->cursor( FL_CURSOR_CROSS );
            return 1;
            break;
        case FL_LEAVE:
        case FL_UNFOCUS:
            window()->cursor( FL_CURSOR_DEFAULT );
            return 1;
            break;
        case FL_PUSH:
        {
            if (!children()) take_focus();
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

                unsigned rawkey = Fl::event_key();

                p.popupMenu = std::make_unique<Fl_Menu_Button>( 0, 0, 0, 0 );
                p.popupMenu->type( Fl_Menu_Button::POPUP3 );

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
            _updateCoords();
            // If we are drawing or erasing, draw the cursor
            if ( p.actionMode != ActionMode::kScrub &&
                 p.actionMode != ActionMode::kSelection )
            {
                redraw_windows();
            }
            // Don't update the pixel bar here if we are playing the movie
            if ( !p.timelinePlayers.empty() &&
                 p.timelinePlayers[0]->playback() == timeline::Playback::Stop )
                updatePixelBar();
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
            }
            else if ( Fl::event_button1() )
            {
                _handleDragLeftMouseButton();
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
                p.ui->uiGammaInput->value( 1.0 );
                p.ui->uiGainInput->value( 1.0 );
                updateDisplayOptions();
                _refresh();
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
            else if ( kAreaMode.match( rawkey ) )
            {
                setActionMode( ActionMode::kSelection );
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
            else if ( kToggleToolBar.match( rawkey ) )
            {
                toggle_action_tool_bar( nullptr, p.ui );
                save_ui_state( p.ui, p.ui->uiToolsGroup );
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
                p.ui->uiStartButton->value( ! p.ui->uiStartButton->value() );
                p.ui->uiStartButton->do_callback();
                return 1;
            }
            else if ( kSetOutPoint.match( rawkey ) )
            {
                p.ui->uiEndButton->value( ! p.ui->uiEndButton->value() );
                p.ui->uiEndButton->do_callback();
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

        return Fl_SuperClass::handle( event );
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
