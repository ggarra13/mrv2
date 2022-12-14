// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <string>
#include <vector>
#include <map>

#include <FL/Fl_Button.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_RGB_Image.H>



#include "mrvWidgets/mrvHorSlider.h"
#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvClipButton.h"
#include "mrvWidgets/mrvButton.h"

#include "mrvFl/mrvCompareTool.h"
#include "mrvFl/mrvToolsCallbacks.h"

#include "mrvGL/mrvThumbnailCreator.h"

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/App.h"

#include "mrViewer.h"


namespace mrv
{

    typedef std::map< ClipButton*, int64_t > WidgetIds;
    typedef std::map< ClipButton*, size_t >  WidgetIndices;

    struct CompareTool::Private
    {
        std::weak_ptr<system::Context> context;
        mrv::ThumbnailCreator*    thumbnailCreator;
        std::map< std::string, ClipButton* >    map;
        WidgetIds                              ids;
        WidgetIndices                      indices;
        std::vector< Fl_Button* >          buttons;
    };

    struct ThumbnailData
    {
        ClipButton* widget;
    };



    void compareThumbnail_cb( const int64_t id,
                              const std::vector< std::pair<otime::RationalTime,
                              Fl_RGB_Image*> >& thumbnails,
                              void* opaque )
    {
        ThumbnailData* data = static_cast< ThumbnailData* >( opaque );
        ClipButton* w = data->widget;
        if ( compareTool )
            compareTool->compareThumbnail( id, thumbnails, w );
        delete data;
    }

    void CompareTool::compareThumbnail( const int64_t id,
                                        const std::vector< std::pair<otime::RationalTime,
                                        Fl_RGB_Image*> >& thumbnails,
                                        ClipButton* w)
    {
        WidgetIds::const_iterator it = _r->ids.find( w );
        if ( it == _r->ids.end() ) return;

        if ( it->second == id )
        {
            for (const auto& i : thumbnails)
            {
                Fl_Image* img = w->image();
                w->image( i.second );
                delete img;
                w->redraw();
            }
        }
        else
        {
            for (const auto& i : thumbnails)
            {
                delete i.second;
            }
        }
    }

    CompareTool::CompareTool( ViewerUI* ui ) :
        _r( new Private ),
        ToolWidget( ui )
    {
        _r->context = ui->app->getContext();

        add_group( _("Compare") );

        Fl_SVG_Image* svg = load_svg( "Compare.svg" );
        g->image( svg );


        g->callback( []( Fl_Widget* w, void* d ) {
            ViewerUI* ui = static_cast< ViewerUI* >( d );
            delete compareTool; compareTool = nullptr;
            ui->uiMain->fill_menu( ui->uiMenuBar );
        }, ui );

    }

    CompareTool::~CompareTool()
    {
        cancel_thumbnails();
        clear_controls();
    }

    void CompareTool::clear_controls()
    {
        for (const auto& i : _r->map )
        {
            ClipButton* b = i.second;
            delete b->image(); b->image(nullptr);
            delete b;
        }

        // Clear buttons' SVG images
        for (const auto& b : _r->buttons )
        {
            delete b->image(); b->image(nullptr);
        }

        _r->buttons.clear();
        _r->map.clear();
        _r->indices.clear();
    }

  void CompareTool::cancel_thumbnails()
  {
    for ( const auto& it : _r->ids )
      {
        _r->thumbnailCreator->cancelRequests( it.second );
      }

    _r->ids.clear();
  }

