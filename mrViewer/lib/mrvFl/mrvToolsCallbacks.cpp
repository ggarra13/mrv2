
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
                                     dg->w()-bar->w(), 300, "Color");
        g->begin();

        CollapsibleGroup* cg = new CollapsibleGroup( g->x(), 20, g->w(), 20,
                                                     "Levels" );
        Fl_Button* b = cg->button();
        b->labelsize(12);
        b->size(b->w(), 14);
        cg->begin();
        
        Fl_Check_Button* c = new Fl_Check_Button( g->x(), 40, g->w(), 20,
                                                 "Enabled" );
        c->labelsize(12);
        
        mrv::HorSlider* s = new mrv::HorSlider( g->x(), 60, g->w(), 20, "Add" );
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
        
        c = new Fl_Check_Button( g->x(), 160, g->w(), 20, "Invert" );
        c->labelsize(12);

        cg->end();

        cg = new CollapsibleGroup( g->x(), 180, g->w(), 20, "Exposure" );
        b = cg->button();
        b->labelsize(12);
        b->size(b->w(), 14);
        cg->layout();

        cg->begin();
        cg->end();
        
        g->end();
        g->box( FL_FLAT_BOX );
        g->redraw();
    }
}
