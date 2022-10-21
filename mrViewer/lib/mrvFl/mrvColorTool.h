#pragma once

#include <vector>

class ViewerUI;
class Fl_Check_Button;
class Fl_Widget;

namespace mrv
{
    
    class ColorTool
    {
        Fl_Check_Button* colorOn;
        Fl_Check_Button* levelsOn;
        Fl_Check_Button* softClipOn;
        std::vector< Fl_Widget* > widgets;
    public:
        ColorTool( ViewerUI* ui );

        void refresh() noexcept;
    };


} // namespace mrv
