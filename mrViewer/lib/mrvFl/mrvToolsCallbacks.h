#pragma once

#include <memory>
#include <vector>

#include "tlCore/Util.h"

#include "mrvColorTool.h"

class ViewerUI;
class Fl_Widget;

namespace mrv
{
    
    static ColorTool* colorTool = nullptr;
    
    void color_tool_grp( Fl_Widget* w, ViewerUI* ui );
    
} // namespace mrv
