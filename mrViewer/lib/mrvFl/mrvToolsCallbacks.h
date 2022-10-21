#pragma once

#include <memory>

#include "tlCore/Util.h"

class ViewerUI;
class Fl_Check_Button;

namespace mrv
{
    class DockGroup;

    void color_tool_grp( Fl_Widget* w, ViewerUI* ui );

    class ColorTool
    {
        Fl_Check_Button* colorOn;
        Fl_Check_Button* levelsOn;
        Fl_Check_Button* softClipOn;
    public:
        ColorTool( ViewerUI* ui );
    };

} // namespace mrv
