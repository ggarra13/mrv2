#include <iostream>

#include "mrvToolsCallbacks.h"

namespace mrv {

    ColorTool*       colorTool = nullptr;
    FilesTool*       filesTool = nullptr;
    CompareTool*   compareTool = nullptr;
    SettingsTool* settingsTool = nullptr;
    LogsTool*         logsTool = nullptr;
    DevicesTool*   devicesTool = nullptr;
    ColorAreaTool*  colorAreaTool = nullptr;
    
    void color_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( colorTool )
        {
            delete colorTool; colorTool = nullptr;
            return;
        }
        colorTool = new ColorTool( ui );
    }

    void files_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( filesTool )
        {
            delete filesTool; filesTool = nullptr;
            return;
        }
        filesTool = new FilesTool( ui );
    }
    
    void compare_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( compareTool )
        {
            delete compareTool; compareTool = nullptr;
            return;
        }
        compareTool = new CompareTool( ui );
    }
    
    void settings_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( settingsTool )
        {
            delete settingsTool; settingsTool = nullptr;
            return;
        }
        settingsTool = new SettingsTool( ui );
    }
    
    void logs_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( logsTool )
        {
            delete logsTool; logsTool = nullptr;
            return;
        }
        logsTool = new LogsTool( ui );
    }
    
    void devices_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( devicesTool )
        {
            delete devicesTool; devicesTool = nullptr;
            return;
        }
        devicesTool = new DevicesTool( ui );
    }
    
    void color_area_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( colorAreaTool )
        {
            delete colorAreaTool; colorAreaTool = nullptr;
            return;
        }
        colorAreaTool = new ColorAreaTool( ui );
    }
}
