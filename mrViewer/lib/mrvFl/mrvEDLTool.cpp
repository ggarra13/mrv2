// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <string>
#include <vector>
#include <map>

#include <FL/Fl_Button.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_RGB_Image.H>



#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvClipButton.h"
#include "mrvWidgets/mrvButton.h"

#include "mrvFl/mrvEDLTool.h"
#include "mrvFl/mrvToolsCallbacks.h"

#include "mrvGL/mrvThumbnailCreator.h"

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/App.h"

#include "mrViewer.h"


namespace mrv
{

    typedef std::map< ClipButton*, int64_t > WidgetIds;

    struct EDLTool::Private
    {
        std::weak_ptr<system::Context> context;
        mrv::ThumbnailCreator*    thumbnailCreator;

        std::vector< std::shared_ptr<FilesModelItem> > edl;

        
        WidgetIds                              ids;
        std::vector< ClipButton* >           clips;
        std::vector< Fl_Button* >          buttons;
    };

    struct ThumbnailData
    {
        ClipButton* widget;
    };



    void EDLThumbnail_cb( const int64_t id,
                              const std::vector< std::pair<otime::RationalTime,
                              Fl_RGB_Image*> >& thumbnails,
                              void* opaque )
    {
        ThumbnailData* data = static_cast< ThumbnailData* >( opaque );
        ClipButton* w = data->widget;
        if ( edlTool )
            edlTool->EDLThumbnail( id, thumbnails, w );
        delete data;
    }

    void EDLTool::EDLThumbnail( const int64_t id,
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

    EDLTool::EDLTool( ViewerUI* ui ) :
        _r( new Private ),
        ToolWidget( ui )
    {
        _r->context = ui->app->getContext();

        add_group( _("EDL Edit") );

        // Fl_SVG_Image* svg = load_svg( "EDLEdit.svg" );
        // g->image( svg );


        g->callback( []( Fl_Widget* w, void* d ) {
            ViewerUI* ui = static_cast< ViewerUI* >( d );
            delete edlTool; edlTool = nullptr;
            ui->uiMain->fill_menu( ui->uiMenuBar );
        }, ui );

    }

    EDLTool::~EDLTool()
    {
        cancel_thumbnails();
        clear_controls();
    }

    void EDLTool::clear_controls()
    {
        // nothing to do here
    }

  void EDLTool::cancel_thumbnails()
  {
    for ( const auto& it : _r->ids )
      {
        _r->thumbnailCreator->cancelRequests( it.second );
      }

    _r->ids.clear();
  }

    void EDLTool::add_controls()
    {
        TLRENDER_P();

        _r->thumbnailCreator =
            p.ui->uiTimeWindow->uiTimeline->thumbnailCreator();

        g->clear();
        g->begin();
        
        const auto& model = p.ui->app->filesModel();
        
        size_t numFiles = _r->edl.size();
        
        const auto& player = p.ui->uiView->getTimelinePlayer();

        otio::RationalTime time = otio::RationalTime(0.0,1.0);
        if ( player )      time = player->currentTime();


        imaging::Size size( 128, 64 );

        for ( size_t i = 0; i < numFiles; ++i )
        {
            const auto& media = _r->edl[i];
            const auto& path = media->path;

            const std::string& dir = path.getDirectory();
            const std::string file = path.getBaseName() + path.getNumber() +
                                     path.getExtension();
            const std::string& fullfile = path.get();

            auto bW = new Widget<ClipButton>( g->x(), g->y()+20+i*68, g->w(), 68 );
            ClipButton* b = bW;
            b->deactivate();
            bW->callback( [=]( auto b ) {
                // @todo: no
                } );


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
                                                       EDLThumbnail_cb,
                                                       (void*)data );
                    _r->ids[b] = id;
                }
                catch (const std::exception&)
                {
                }
            }

        }

        int Y = g->y() + 20 + numFiles*64 ;

        Fl_Pack* bg = new Fl_Pack( g->x(), Y, g->w(), 30 );

        bg->type( Fl_Pack::HORIZONTAL );
        bg->begin();
        
        auto bW = new Widget< Button >( g->x() + 150, Y, 50, 30, "Add" );
        Button* b = bW;
        b->tooltip( _("Add current file to EDL") );
        bW->callback( [=]( auto w ) {
            const auto& model = p.ui->app->filesModel();
            const auto& files = model->observeFiles();
            const auto Aindex = model->observeAIndex()->get();
            const auto& item = files->getItem( Aindex );
            auto clip = std::make_shared<FilesModelItem>();
            clip = item;
            const auto& player = p.ui->uiView->getTimelinePlayer();
            clip->inOutRange = player->inOutRange();
            _r->edl.push_back( clip );
            refresh();
        } );
        
        bW = new Widget< Button >( g->x() + 150, Y, 50, 30, "Clear" );
        b = bW;
        b->tooltip( _("Ckear all files from EDL") );
        bW->callback( [=]( auto w ) {
            _r->edl.clear();
            refresh();
        } );
        
        bW = new Widget< Button >( g->x() + 150, Y, 50, 30, "EDL" );
        b = bW;
        b->tooltip( _("Create .otio EDL") );
        bW->callback( [=]( auto w ) {
            create_edl( p.ui, _r->edl );
            _r->edl.clear();
            refresh();
        } );
        
        bg->end();

        g->end();

    }


    void EDLTool::refresh()
    {
        cancel_thumbnails();
        clear_controls();
        add_controls();
        end_group();
    }

}
