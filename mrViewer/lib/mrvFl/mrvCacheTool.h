#pragma once


#include <tlCore/Time.h>

#include "mrvToolWidget.h"

class ViewerUI;

namespace mrv
{
    using namespace tl;
    
    class CacheTool : public ToolWidget
    {
    public:
        CacheTool( ViewerUI* ui );
        virtual ~CacheTool() {};

        void add_controls() override;
        
    };


} // namespace mrv
