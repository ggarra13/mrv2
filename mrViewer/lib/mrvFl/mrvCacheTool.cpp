
#include <FL/Fl_Input.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>

#include "mrvFl/mrvDockGroup.h"
#include "mrvFl/mrvCacheTool.h"
#include "mrvFl/mrvToolGroup.h"
#include "mrvFl/mrvHorSlider.h"
#include "mrvFl/mrvCollapsibleGroup.h"

#include "mrvFl/mrvToolsCallbacks.h"

#include "mrvFl/mrvFunctional.h"

#include <mrvPlayApp/mrvFilesModel.h>

#include "mrViewer.h"

namespace mrv
{

    
    CacheTool::CacheTool( ViewerUI* ui ) :
        ToolWidget( ui )
    {
        add_group( "A/V" );
    }

    
    void CacheTool::add_controls()
    {
        TLRENDER_P();
        
        auto cg = new CollapsibleGroup( g->x(), 20, g->w(), 20, "Caches" );
        auto b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);

        cg->begin();
        
        HorSlider* s;
        auto sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20,
                                           "Read Ahead" );
        s = sV;
        s->step( 0.1f );
        s->range( 0.f, 100.0f );
        s->default_value( 5.0f );
        s->value( p.ui->uiPrefs->uiPrefsCacheReadAhead->value() );
        sV->callback( [=]( auto w ) {
            p.ui->uiPrefs->uiPrefsCacheReadAhead->value( w->value() );
            size_t active = p.app->filesModel()->observeActive()->get().size();
            auto players = p.ui->uiView->getTimelinePlayers();
            for ( auto& player : players )
            {
                auto value = w->value() / static_cast<double>( active );
                player->setCacheReadAhead( otio::RationalTime( value, 1.0 ) );
            }
        } );
        
        sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20,
                                      "Read Behind" );
        s = sV;
        s->step( 0.1f );
        s->range( 0.f, 100.0f );
        s->default_value( 0.1f );
        s->value( p.ui->uiPrefs->uiPrefsCacheReadBehind->value() );
        sV->callback( [=]( auto w ) {
            p.ui->uiPrefs->uiPrefsCacheReadBehind->value( w->value() );
            size_t active = p.app->filesModel()->observeActive()->get().size();
            auto players = p.ui->uiView->getTimelinePlayers();
            for ( auto& player : players )
            {
                auto value = w->value() / static_cast<double>( active );
                player->setCacheReadBehind( otio::RationalTime( value, 1.0 ) );
            }
        } );
        
        cg->end();

        
        g->callback( []( Fl_Widget* w, void* d ) {
            ToolGroup* t = (ToolGroup*) d;
            ToolGroup::cb_dismiss( NULL, t );
            delete cacheTool; cacheTool = nullptr;
        }, g );
    }

}
