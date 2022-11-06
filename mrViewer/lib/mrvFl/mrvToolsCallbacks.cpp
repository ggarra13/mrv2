#include <iostream>

#include "mrvToolsCallbacks.h"

namespace mrv {

    ColorTool*     colorTool = nullptr;
    FilesTool*       filesTool = nullptr;
    CompareTool* compareTool = nullptr;
    SettingsTool*     settingsTool = nullptr;
    
    void color_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( colorTool ) return;
        colorTool = new ColorTool( ui );
    }

    void files_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( filesTool ) return;
        filesTool = new FilesTool( ui );
    }
    
    void compare_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( compareTool ) return;
        compareTool = new CompareTool( ui );
    }
    
    void settings_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( settingsTool ) return;
        settingsTool = new SettingsTool( ui );
    }
}
