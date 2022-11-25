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
    
    class HistogramTool : public ToolWidget
    {
    public:
        HistogramTool( ViewerUI* ui );
        ~HistogramTool();

        void add_controls() override;


        void update( const area::Info& info );

        
    private:
        struct Private;
        std::unique_ptr<Private> _r;
    };


} // namespace mrv
