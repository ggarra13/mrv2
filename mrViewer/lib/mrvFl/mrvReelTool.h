#pragma once

#include "mrvToolWidget.h"

class ViewerUI;

namespace mrv
{
    
    class ReelTool : public ToolWidget
    {
    public:
        ReelTool( ViewerUI* ui );
        
        void add_controls() override;
    private:
        struct Private;
        std::unique_ptr<Private> _r;
    };


} // namespace mrv
