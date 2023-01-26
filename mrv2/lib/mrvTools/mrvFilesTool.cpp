// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <string>
#include <vector>
#include <map>

#include "FL/Fl_Pack.H"

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvFileButton.h"
#include "mrvWidgets/mrvButton.h"

#include "mrvFilesTool.h"
#include "mrvToolsCallbacks.h"

#include "mrvGL/mrvThumbnailCreator.h"

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/App.h"

#include "mrViewer.h"

#include "mrvFl/mrvIO.h"


namespace mrv
{

    typedef std::map< FileButton*, int64_t > WidgetIds;
    typedef std::map< FileButton*, size_t >  WidgetIndices;

    struct FilesTool::Private
    {
        std::weak_ptr<system::Context> context;
        mrv::ThumbnailCreator*    thumbnailCreator;
        App*                                   app;
        std::map< std::string, FileButton* >    map;
        WidgetIds                              ids;
        WidgetIndices                      indices;
        std::vector< Fl_Button* >        buttons;
    };

    struct ThumbnailData
    {
        FileButton* widget;
    };


    void filesThumbnail_cb( const int64_t id,
                            const std::vector< std::pair<otime::RationalTime,
                            Fl_RGB_Image*> >& thumbnails, void* opaque )
    {
        ThumbnailData* data = static_cast< ThumbnailData* >( opaque );
        FileButton* w = data->widget;
        if ( filesTool )
            filesTool->filesThumbnail( id, thumbnails, w );
        delete data;
    }

    void FilesTool::filesThumbnail( const int64_t id,
                                    const std::vector<
                                    std::pair<otime::RationalTime,
                                    Fl_RGB_Image*> >& thumbnails, FileButton* w)
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

    FilesTool::FilesTool( ViewerUI* ui ) :
        _r( new Private ),
        ToolWidget( ui )
    {
        _r->context = ui->app->getContext();

        add_group( _("Files") );


        Fl_SVG_Image* svg = load_svg( "Files.svg" );
        g->image( svg );


        g->callback( []( Fl_Widget* w, void* d ) {
            ViewerUI* ui = static_cast< ViewerUI* >( d );
            delete filesTool; filesTool = nullptr;
            ui->uiMain->fill_menu( ui->uiMenuBar );
        }, ui );

    }

    FilesTool::~FilesTool()
    {
        cancel_thumbnails();
        clear_controls();
    }

    void FilesTool::cancel_thumbnails()
    {
        for ( const auto& it : _r->ids )
        {
            _r->thumbnailCreator->cancelRequests( it.second );
        }

        _r->ids.clear();
    }

