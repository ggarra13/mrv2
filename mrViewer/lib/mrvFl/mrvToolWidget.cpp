
#include "mrvFl/mrvDockGroup.h"
#include "mrvFl/mrvResizableBar.h"
#include "mrvFl/mrvToolGroup.h"

#include "mrvFl/mrvToolWidget.h"

#include "mrViewer.h"

namespace mrv
{
    
    
    ToolWidget::ToolWidget( ViewerUI* ui ) :
        _p( new Private )
    {
        _p->ui = ui;
    }

    void ToolWidget::add_group( const char* label )
    {
        TLRENDER_P();
        
        Fl_Group* dg = p.ui->uiDockGroup;
        ResizableBar* bar = p.ui->uiResizableBar;
        DockGroup* dock = p.ui->uiDock;
        g = new ToolGroup(dock, 0, dock->x(), dock->y(),
                          dg->w()-bar->w(), 10, label );
        
        g->begin();

        add_controls();
        
        g->end();
        g->box( FL_FLAT_BOX );
    }

}