    void CompareTool::add_controls()
    {
        TLRENDER_P();

        _r->thumbnailCreator =
            p.ui->uiTimeWindow->uiTimeline->thumbnailCreator();

        g->clear();
        g->begin();
        const auto& model = p.ui->app->filesModel();
        const auto& files = model->observeFiles();
        size_t numFiles = files->getSize();
        auto Bindices = model->observeBIndexes()->get();

        auto player = p.ui->uiView->getTimelinePlayer();

        otio::RationalTime time = otio::RationalTime(0.0,1.0);
        if ( player ) time = player->currentTime();

        imaging::Size size( 128, 64 );

        for ( size_t i = 0; i < numFiles; ++i )
        {
            const auto& media = files->getItem( i );
            const auto& path = media->path;


            const std::string& dir = path.getDirectory();
            const std::string file = path.getBaseName() + path.getNumber() +
                                     path.getExtension();
            const std::string fullfile = dir + file;

            auto bW = new Widget<ClipButton>( g->x(), g->y()+20+i*68, g->w(), 68 );
            ClipButton* b = bW;
            _r->indices.insert( std::make_pair( b, i ) );
            for( auto Bindex : Bindices )
            {
                if ( Bindex == i )
                {
                    b->value( 1 );
                    break;
                }
            }
            bW->callback( [=]( auto b ) {
                    WidgetIndices::const_iterator it = _r->indices.find( b );
                    if ( it == _r->indices.end() ) return;
                    int index = (*it).second;
                    const auto& model = p.ui->app->filesModel();
                    const auto bIndexes = model->observeBIndexes()->get();
                    const auto i = std::find(bIndexes.begin(), bIndexes.end(),
                                             index);
                    model->setB( index, i == bIndexes.end() );
                    redraw();
                } );

            _r->map.insert( std::make_pair( fullfile, b ) );

            std::string text = dir + "\n" + file;
            b->copy_label( text.c_str() );

            if ( auto context = _r->context.lock() )
            {
                ThumbnailData* data = new ThumbnailData;
                data->widget  = b;

                WidgetIds::const_iterator it = _r->ids.find( b );
                if ( it != _r->ids.end() )
                {
                    _r->thumbnailCreator->cancelRequests( it->second );
                    _r->ids.erase(it);
                }

                auto timeline = timeline::Timeline::create(path.get(), context);
                auto timeRange = timeline->getTimeRange();

                auto startTime = timeRange.start_time();
                auto endTime   = timeRange.end_time_inclusive();

                if ( time < startTime ) time = startTime;
                else if ( time > endTime ) time = endTime;

                _r->thumbnailCreator->initThread();
                try
                {
                    int64_t id =
                        _r->thumbnailCreator->request( fullfile, time, size,
                                                       compareThumbnail_cb,
                                                       (void*)data );
                    _r->ids[b] = id;
                }
                catch (const std::exception&)
                {
                }
            }

        }

        Fl_Pack* bg = new Fl_Pack( g->x(), g->y()+20+numFiles*64, g->w(), 30 );
        bg->type( Fl_Pack::HORIZONTAL );
        bg->begin();

        Fl_Button* b;
        auto bW = new Widget< Button >( g->x(), 90, 30, 30 );
        b = bW;
        Fl_SVG_Image* svg = load_svg( "CompareA.svg" );
        b->image( svg );
        _r->buttons.push_back( b );
        b->tooltip( _("Compare A") );
        bW->callback( [=]( auto w ) {
            auto o = model->observeCompareOptions()->get();
            o.mode = timeline::CompareMode::A;
            model->setCompareOptions( o );
            p.ui->uiView->setCompareOptions( o );
            p.ui->uiView->frameView();
            p.ui->uiView->redraw();
        } );

        bW = new Widget< Button >( g->x(), 90, 30, 30 );
        b = bW;
        svg = load_svg( "CompareB.svg" );
        b->image( svg );
        _r->buttons.push_back( b );
        b->tooltip( _("Compare B") );

        bW->callback( [=]( auto w ) {
            auto o = model->observeCompareOptions()->get();
            o.mode = timeline::CompareMode::B;
            model->setCompareOptions( o );
            p.ui->uiView->setCompareOptions( o );
            p.ui->uiView->frameView();
            p.ui->uiView->redraw();
        } );

        bW = new Widget< Button >( g->x(), 90, 30, 30 );
        b = bW;
        svg = load_svg( "CompareWipe.svg" );
        b->image( svg );
        _r->buttons.push_back( b );
        b->tooltip( _("Wipe between the A and B files\n\n"
                      "Use the Alt key + left mouse button to move the wipe") );

        bW->callback( [=]( auto w ) {
            auto o = model->observeCompareOptions()->get();
            o.mode = timeline::CompareMode::Wipe;
            model->setCompareOptions( o );
            p.ui->uiView->setCompareOptions( o );
            p.ui->uiView->frameView();
            p.ui->uiView->redraw();
        } );


        bW = new Widget< Button >( g->x(), 90, 30, 30 );
        b = bW;
        svg = load_svg( "CompareOverlay.svg" );
        b->image( svg );
        _r->buttons.push_back( b );
        b->tooltip( _("Overlay the A and B files with optional transparencyy") );

        bW->callback( [=]( auto w ) {
            auto o = model->observeCompareOptions()->get();
            o.mode = timeline::CompareMode::Overlay;
            model->setCompareOptions( o );
            p.ui->uiView->setCompareOptions( o );
            p.ui->uiView->frameView();
            p.ui->uiView->redraw();
        } );


        bW = new Widget< Button >( g->x(), 90, 30, 30 );
        b = bW;
        svg = load_svg( "CompareDifference.svg" );
        b->image( svg );
        _r->buttons.push_back( b );
        b->tooltip( _("Difference the A and B files") );

        bW->callback( [=]( auto w ) {
            auto o = model->observeCompareOptions()->get();
            o.mode = timeline::CompareMode::Difference;
            model->setCompareOptions( o );
            p.ui->uiView->setCompareOptions( o );
            p.ui->uiView->frameView();
            p.ui->uiView->redraw();
        } );

        bW = new Widget< Button >( g->x(), 90, 30, 30 );
        b = bW;
        svg = load_svg( "CompareHorizontal.svg" );
        b->image( svg );
        _r->buttons.push_back( b );
        b->tooltip( _("Compare the A and B files side by side") );

        bW->callback( [=]( auto w ) {
            auto o = model->observeCompareOptions()->get();
            o.mode = timeline::CompareMode::Horizontal;
            model->setCompareOptions( o );
            p.ui->uiView->setCompareOptions( o );
            p.ui->uiView->frameView();
            p.ui->uiView->redraw();
        } );

        bW = new Widget< Button >( g->x(), 90, 30, 30 );
        b = bW;
        svg = load_svg( "CompareVertical.svg" );
        b->image( svg );
        _r->buttons.push_back( b );
        b->tooltip( _("Show the A file above the B file") );

        bW->callback( [=]( auto w ) {
            auto o = model->observeCompareOptions()->get();
            o.mode = timeline::CompareMode::Vertical;
            model->setCompareOptions( o );
            p.ui->uiView->setCompareOptions( o );
            p.ui->uiView->frameView();
            p.ui->uiView->redraw();
        } );

        bW = new Widget< Button >( g->x(), 90, 30, 30 );
        b = bW;
        svg = load_svg( "CompareTile.svg" );
        b->image( svg );
        _r->buttons.push_back( b );
        b->tooltip( _("Tile the A and B files") );

        bW->callback( [=]( auto w ) {
            auto o = model->observeCompareOptions()->get();
            o.mode = timeline::CompareMode::Tile;
            model->setCompareOptions( o );
            p.ui->uiView->setCompareOptions( o );
            p.ui->uiView->frameView();
            p.ui->uiView->redraw();
        } );

        bW = new Widget< Button >( g->x() + 120, 90, 30, 30 );
        b = bW;
        svg = load_svg( "Prev.svg" );
        b->image( svg );
        _r->buttons.push_back( b );
        b->tooltip( _("Previous filename") );
        bW->callback( [=]( auto w ) {
            p.ui->app->filesModel()->prevB();
        } );


        bW = new Widget< Button >( g->x() + 150, 90, 30, 30 );
        b = bW;
        svg = load_svg( "Next.svg" );
        b->image( svg );
        _r->buttons.push_back( b );
        b->tooltip( _("Next filename") );
        bW->callback( [=]( auto w ) {
            p.ui->app->filesModel()->nextB();
        } );

        bg->end();


        HorSlider* s;
        CollapsibleGroup* cg = new CollapsibleGroup( g->x(), 20, g->w(), 20,
                                                     "Wipe" );
        b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);
        cg->layout();
        cg->begin();


