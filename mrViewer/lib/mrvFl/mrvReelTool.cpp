
#include <string>
#include <map>

#include "mrvFl/mrvDockGroup.h"
#include "mrvFl/mrvReelTool.h"
#include "mrvFl/mrvToolGroup.h"

#include "mrvFl/mrvToolsCallbacks.h"

#include "mrvFl/mrvFunctional.h"

#include "mrvGL/mrvThumbnailCreator.h"

#include "mrvPlayApp/mrvFilesModel.h"
#include "mrvPlayApp/App.h"

#include "mrViewer.h"


namespace mrv
{
    
    typedef std::map< Fl_Box*, int64_t > WidgetIds;

    struct ReelTool::Private
    {
        std::weak_ptr<system::Context> context;
        std::unique_ptr<mrv::ThumbnailCreator> thumbnailCreator;
        App*                                   app;
        std::map< std::string, Fl_Box* >       map;
        WidgetIds                              ids;
    };

    struct ThumbnailData
    {
        Fl_Box* widget;
    };


    void createdThumbnail_cb( const int64_t id,
                              const std::vector< std::pair<otime::RationalTime,
                              Fl_RGB_Image*> >& thumbnails,
                              void* opaque )
    {
        ThumbnailData* data = static_cast< ThumbnailData* >( opaque );
        Fl_Box* w = data->widget;
        if ( reelTool )
            reelTool->createdThumbnail( id, thumbnails, w );
        delete data;
    }
        
    void ReelTool::createdThumbnail( const int64_t id,
                                     const std::vector< std::pair<otime::RationalTime,
                                     Fl_RGB_Image*> >& thumbnails,
                                     Fl_Box* w)
    {
        WidgetIds::const_iterator it = _r->ids.find( w );
        if ( it == _r->ids.end() ) return;

        if ( it->second == id )
        {       
            for (const auto& i : thumbnails)
            {
                delete w->image();
                w->image( i.second );
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
    
    ReelTool::ReelTool( ViewerUI* ui ) :
        _r( new Private ),
        ToolWidget( ui )
    {
        _r->app = ui->uiMain->app();
        std::shared_ptr<system::Context> context = _r->app->getContext();
        _r->context = context;
        _r->thumbnailCreator = std::make_unique<mrv::ThumbnailCreator>( context );

        add_group( "Reel" );
        
        g->callback( []( Fl_Widget* w, void* d ) {
            ToolGroup* t = (ToolGroup*) d;
            ToolGroup::cb_dismiss( NULL, t );
            delete reelTool; reelTool = nullptr;
        }, g );
        
    }

    ReelTool::~ReelTool()
    {
        for (const auto& i : _r->map )
        {
            Fl_Box* b = i.second;
            delete b->image(); b->image(nullptr);
            delete b;
        }
    }

    void ReelTool::add_controls()
    {
        TLRENDER_P();
        const auto& model = _r->app->filesModel();
        const auto& files = model->observeFiles();
        size_t numFiles = files->getSize();

        otio::RationalTime time =
            p.ui->uiView->getTimelinePlayer()->currentTime();
        imaging::Size size( 128, 64 );
            
        for ( size_t i = 0; i < numFiles; ++i )
        {
            const auto& media = files->getItem( i );
            const auto& path = media->path;


            const std::string& dir = path.getDirectory();
            const std::string file = path.getBaseName() + path.getNumber() +
                                     path.getExtension();
            const std::string fullfile = dir + file;

            Fl_Box* w = new Fl_Box( g->x(), g->y()+20+i*64, g->w(), 64 );
            _r->map.insert( std::make_pair( fullfile, w ) );
            
            std::string text = dir + "\n" + file;
            w->copy_label( text.c_str() );
            w->align( FL_ALIGN_LEFT | FL_ALIGN_INSIDE |
                      FL_ALIGN_IMAGE_NEXT_TO_TEXT );
            w->box( FL_ENGRAVED_BOX );
            w->labelcolor( FL_WHITE );


            if ( auto context = _r->context.lock() )
            {
                ThumbnailData* data = new ThumbnailData;
                data->widget  = w;
                _r->thumbnailCreator->initThread();
                int64_t id = _r->thumbnailCreator->request( fullfile, time,
                                                            size,
                                                            createdThumbnail_cb,
                                                            (void*)data );
                _r->ids.insert( std::make_pair( w, id ) );
            }
            
        }
        
    }


    void ReelTool::refresh()
    {
        TLRENDER_P();
        
        otio::RationalTime time =
            p.ui->uiView->getTimelinePlayer()->currentTime();
        imaging::Size size( 128, 76 );
            
        for ( const auto& m : _r->map )
        {

            const std::string& fullfile = m.first;
            Fl_Box* w = m.second;

            if ( auto context = _r->context.lock() )
            {
                ThumbnailData* data = new ThumbnailData;
                data->widget  = w;

                WidgetIds::const_iterator it = _r->ids.find( w );
                if ( it != _r->ids.end() )
                {
                    _r->thumbnailCreator->cancelRequests( it->second );
                    _r->ids.erase(it);
                }
                _r->thumbnailCreator->initThread();
                int64_t id = _r->thumbnailCreator->request( fullfile, time,
                                                            size,
                                                            createdThumbnail_cb,
                                                            (void*)data );
                _r->ids.insert( std::make_pair( w, id ) );
            }
            
        }
    }

}
