#pragma once


#include "mrvToolWidget.h"

#include "mrvFl/mrvColorAreaInfo.h"

class ViewerUI;

namespace mrv
{
    namespace area
    {
        class Info;
    }
    
    class ColorAreaTool : public ToolWidget
    {
    public:
        ColorAreaTool( ViewerUI* ui );
        ~ColorAreaTool();

        void add_controls() override;


        void update( const area::Info& info );

        
    private:
        struct Private;
        std::unique_ptr<Private> _r;
    };


} // namespace mrv
