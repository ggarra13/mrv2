#include <iostream>

#include "mrvToolsCallbacks.h"

namespace mrv {

    ColorTool*     colorTool = nullptr;
    ReelTool*       reelTool = nullptr;
    CompareTool* compareTool = nullptr;
    
    void color_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( colorTool ) return;
        colorTool = new ColorTool( ui );
    }

    void reel_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( reelTool ) return;
        reelTool = new ReelTool( ui );
    }
    
    void compare_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( compareTool ) return;
        compareTool = new CompareTool( ui );
    }
}
