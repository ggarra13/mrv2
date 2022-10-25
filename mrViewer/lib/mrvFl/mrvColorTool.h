#pragma once

#include <vector>

#include "mrvToolWidget.h"

class ViewerUI;
class Fl_Input;
class Fl_Check_Button;
class Fl_Widget;

namespace mrv
{
    
    class ColorTool : public ToolWidget
    {
        Fl_Check_Button* colorOn;
        Fl_Check_Button* levelsOn;
        Fl_Check_Button* softClipOn;
        Fl_Input*        lutFilename;
        std::vector< Fl_Widget* > widgets;
    public:
        ColorTool( ViewerUI* ui );
        void refresh() noexcept;

        virtual void add_controls() override;
        
    };


} // namespace mrv
