#pragma once


#include "mrvToolWidget.h"

class ViewerUI;

namespace mrv
{
    namespace
    {
        const char* kTextFont = "Annotations/Text Font";
        const char* kPenColor = "Annotations/Pen Color";
        const char* kPenSize  = "Annotations/Pen Size";
        const char* kGhostPrevious = "Annotations/Ghost Previous";
        const char* kGhostNext = "Annotations/Ghost Next";
        const char* kAllFrames = "Annotations/All Frames";
    }

    using namespace tl;
    
    class AnnotationsTool : public ToolWidget
    {
    public:
        AnnotationsTool( ViewerUI* ui );
        virtual ~AnnotationsTool() {};

        void add_controls() override;
        
    };


} // namespace mrv
