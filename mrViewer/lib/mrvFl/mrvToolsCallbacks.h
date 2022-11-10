#pragma once

#include <memory>
#include <vector>

#include "tlCore/Util.h"

#include "mrvFilesTool.h"
#include "mrvColorTool.h"
#include "mrvCompareTool.h"
#include "mrvSettingsTool.h"
#include "mrvLogsTool.h"

class ViewerUI;
class Fl_Widget;

namespace mrv
{
    
    extern ColorTool*    colorTool;
    extern FilesTool*     filesTool;
    extern CompareTool*  compareTool;
    extern SettingsTool* settingsTool;
    extern LogsTool*         logsTool;
    
    void color_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void files_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void compare_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void settings_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void logs_tool_grp( Fl_Widget* w, ViewerUI* ui );
    
} // namespace mrv
