

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
        
        ToolGroup *tgroup = new ToolGroup(dock, 0, dock->w(), 160);
        tgroup->begin();
        mrv::CollapsibleGroup* g = new mrv::CollapsibleGroup( tgroup->x(), 20, tgroup->w(), 120, "Video" );
        g->button()->labelsize( 14 );
        g->begin();
        mrv::HorSlider* s = new mrv::HorSlider( g->x(), g->y()+40,
                                                g->w(), 20, "Gain" );
        s->setRange( 0.f, 10.0f );
        s->setStep( 0.1f );
        s->setDefaultValue( 1.0f );
        s = new mrv::HorSlider( g->x(), g->y()+60, g->w(), 20, "Brightness" );
        s->setRange( 0.f, 10.0f );
        s->setStep( 0.1f );
        s->setDefaultValue( 1.0f );
        g->end();
        tgroup->end();
        tgroup->box(FL_BORDER_BOX);
    }
}
