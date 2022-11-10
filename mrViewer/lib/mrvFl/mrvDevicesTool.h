#pragma once


#include "mrvToolWidget.h"

class ViewerUI;

namespace mrv
{
    class DevicesTool : public ToolWidget
    {
    public:
        DevicesTool( ViewerUI* ui );
        ~DevicesTool();

        void add_controls() override;

        
    private:
        struct Private;
        std::unique_ptr<Private> _r;
    };


} // namespace mrv
