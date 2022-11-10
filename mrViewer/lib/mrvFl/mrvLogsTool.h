#pragma once

#include <vector>

#include <FL/Fl_Button.H>

#include "mrvToolWidget.h"

class ViewerUI;

namespace mrv
{
    using namespace tl;

    class ClipButton;
    
    class LogsTool : public ToolWidget
    {
    public:
        LogsTool( ViewerUI* ui );
        ~LogsTool();

        void add_controls() override;

        
    private:
        struct Private;
        std::unique_ptr<Private> _r;
    };


} // namespace mrv
