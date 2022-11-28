// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.



#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Radio_Round_Button.H>

#include "mrvFl/mrvAnnotationsTool.h"

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvHorSlider.h"
#include "mrvWidgets/mrvCollapsibleGroup.h"
#include "mrvWidgets/mrvDoubleSpinner.h"
#include "mrvWidgets/mrvMultilineInput.h"

#include "mrvFl/mrvToolsCallbacks.h"

#include "mrvFl/mrvDefines.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrViewer.h"



namespace mrv
{
    
    AnnotationsTool::AnnotationsTool( ViewerUI* ui ) :
        ToolWidget( ui )
    {
        add_group( _( "Annotations" ) );

        // @todo. add an icon
        // Fl_SVG_Image* svg = load_svg( "Annotations.svg" );
        // g->image( svg );
        
        g->callback( []( Fl_Widget* w, void* d ) {
            ViewerUI* ui = static_cast< ViewerUI* >( d );
            delete annotationsTool; annotationsTool = nullptr;
            ui->uiMain->fill_menu( ui->uiMenuBar );
        }, ui );
    }

    
    void AnnotationsTool::add_controls()
    {
        TLRENDER_P();

        SettingsObject* settingsObject = p.ui->app->settingsObject();

        int X = g->x();
        int Y = 20;
        
        std_any value;
        Fl_Box* box;
        Pack*    sg;
        CollapsibleGroup* cg;
        Fl_Round_Button* r;
        DoubleSpinner*   d;
        Fl_Button* b;
        HorSlider* s;
        Fl_Choice* c;

        cg = new CollapsibleGroup( X, Y, g->w(), 40, _("Text") );
        b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);

        cg->begin();
        
        Fl_Group* bg = new Fl_Group( g->x(), Y+20, g->w(), 40 );
        bg->begin();
	
	bg = new Fl_Group( g->x(), Y+20, g->w(), 40 );
        bg->begin();
        
        auto cW = new Widget< Fl_Choice >( g->x()+100, Y+20,
                                           g->w()-100, 20,
                                           _("Font") );
        c = cW;
        c->labelsize(12);
        c->align( FL_ALIGN_LEFT );

#ifdef USE_OPENGL2
        auto numFonts = Fl::set_fonts( "-*" );
        for ( unsigned i = 0; i < numFonts; ++i )
	  {
            int attrs = 0;
            const char* fontName = Fl::get_font_name( (Fl_Font) i,
                                                      &attrs );
            c->add( fontName );
	  }
#else
        const char* kFonts[3] = {
	  "NotoSans-Regular",
	  "NotoSans-Bold",
	  "NotoMono-Regular"
        };

        int numFonts = sizeof(kFonts) / sizeof(char*);
        for ( unsigned i = 0; i < numFonts; ++i )
	  {
            c->add( kFonts[i] );
	  }
#endif

        value = settingsObject->value( kTextFont );
        int font = std_any_empty( value ) ? FL_HELVETICA :
	  std_any_cast<int>( value );
        if ( font > numFonts ) font = 0;
        c->value( font );
        c->tooltip( _("Selects the current font from the list") );
        cW->callback([=]( auto o ) {
	  int font = o->value();
	  auto numFonts = Fl::set_fonts( "-*" );
	  settingsObject->setValue( kTextFont, font );
	  MultilineInput* w = p.ui->uiView->getMultilineInput();
	  if (!w) return;
	  if ( font >= numFonts ) font = FL_HELVETICA;
#ifdef USE_OPENGL2
	  int attrs = 0;
	  const char* fontName = Fl::get_font_name( (Fl_Font) font,
						    &attrs );
#else
	  const Fl_Menu_Item* item = c->mvalue();
	  std::string fontName = item->label();
	  w->fontFamily = fontName;
#endif
	  w->textfont( (Fl_Font ) font );
	  w->redraw();
	} );


	
        auto sV = new Widget< HorSlider >( X, Y+40, g->w(), 20,
                                           _("Size:") );
        s = sV;
        s->range( 12, 100 );
        s->step( 1 );
        s->tooltip( _("Selects the current font size.") );
        value = settingsObject->value( kFontSize );
        s->default_value( std_any_empty( value ) ? kFONT_SIZE :
                          std_any_cast< int >( value ) );
	settingsObject->setValue( kFontSize, (int) s->value() );
        sV->callback( [=]( auto o ) {
            settingsObject->setValue( kFontSize, (int) o->value() );
            const auto& viewportSize =
                p.ui->uiView->getViewportSize();
            float pct = viewportSize.h / 1024.F;
            MultilineInput* w = p.ui->uiView->getMultilineInput();
            if (!w) return;
            int fontSize = o->value() * pct *
                           p.ui->uiView->viewZoom();
            w->textsize( fontSize );
            w->redraw();
            p.ui->uiView->redrawWindows();
        } );

        bg->end();
	
        cg->end();
        
