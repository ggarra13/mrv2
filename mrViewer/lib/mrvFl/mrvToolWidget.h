#pragma once

#include <iostream>
#include <memory>
#include <tlCore/Util.h>

#include "mrvToolGroup.h"

class ViewerUI;

namespace mrv
{
    
    class ToolWidget
    {
    protected:
        ToolGroup* g = nullptr;
    public:
        ToolWidget( ViewerUI* ui );
        virtual ~ToolWidget() {};

        void add_group( const char* label );
        
        virtual void add_controls() = 0;
        
        TLRENDER_PRIVATE();
    };

    struct ToolWidget::Private
    {
        ViewerUI* ui;
    };

} // namespace mrv