        auto sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20, "X" );
        s = wipeX = sV;
        s->range( 0.f, 1.0f );
        s->step( 0.01F );
        s->default_value( 0.5f );
        sV->callback( [=]( auto w ) {
            auto o = model->observeCompareOptions()->get();
            o.wipeCenter.x = w->value();
            model->setCompareOptions( o );
            p.ui->uiView->setCompareOptions( o );
            p.ui->uiView->redraw();
        } );

        sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20, "Y" );
        s = wipeY = sV;
        s->range( 0.f, 1.0f );
        s->step( 0.01F );
        s->default_value( 0.5f );
        sV->callback( [=]( auto w ) {
            auto o = model->observeCompareOptions()->get();
            o.wipeCenter.y = w->value();
            model->setCompareOptions( o );
            p.ui->uiView->setCompareOptions( o );
            p.ui->uiView->redraw();
        } );

        sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20, "Rotation" );
        s = wipeRotation = sV;
        s->range( 0.f, 360.0f );
        s->default_value( 0.0f );
        sV->callback( [=]( auto w ) {
            auto o = model->observeCompareOptions()->get();
            o.wipeRotation = w->value();
            model->setCompareOptions( o );
            p.ui->uiView->setCompareOptions( o );
            p.ui->uiView->redraw();
        } );

        cg->end();

        cg = new CollapsibleGroup( g->x(), 20, g->w(), 20, "Overlay" );
        b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);
        cg->layout();
        cg->begin();

        sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20, "Overlay" );
        s = overlay = sV;
        s->range( 0.f, 1.0f );
        s->step( 0.01F );
        s->default_value( 0.5f );
        sV->callback( [=]( auto w ) {
            auto o = model->observeCompareOptions()->get();
            o.overlay = w->value();
            model->setCompareOptions( o );
            p.ui->uiView->setCompareOptions( o );
            p.ui->uiView->redraw();
        } );

        cg->end();

        g->end();

    }

    void CompareTool::redraw()
    {
        TLRENDER_P();

        auto player = p.ui->uiView->getTimelinePlayer();
        if (!player) return;
        otio::RationalTime time = player->currentTime();

        imaging::Size size( 128, 64 );

        const auto& model = p.ui->app->filesModel();
        const auto& files = model->observeFiles();

        auto Aindex = model->observeAIndex()->get();
        auto Bindices = model->observeBIndexes()->get();
        auto o = model->observeCompareOptions()->get();

        for ( auto& m : _r->map )
        {
            const std::string fullfile = m.first;
            ClipButton* b = m.second;
            WidgetIndices::iterator it = _r->indices.find( b );
            int i = it->second;

            const auto& media = files->getItem( i );
            const auto& path = media->path;

            bool found = false;
            for( auto Bindex : Bindices )
            {
                if ( Bindex == i )
                {
                    b->value( 1 );
                    b->redraw();
                    found = true;
                    break;
                }
            }

            if ( ! found )
              {
                  b->value(0);
                  b->redraw();
                  if ( b->image() && ( o.mode == timeline::CompareMode::A ||
                                       i != Aindex ) ) continue;
              }

            if ( auto context = _r->context.lock() )
            {
                ThumbnailData* data = new ThumbnailData;
                data->widget  = b;

                WidgetIds::const_iterator it = _r->ids.find( b );
                if ( it != _r->ids.end() )
                {
                    _r->thumbnailCreator->cancelRequests( it->second );
                    _r->ids.erase(it);
                }


                auto timeline = timeline::Timeline::create(path.get(), context);
                auto timeRange = timeline->getTimeRange();

                auto startTime = timeRange.start_time();
                auto endTime   = timeRange.end_time_inclusive();

                if ( time < startTime ) time = startTime;
                else if ( time > endTime ) time = endTime;

                _r->thumbnailCreator->initThread();
                try
                {
                    int64_t id =
                        _r->thumbnailCreator->request( fullfile, time, size,
                                                       compareThumbnail_cb,
                                                       (void*)data );
                    _r->ids[b] = id;
                }
                catch (const std::exception&)
                {
                }
            }

        }

    }

    void CompareTool::refresh()
    {
        cancel_thumbnails();
        clear_controls();
        add_controls();
        end_group();
    }

}
