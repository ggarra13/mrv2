#pragma once


#include "mrvToolWidget.h"

class ViewerUI;

namespace mrv
{
    class ColorAreaTool : public ToolWidget
    {
    public:
        ColorAreaTool( ViewerUI* ui );
        ~ColorAreaTool();

        void add_controls() override;

        
    private:
        struct Private;
        std::unique_ptr<Private> _r;
    };


} // namespace mrv
