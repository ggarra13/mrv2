#pragma once


#include "mrvToolWidget.h"

class ViewerUI;

namespace mrv
{
    using namespace tl;
    
    class SettingsTool : public ToolWidget
    {
    public:
        SettingsTool( ViewerUI* ui );
        virtual ~SettingsTool() {};

        void add_controls() override;
        
    };


} // namespace mrv