    void FilesTool::clear_controls()
    {
        for (const auto& i : _r->map )
        {
            Fl_Button* b = i.second;

            delete b->image(); b->image(nullptr);

            g->remove( b );

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

    void FilesTool::add_controls()
    {
        TLRENDER_P();

        Fl_SVG_Image* svg;
        _r->thumbnailCreator =
            p.ui->uiTimeWindow->uiTimeline->thumbnailCreator();


        g->clear();

        g->begin();

        const auto& model = p.ui->app->filesModel();

        const auto& files = model->observeFiles();

        size_t numFiles = files->getSize();

        auto Aindex = model->observeAIndex()->get();

        const auto& player = p.ui->uiView->getTimelinePlayer();


        otio::RationalTime time = otio::RationalTime(0.0,1.0);
        if ( player )      time = player->currentTime();

        imaging::Size size( 128, 64 );

        for ( size_t i = 0; i < numFiles; ++i )
        {
            const auto& media = files->getItem( i );
            const auto& path = media->path;




            const std::string& dir = path.getDirectory();
            const std::string file = path.getBaseName() + path.getNumber() +
                                     path.getExtension();
            const std::string fullfile = dir + file;

            auto bW = new Widget<FileButton>( g->x(), g->y()+22+i*68,
                                              g->w(), 68 );
            FileButton* b = bW;
            _r->indices.insert( std::make_pair( b, i ) );
            bW->callback( [=]( auto b ) {
                WidgetIndices::const_iterator it = _r->indices.find( b );
                if ( it == _r->indices.end() ) return;
                int index = (*it).second;
                auto model = _p->ui->app->filesModel();
                model->setA( index );
                redraw();
            } );

            _r->map[ fullfile ] = b;


            std::string text = dir + "\n" + file;
            b->copy_label( text.c_str() );
            if ( Aindex == i )
            {
                b->value( 1 );
            }
            else
            {
                b->value( 0 );
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
                                                       filesThumbnail_cb,
                                                       (void*)data );
                    _r->ids[ b ] = id;
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


        Fl_Button* b;
        auto bW = new Widget< Button >( g->x(), Y, 30, 30 );
        b = bW;

        svg = load_svg( "FileOpen.svg" );
        b->image( svg );

        _r->buttons.push_back( b );


        b->tooltip( _("Open a filename") );
        bW->callback( [=]( auto w ) {
            open_cb( w, p.ui );
        } );


        bW = new Widget< Button >( g->x() + 30, Y, 30, 30 );
        b = bW;
        svg = load_svg( "FileOpenSeparateAudio.svg" );
        b->image( svg );
        _r->buttons.push_back( b );
        b->tooltip( _("Open a filename with audio") );
        bW->callback( [=]( auto w ) {
            open_separate_audio_cb( w, p.ui );
        } );

        bW = new Widget< Button >( g->x() + 60, Y, 30, 30 );
        b = bW;
        svg = load_svg( "FileClose.svg" );
        b->image( svg );
        _r->buttons.push_back( b );
        b->tooltip( _("Close current filename") );
        bW->callback( [=]( auto w ) {
            close_current_cb( w, p.ui );
        } );


        bW = new Widget< Button >( g->x() + 90, Y, 30, 30 );
        b = bW;
        svg = load_svg( "FileCloseAll.svg" );
        b->image( svg );
        _r->buttons.push_back( b );
        b->tooltip( _("Close all filenames") );
        bW->callback( [=]( auto w ) {
            close_all_cb( w, p.ui );
        } );


        bW = new Widget< Button >( g->x() + 120, Y, 30, 30 );
        b = bW;
        svg = load_svg( "Prev.svg" );
        b->image( svg );
        _r->buttons.push_back( b );
        b->tooltip( _("Previous filename") );
        bW->callback( [=]( auto w ) {
            p.ui->app->filesModel()->prev();
        } );


        bW = new Widget< Button >( g->x() + 150, Y, 30, 30 );
        b = bW;
        svg = load_svg( "Next.svg" );
        b->image( svg );
        _r->buttons.push_back( b );
        b->tooltip( _("Next filename") );
        bW->callback( [=]( auto w ) {
            p.ui->app->filesModel()->next();
        } );



        bg->end();

        g->end();



    }

    void FilesTool::redraw()
    {

        TLRENDER_P();

        otio::RationalTime time = otio::RationalTime(0.0,1.0);

        const auto& player = p.ui->uiView->getTimelinePlayer();
        if ( player )      time = player->currentTime();

        imaging::Size size( 128, 64 );

        const auto& model = p.ui->app->filesModel();
        auto Aindex = model->observeAIndex()->get();

        for ( auto& m : _r->map )
        {
            const std::string fullfile = m.first;
            FileButton* b = m.second;

            b->labelcolor( FL_WHITE );
            WidgetIndices::iterator it = _r->indices.find( b );
            int i = it->second;
            if ( Aindex != i )
            {
                b->value( 0 );
                if ( b->image() ) continue;
                time = otio::RationalTime(0.0,1.0);
            }
            else
              {
                  b->value( 1 );
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

                _r->thumbnailCreator->initThread();
                try
                {
                    int64_t id =
                        _r->thumbnailCreator->request( fullfile, time, size,
                                                       filesThumbnail_cb,
                                                       (void*)data );
                    _r->ids[ b ] = id;
                }
                catch (const std::exception&)
                {
                }
            }
        }

    }

    void FilesTool::refresh()
    {
        cancel_thumbnails();
        clear_controls();
        add_controls();
        end_group();
    }

}
