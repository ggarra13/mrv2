
#include <FL/Fl_Input.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>

#include "mrvFl/mrvColorTool.h"
#include "mrvFl/mrvHorSlider.h"
#include "mrvWidgets/mrvCollapsibleGroup.h"

#include "mrvFl/mrvToolsCallbacks.h"

#include "mrvFl/mrvFunctional.h"

#include "mrViewer.h"

namespace mrv
{

    
    ColorTool::ColorTool( ViewerUI* ui ) :
        colorOn( nullptr ),
        levelsOn( nullptr ),
        softClipOn( nullptr ),
        ToolWidget( ui )
    {
        add_group( "Color" );

        svg = new Fl_SVG_Image( ( svg_root + "Color.svg" ).c_str() );
        g->image( svg );
    }

    
    void ColorTool::add_controls()
    {
        TLRENDER_P();
        
        CollapsibleGroup* cg = new CollapsibleGroup( g->x(), 20, g->w(), 20,
                                                     "LUT" );
        Fl_Button* b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);

        cg->begin();

        Fl_Group* gb = new Fl_Group( g->x(), 20, g->w(), 20 );
        gb->begin();

        Fl_Input* i;
        auto iW = new Widget<Fl_Input>( g->x()+100, 20, g->w()-g->x()-120, 20,
                                        "Filename" );
        i = lutFilename = iW;
        i->labelsize(12);
        iW->callback([=]( auto o ) {
            std::string file = o->value();
            auto& lutOptions = p.ui->uiView->lutOptions();
            lutOptions.fileName = file;
            p.ui->uiView->redraw();
        });
        
        auto bW = new Widget<Fl_Button>( g->x() + g->w() - 20, 20, 20, 20,
                                         "@fileopen" );
        b = bW;
        b->align( FL_ALIGN_INSIDE | FL_ALIGN_CENTER );
        bW->callback([=]( auto t ) {
            std::string file = open_lut_file( lutFilename->value(), p.ui );
            if ( !file.empty() )
            {
                lutFilename->value( file.c_str() );
                lutFilename->do_callback();
            }
        });
        
        gb->end();

        gb = new Fl_Group( g->x(), 20, g->w(), 20 );
        gb->begin();
        auto mW = new Widget< Fl_Choice >( g->x()+100, 21, g->w()-100, 20,
                                           "Order" );
        Fl_Choice* m = mW;
        m->labelsize(12);
        m->align( FL_ALIGN_LEFT );
        m->add( "PostColorConfig" );
        m->add( "PreColorConfig" );
        m->value(0);
        mW->callback([=]( auto o ) {
            timeline::LUTOrder order = (timeline::LUTOrder)o->value();
            auto& lutOptions = p.ui->uiView->lutOptions();
            lutOptions.order = order;
            p.ui->uiView->redraw();
        });
        
        gb->end();

        cg->end();
        
