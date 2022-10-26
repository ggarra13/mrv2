
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
    
    typedef std::map< Fl_Button*, int64_t > WidgetIds;
    typedef std::map< Fl_Button*, size_t >  WidgetIndices;

    struct ReelTool::Private
    {
        std::weak_ptr<system::Context> context;
        mrv::ThumbnailCreator*    thumbnailCreator;
        App*                                   app;
        std::map< std::string, Fl_Button* >    map;
        WidgetIds                              ids;
        WidgetIndices                      indices;
    };

    struct ThumbnailData
    {
        Fl_Button* widget;
    };


    void reelThumbnail_cb( const int64_t id,
                              const std::vector< std::pair<otime::RationalTime,
                              Fl_RGB_Image*> >& thumbnails,
                              void* opaque )
    {
        ThumbnailData* data = static_cast< ThumbnailData* >( opaque );
        Fl_Button* w = data->widget;
        if ( reelTool )
            reelTool->reelThumbnail( id, thumbnails, w );
        delete data;
    }
        
    void ReelTool::reelThumbnail( const int64_t id,
                                  const std::vector<
                                  std::pair<otime::RationalTime, Fl_RGB_Image*>
                                  >& thumbnails, Fl_Button* w)
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
    
    ReelTool::ReelTool( ViewerUI* ui ) :
        _r( new Private ),
        ToolWidget( ui )
    {
        _r->app = ui->uiMain->app();
        std::shared_ptr<system::Context> context = _r->app->getContext();
        _r->context = context;

        add_group( "Reel" );
        
        g->callback( []( Fl_Widget* w, void* d ) {
            ToolGroup* t = (ToolGroup*) d;
            ToolGroup::cb_dismiss( NULL, t );
            delete reelTool; reelTool = nullptr;
        }, g );
        
    }

    ReelTool::~ReelTool()
    {
        clear_controls();
    }

    void ReelTool::clear_controls()
    {
        for (const auto& i : _r->map )
        {
            Fl_Button* b = i.second;
            delete b->image(); b->image(nullptr);
            delete b;
        }
        _r->map.clear();
    }

    void ReelTool::add_controls()
    {
        TLRENDER_P();

        std::shared_ptr<system::Context> context = _r->app->getContext();
        _r->thumbnailCreator = p.ui->uiTimeline->thumbnailCreator();
        
        g->clear();
        g->begin();
        const auto& model = _r->app->filesModel();
        const auto& files = model->observeFiles();
        size_t numFiles = files->getSize();
        auto Aindex = model->observeAIndex()->get();

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

            auto bW = new Widget<Fl_Button>( g->x(), g->y()+22+i*64, g->w(), 68 );
            Fl_Button* b = bW;
            _r->indices.insert( std::make_pair( b, i ) );
            bW->callback( [=]( auto b ) {
                    App* app = p.ui->uiMain->app();
                    auto model = app->filesModel();
                    model->setA( i );
                    redraw();
                } );
            
            _r->map.insert( std::make_pair( fullfile, b ) );
            
            std::string text = dir + "\n" + file;
            b->copy_label( text.c_str() );
            b->align( FL_ALIGN_LEFT | FL_ALIGN_INSIDE |
                      FL_ALIGN_IMAGE_NEXT_TO_TEXT );
            b->box( FL_ENGRAVED_BOX );
            b->labelsize( 12 );
            b->labelcolor( FL_WHITE );
            if ( Aindex == i )
            {
                b->color( FL_BLUE );
            }
            else
            {
                b->color( FL_GRAY );
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
                int64_t id = _r->thumbnailCreator->request( fullfile, time,
                                                            size,
                                                            reelThumbnail_cb,
                                                            (void*)data );
                _r->ids.insert( std::make_pair( b, id ) );
            }
            
        }

        g->end();

        
    }

    void ReelTool::redraw()
    {
        TLRENDER_P();
        
        otio::RationalTime time =
            p.ui->uiView->getTimelinePlayer()->currentTime();
        imaging::Size size( 128, 64 );
        
        const auto& model = _r->app->filesModel();
        auto Aindex = model->observeAIndex()->get();
        
        for ( auto& m : _r->map )
        {
            const std::string fullfile = m.first;
            Fl_Button* b = m.second;
            b->labelcolor( FL_WHITE );
            WidgetIndices::iterator it = _r->indices.find( b );
            int i = it->second;
            if ( Aindex == i )
            {
                b->color( FL_BLUE );
            }
            else
            {
                b->color( FL_GRAY );
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
                int64_t id = _r->thumbnailCreator->request( fullfile, time,
                                                            size,
                                                            reelThumbnail_cb,
                                                            (void*)data );
                _r->ids.insert( std::make_pair( b, id ) );
            }
        }
            
    }

    void ReelTool::refresh()
    {
        clear_controls();
        add_controls();
        end_group();
    }

}