        cg = new CollapsibleGroup( X, Y, g->w(), 65, _("Pen") );
        b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);

        cg->begin();
        
        Fl_Group* pg = new Fl_Group( X, Y, g->w(), 30 );
        pg->begin();
        
        b = penColor = new Fl_Button( X+100, Y, 25, 25, _("Color:") );
        b->tooltip(_("Selects the current pen color."));
        b->box(FL_EMBOSSED_BOX);
        b->align( FL_ALIGN_LEFT );
        b->color( p.ui->uiPenColor->color() );
        b->labelsize(11);
        b->callback( (Fl_Callback*)set_pen_color_cb, p.ui );
        
        pg->resizable(0);
        pg->end();
        
        sV = new Widget< HorSlider >( X, Y+40, g->w(), 20, _("Pen Size:") );
        s = sV;
        s->range( 1, 50 );
        s->step( 1 );
        s->tooltip( _("Selects the current pen size.") );
        value = settingsObject->value( kPenSize );
        s->default_value( std_any_empty( value ) ? 20 :
                          std_any_cast< int >( value ) );
	settingsObject->setValue( kPenSize, (int) s->value() );
        sV->callback( [=]( auto o ) {
            settingsObject->setValue( kPenSize, (int) o->value() );
            const auto& renderSize =
                p.ui->uiView->getRenderSize();
            float pct = renderSize.h / 1024.F;
            MultilineInput* w = p.ui->uiView->getMultilineInput();
            if (!w) return;
            int fontSize = o->value() * pct *
                           p.ui->uiView->viewZoom();
            w->textsize( fontSize );
            w->redraw();
            p.ui->uiView->redrawWindows();
        } );

        cg->end();
        
        cg = new CollapsibleGroup( X, Y, g->w(), 20, _("Ghosting" ) );
        b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);

        cg->begin();

        sg = new Pack( X, Y, g->w(), 25 );
        sg->type( Pack::HORIZONTAL );
        sg->spacing(5);
        sg->begin();
        
        box = new Fl_Box( X, Y, 120, 20, _("Previous:" ) );
        box->labelsize(12);
        box->align( FL_ALIGN_LEFT | FL_ALIGN_INSIDE );
        
        auto dV = new Widget< DoubleSpinner >( X, 40, 50, 20 );
        d = dV;
        d->range( 1, 50 );
        d->step( 1 );
        d->tooltip( _("Selects the number of fading frames previous to the frame of the annotation.") );
        value = settingsObject->value( kGhostPrevious );
        d->value( std_any_empty( value ) ? 5 : std_any_cast< int >( value ) );
        dV->callback( [=]( auto w ) {
            settingsObject->setValue( kGhostPrevious, (int) w->value() );
            p.ui->uiView->setGhostPrevious( (int) w->value() );
            p.ui->uiView->redrawWindows();
        } );
        sg->end();

        sg = new Pack( X, Y, g->w(), 25 );
        sg->type( Pack::HORIZONTAL );
        sg->spacing(5);
        sg->begin();
        
        box = new Fl_Box( X, Y, 120, 20, _("Next:" ) );
        box->labelsize(12);
        box->align( FL_ALIGN_LEFT | FL_ALIGN_INSIDE );
        dV = new Widget< DoubleSpinner >( X, 60, 50, 20 );
        d = dV;
        d->range( 1, 50 );
        d->step( 1 );
        d->tooltip( _( "Selects the number of fading frames following the frame of the annotation.") );
        value = settingsObject->value( kGhostNext );
        d->value( std_any_empty( value ) ? 5 : std_any_cast< int >( value ) );
        dV->callback( [=]( auto w ) {
            settingsObject->setValue( kGhostNext, (int) w->value() );
            p.ui->uiView->setGhostNext( (int) w->value() );
            p.ui->uiView->redrawWindows();
        } );
        
        sg->end();
        
        cg->end();


        
        cg = new CollapsibleGroup( X, 20, g->w(), 20, _("Frames" ) );
        b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);

        cg->begin();
        
        auto rV = new Widget< Fl_Radio_Round_Button >( X, 40, 50, 20,
                                                       _("Current" ) );
        r = rV;
        value = settingsObject->value( kAllFrames );
        r->tooltip( _("Makes the following annotation "
                      "show on this frame only."));
        r->value( std_any_empty( value ) ? 1 : !std_any_cast< int >( value ) );
        rV->callback( [=]( auto w ) {
            settingsObject->setValue( kAllFrames, (int) !w->value() );
        } );

        
        rV = new Widget< Fl_Radio_Round_Button >( X, 40, 50, 20, _( "All" ) );
        r = rV;
        r->tooltip( _("Makes the following annotation "
                      "show on all frames."));
        value = settingsObject->value( kAllFrames );
        r->value( std_any_empty( value ) ? 0 : std_any_cast< int >( value ) );
        rV->callback( [=]( auto w ) {
            settingsObject->setValue( kAllFrames, (int) w->value() );
        } );
        
        cg->end();
    }

    void AnnotationsTool::redraw()
    {
        TLRENDER_P();
        penColor->color( p.ui->uiPenColor->color() );
        penColor->redraw();
    }

}
