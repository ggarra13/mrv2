#pragma once

#include <memory>

#include "tlCore/Util.h"

class ViewerUI;

namespace mrv
{
    class DockGroup;

    void color_tool_grp( Fl_Widget* w, ViewerUI* ui );

    class ColorTool
    {
    public:
        ColorTool( ViewerUI* ui );
        
    private:
        TLRENDER_PRIVATE();
    };

} // namespace mrv
