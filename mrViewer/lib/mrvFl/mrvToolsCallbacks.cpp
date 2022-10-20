
#include <FL/Fl_Check_Button.H>

#include "mrvFl/mrvDockGroup.h"
#include "mrvFl/mrvToolsCallbacks.h"
#include "mrvFl/mrvToolGroup.h"
#include "mrvFl/mrvHorSlider.h"
#include "mrvFl/mrvCollapsibleGroup.h"

#include "mrViewer.h"

namespace mrv
{
    void color_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        DockGroup* dock = ui->uiDock;
        ResizableBar* bar = ui->uiResizableBar;
        Fl_Group* dg = ui->uiDockGroup;
        ToolGroup *g = new ToolGroup(dock, 0, dock->x(), dock->y(),
                                     dg->w()-bar->w(), 390, "Color");
        g->begin();

        CollapsibleGroup* cg = new CollapsibleGroup( g->x(), 20, g->w(), 20,
                                                     "Color Controls" );
        Fl_Button* b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);
        cg->begin();
        
        Fl_Check_Button* c = new Fl_Check_Button( g->x()+90, 50, g->w(), 20,
                                                 "Enabled" );
        c->labelsize(12);
        
        mrv::HorSlider* s = new mrv::HorSlider( g->x(), 70, g->w(), 20, "Add" );
        s->setDefaultValue( 0.0f );
        
        s = new mrv::HorSlider( g->x(), 80, g->w(), 20, "Brightness" );
        s->setRange( 0.f, 4.0f );
        s->setDefaultValue( 1.0f );
        
        s = new mrv::HorSlider( g->x(), 100, g->w(), 20, "Contrast" );
        s->setRange( 0.f, 4.0f );
        s->setDefaultValue( 1.0f );
        
        s = new mrv::HorSlider( g->x(), 120, g->w(), 20, "Saturation" );
        s->setRange( 0.f, 4.0f );
        s->setDefaultValue( 1.0f );
        
        s = new mrv::HorSlider( g->x(), 140, g->w(), 20, "Tint" );
        s->setDefaultValue( 0.0f );
        
        c = new Fl_Check_Button( g->x()+90, 160, g->w(), 20, "Invert" );
        c->labelsize(12);

        cg->end();

        cg = new CollapsibleGroup( g->x(), 180, g->w(), 20, "Levels" );
        b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);
        cg->layout();

        cg->begin();

        s = new mrv::HorSlider( g->x(), 140, g->w(), 20, "In Low" );
        s->setDefaultValue( 0.0f );
        
        s = new mrv::HorSlider( g->x(), 140, g->w(), 20, "In High" );
        s->setDefaultValue( 1.0f );
        
        s = new mrv::HorSlider( g->x(), 140, g->w(), 20, "Out Low" );
        s->setDefaultValue( 0.0f );
        
        s = new mrv::HorSlider( g->x(), 140, g->w(), 20, "Out High" );
        s->setDefaultValue( 1.0f );
        
        cg->end();
        cg = new CollapsibleGroup( g->x(), 180, g->w(), 20, "Soft Clip" );
        b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);
        cg->layout();

        cg->begin();

        c = new Fl_Check_Button( g->x()+90, 160, g->w(), 20, "Enabled" );
        c->labelsize(12);
        
        s = new mrv::HorSlider( g->x(), 140, g->w(), 20, "Soft Clip" );
        s->setDefaultValue( 0.0f );
        
        
        
        cg->end();
        
        g->end();
        g->box( FL_FLAT_BOX );
        g->redraw();
    }
}