        cg = new CollapsibleGroup( g->x(), 20, g->w(), 20, "Color Controls" );
        b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);

        cg->begin();
        
        Fl_Check_Button* c;
        HorSlider* s;
        auto cV = new Widget< Fl_Check_Button >( g->x()+90, 50,
                                                 g->w(), 20, "Enabled" );
        c = colorOn = cV;
        widgets.push_back( c );
        c->labelsize(12);
        cV->callback( [=]( auto w ) {
            timeline::DisplayOptions& o = p.ui->uiView->getDisplayOptions(0);
            o.colorEnabled = w->value();
            p.ui->uiView->redraw();
        } );
        
        
        auto sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20, "Add" );
        s = sV;
        widgets.push_back( s );
        s->step( 0.01f );
        s->range( 0.f, 1.0f );
        s->default_value( 0.0f );
        sV->callback( [=]( auto w ) {
            colorOn->value(1); colorOn->do_callback();
            timeline::DisplayOptions& o = p.ui->uiView->getDisplayOptions(0);
            float f = w->value();
            o.color.add = math::Vector3f( f, f, f );
            p.ui->uiView->redraw();
        } );
        
        sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20, "Contrast" );
        s = sV;
        widgets.push_back( s );
        s->range( 0.f, 4.0f );
        s->default_value( 1.0f );
        sV->callback( [=]( auto w ) {
            colorOn->value(1); colorOn->do_callback();
            timeline::DisplayOptions& o = p.ui->uiView->getDisplayOptions(0);
            float f = w->value();
            o.color.contrast = math::Vector3f( f, f, f );
            p.ui->uiView->redraw();
        } );
        
        sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20, "Saturation" );
        s = sV;
        widgets.push_back( s );
        s->range( 0.f, 4.0f );
        s->default_value( 1.0f );
        sV->callback( [=]( auto w ) {
            colorOn->value(1); colorOn->do_callback();
            timeline::DisplayOptions& o = p.ui->uiView->getDisplayOptions(0);
            float f = w->value();
            o.color.saturation = math::Vector3f( f, f, f );
            p.ui->uiView->redraw();
        } );
        
        sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20, "Tint" );
        s = sV;
        widgets.push_back( s );
        s->range( 0.f, 1.0f );
        s->step( 0.01 );
        s->default_value( 0.0f );
        sV->callback( [=]( auto w ) {
            colorOn->value(1); colorOn->do_callback();
            timeline::DisplayOptions& o = p.ui->uiView->getDisplayOptions(0);
            o.color.tint = w->value();
            p.ui->uiView->redraw();
        } );
        
        cV = new Widget< Fl_Check_Button >( g->x()+90, 50, g->w(), 20,
                                            "Invert" );
        c = cV;
        widgets.push_back( c );
        c->labelsize(12);
        cV->callback( [=]( auto w ) {
            colorOn->value(1); colorOn->do_callback();
            timeline::DisplayOptions& o = p.ui->uiView->getDisplayOptions(0);
            o.color.invert = w->value();
            p.ui->uiView->redraw();
        } );

        cg->end();

        cg = new CollapsibleGroup( g->x(), 180, g->w(), 20, "Levels" );
        b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);
        cg->layout();

        cg->begin();
        
        cV = new Widget< Fl_Check_Button >( g->x()+90, 50, g->w(), 20,
                                            "Enabled" );
        c = levelsOn = cV;
        widgets.push_back( c );
        c->labelsize(12);
        cV->callback( [=]( auto w ) {
            timeline::DisplayOptions& o = p.ui->uiView->getDisplayOptions(0);
            o.levelsEnabled = w->value();
            p.ui->uiView->redraw();
        } );

        sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20, "In Low" );
        s = sV;
        widgets.push_back( s );
        s->range( 0.f, 1.0f );
        s->step( 0.01 );
        s->default_value( 0.0f );
        sV->callback( [=]( auto w ) {
            levelsOn->value(1); levelsOn->do_callback();
            timeline::DisplayOptions& o = p.ui->uiView->getDisplayOptions(0);
            o.levels.inLow = w->value();
            p.ui->uiView->redraw();
        } );
        
        
        sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20, "In High" );
        s = sV;
        widgets.push_back( s );
        s->range( 0.f, 1.0f );
        s->step( 0.01 );
        s->default_value( 1.0f );
        sV->callback( [=]( auto w ) {
            levelsOn->value(1); levelsOn->do_callback();
            timeline::DisplayOptions& o = p.ui->uiView->getDisplayOptions(0);
            o.levels.inHigh = w->value();
            p.ui->uiView->redraw();
        } );
        
        sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20, "Gamma" );
        s = sV;
        widgets.push_back( s );
        s->range( 0.f, 6.0f );
        s->step( 0.01 );
        s->default_value( 1.0f );
        s->value( p.ui->uiGamma->value() );
        
        sV->callback( [=]( auto w ) {
            levelsOn->value(1); levelsOn->do_callback();
            timeline::DisplayOptions& o = p.ui->uiView->getDisplayOptions(0);
            float f = w->value();
            o.levels.gamma = f;
            p.ui->uiGamma->value( f );
            p.ui->uiGammaInput->value( f );
            p.ui->uiView->redraw();
        } );
        
        sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20, "Out Low" );
        s = sV;
        widgets.push_back( s );
        s->range( 0.f, 1.0f );
        s->step( 0.01 );
        s->default_value( 0.0f );
        sV->callback( [=]( auto w ) {
            levelsOn->value(1); levelsOn->do_callback();
            timeline::DisplayOptions& o = p.ui->uiView->getDisplayOptions(0);
            o.levels.outLow = w->value();
            p.ui->uiView->redraw();
        } );
        
        
        sV = new Widget< HorSlider >( g->x(), 90, g->w(), 20, "Out High" );
        s = sV;
        widgets.push_back( s );
        s->range( 0.f, 1.0f );
        s->step( 0.01 );
        s->default_value( 1.0f );
        sV->callback( [=]( auto w ) {
            levelsOn->value(1); levelsOn->do_callback();
            timeline::DisplayOptions& o = p.ui->uiView->getDisplayOptions(0);
            o.levels.inHigh = w->value();
            p.ui->uiView->redraw();
        } );
        
        cg->end();
        cg = new CollapsibleGroup( g->x(), 180, g->w(), 20, "Soft Clip" );
        b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);
        cg->layout();

        cg->begin();

        cV = new Widget< Fl_Check_Button >( g->x()+90, 120, g->w(), 20,
                                            "Enabled" );
        c = softClipOn = cV;
        widgets.push_back( c );
        c->labelsize(12);
        cV->callback( [=]( auto w ) {
            timeline::DisplayOptions& o = p.ui->uiView->getDisplayOptions(0);
            o.softClipEnabled = w->value();
            p.ui->uiView->redraw();
        } );
        
        sV = new Widget< HorSlider >( g->x(), 140, g->w(), 20, "Soft Clip" );
        s = sV;
        widgets.push_back( s );
        s->range( 0.f, 1.0f );
        s->step( 0.01 );
        s->default_value( 0.0f );
        sV->callback( [=]( auto w ) {
            softClipOn->value(1); softClipOn->do_callback();
            timeline::DisplayOptions& o = p.ui->uiView->getDisplayOptions(0);
            o.softClip = w->value();
            p.ui->uiView->redraw();
        } );
        
        
        
        cg->end();

        
        g->callback( []( Fl_Widget* w, void* d ) {
            delete colorTool; colorTool = nullptr;
        }, g );
    }

    void ColorTool::refresh() noexcept
    {
        // Change of movie file.  Refresh colors by calling all widget callbacks
        for ( auto& widget : widgets )
        {
            widget->do_callback();
        }
    }
}
