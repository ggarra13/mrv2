
#include <string>
#include <map>

#include "FL/Fl_Button.H"
#include "FL/Fl_Pack.H"


#include "mrvFl/mrvCompareTool.h"
#include "mrvFl/mrvHorSlider.h"

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

    struct CompareTool::Private
    {
        std::weak_ptr<system::Context> context;
        mrv::ThumbnailCreator*    thumbnailCreator;
        std::map< std::string, Fl_Button* >    map;
        WidgetIds                              ids;
        WidgetIndices                      indices;
    };

    struct ThumbnailData
    {
        Fl_Button* widget;
    };

    

    void compareThumbnail_cb( const int64_t id,
                              const std::vector< std::pair<otime::RationalTime,
                              Fl_RGB_Image*> >& thumbnails,
                              void* opaque )
    {
        ThumbnailData* data = static_cast< ThumbnailData* >( opaque );
        Fl_Button* w = data->widget;
        if ( compareTool )
            compareTool->compareThumbnail( id, thumbnails, w );
        delete data;
    }
        
    void CompareTool::compareThumbnail( const int64_t id,
                                        const std::vector< std::pair<otime::RationalTime,
                                        Fl_RGB_Image*> >& thumbnails,
                                        Fl_Button* w)
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

        add_group( "Compare" );
        
        svg = load_svg( "Compare.svg" );
        g->image( svg );
        
        g->callback( []( Fl_Widget* w, void* d ) {
            delete compareTool; compareTool = nullptr;
        }, g );
        
    }

    CompareTool::~CompareTool()
    {
        clear_controls();
    }

    void CompareTool::clear_controls()
    {
        for (const auto& i : _r->map )
        {
            Fl_Button* b = i.second;
            delete b->image(); b->image(nullptr);
            delete b;
        }
        _r->map.clear();
    }

    void CompareTool::add_controls()
    {
        TLRENDER_P();

        _r->thumbnailCreator = p.ui->uiTimeline->thumbnailCreator();
	if ( !_r->thumbnailCreator )
	  {
	    std::cerr << "no thumbnail creator in uiTimeline" << std::endl;
	    return;
	  }
        
        g->clear();
        g->begin();
        const auto& model = p.ui->app->filesModel();
        const auto& files = model->observeFiles();
        size_t numFiles = files->getSize();
        auto Bindices = model->observeBIndexes()->get();

        auto player = p.ui->uiView->getTimelinePlayer();
        if (!player) return;
        otio::RationalTime time = player->currentTime();

        imaging::Size size( 128, 64 );
            
        for ( size_t i = 0; i < numFiles; ++i )
        {
            const auto& media = files->getItem( i );
            const auto& path = media->path;


            const std::string& dir = path.getDirectory();
            const std::string file = path.getBaseName() + path.getNumber() +
                                     path.getExtension();
            const std::string fullfile = dir + file;

            auto bW = new Widget<Fl_Button>( g->x(), g->y()+20+i*68, g->w(), 68 );
            Fl_Button* b = bW;
            _r->indices.insert( std::make_pair( b, i ) );
            
            b->color( FL_GRAY );
            b->labelcolor( FL_WHITE );

            for( auto Bindex : Bindices )
            {
                if ( Bindex == i )
                {
                    b->color( FL_BLUE );
                    break;
                }
            }
            bW->callback( [=]( auto b ) {
                    auto model = p.ui->app->filesModel();
                    model->setB( i, true );
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
                                                            compareThumbnail_cb,
                                                            (void*)data );
                _r->ids.insert( std::make_pair( b, id ) );
            }
            
        }

        Fl_Pack* bg = new Fl_Pack( g->x(), g->y()+20+numFiles*64, g->w(), 20 );
        bg->type( Fl_Pack::HORIZONTAL );
	bg->begin();

        Fl_Button* b;
        auto bW = new Widget< Fl_Button >( g->x(), 90, 30, 30 );
        b = bW;
        svg = load_svg( "CompareA.svg" );
        b->image( svg );
        b->tooltip( _("Compare A") );
        bW->callback( [=]( auto w ) {
            auto o = model->observeCompareOptions()->get();
            o.mode = timeline::CompareMode::A;
            model->setCompareOptions( o );
            p.ui->uiView->setCompareOptions( o );
            p.ui->uiView->frameView();
            p.ui->uiView->redraw();
        } );
        
        bW = new Widget< Fl_Button >( g->x(), 90, 30, 30 );
        b = bW;
        svg = load_svg( "CompareB.svg" );
        b->image( svg );
        b->tooltip( _("Compare B") );
        
        bW->callback( [=]( auto w ) {
            auto o = model->observeCompareOptions()->get();
            o.mode = timeline::CompareMode::B;
            model->setCompareOptions( o );
            p.ui->uiView->setCompareOptions( o );
            p.ui->uiView->frameView();
            p.ui->uiView->redraw();
        } );
        
        bW = new Widget< Fl_Button >( g->x(), 90, 30, 30 );
        b = bW;
        svg = load_svg( "CompareWipe.svg" );
        b->image( svg );
        b->tooltip( _("Compare Wipe") );
        
        bW->callback( [=]( auto w ) {
            auto o = model->observeCompareOptions()->get();
            o.mode = timeline::CompareMode::Wipe;
            model->setCompareOptions( o );
            p.ui->uiView->setCompareOptions( o );
            p.ui->uiView->frameView();
            p.ui->uiView->redraw();
        } );
        
        
        bW = new Widget< Fl_Button >( g->x(), 90, 30, 30 );
        b = bW;
        svg = load_svg( "CompareOverlay.svg" );
        b->image( svg );
        b->tooltip( _("Compare Overlay") );
        
        bW->callback( [=]( auto w ) {
            auto o = model->observeCompareOptions()->get();
            o.mode = timeline::CompareMode::Overlay;
            model->setCompareOptions( o );
            p.ui->uiView->setCompareOptions( o );
            p.ui->uiView->frameView();
            p.ui->uiView->redraw();
        } );
        

        bW = new Widget< Fl_Button >( g->x(), 90, 30, 30 );
        b = bW;
        svg = load_svg( "CompareDifference.svg" );
        b->image( svg );
        b->tooltip( _("Compare Difference") );
        
        bW->callback( [=]( auto w ) {
            auto o = model->observeCompareOptions()->get();
            o.mode = timeline::CompareMode::Difference;
            model->setCompareOptions( o );
            p.ui->uiView->setCompareOptions( o );
            p.ui->uiView->frameView();
            p.ui->uiView->redraw();
        } );
        
        bW = new Widget< Fl_Button >( g->x(), 90, 30, 30 );
        b = bW;
        svg = load_svg( "CompareHorizontal.svg" );
        b->image( svg );
        b->tooltip( _("Compare Horizontal") );
        
        bW->callback( [=]( auto w ) {
            auto o = model->observeCompareOptions()->get();
            o.mode = timeline::CompareMode::Horizontal;
            model->setCompareOptions( o );
            p.ui->uiView->setCompareOptions( o );
            p.ui->uiView->frameView();
            p.ui->uiView->redraw();
        } );
        
        bW = new Widget< Fl_Button >( g->x(), 90, 30, 30 );
        b = bW;
        svg = load_svg( "CompareVertical.svg" );
        b->image( svg );
        b->tooltip( _("Compare Vertical") );
        
        bW->callback( [=]( auto w ) {
            auto o = model->observeCompareOptions()->get();
            o.mode = timeline::CompareMode::Vertical;
            model->setCompareOptions( o );
            p.ui->uiView->setCompareOptions( o );
            p.ui->uiView->frameView();
            p.ui->uiView->redraw();
        } );
        
        bW = new Widget< Fl_Button >( g->x(), 90, 30, 30 );
        b = bW;
        svg = load_svg( "CompareTile.svg" );
        b->image( svg );
        b->tooltip( _("Compare Tile") );
        
        bW->callback( [=]( auto w ) {
            auto o = model->observeCompareOptions()->get();
            o.mode = timeline::CompareMode::Tile;
            model->setCompareOptions( o );
            p.ui->uiView->setCompareOptions( o );
            p.ui->uiView->frameView();
            p.ui->uiView->redraw();
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
        s = sV;
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
        s = sV;
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
        s = sV;
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
        s = sV;
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
        auto Aindex = model->observeAIndex()->get();
        
        for ( auto& m : _r->map )
        {
            const std::string fullfile = m.first;
            Fl_Button* b = m.second;
            WidgetIndices::iterator it = _r->indices.find( b );
            int i = it->second;
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
                                                            compareThumbnail_cb,
                                                            (void*)data );
                _r->ids.insert( std::make_pair( b, id ) );
            }
            
        }
            
    }

    void CompareTool::refresh()
    {
        clear_controls();
        add_controls();
        end_group();
    }

}
