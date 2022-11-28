// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once


#include "mrvToolWidget.h"

class ViewerUI;
class Fl_Button;

namespace mrv
{
    namespace
    {
        const char* kTextFont  = "Annotations/Text Font";
        const char* kPenColorR = "Annotations/Pen Color R";
        const char* kPenColorG = "Annotations/Pen Color G";
        const char* kPenColorB = "Annotations/Pen Color B";
        const char* kPenSize   = "Annotations/Pen Size";
        const char* kFontSize  = "Annotations/Font Size";
        const char* kGhostPrevious = "Annotations/Ghost Previous";
        const char* kGhostNext = "Annotations/Ghost Next";
        const char* kAllFrames = "Annotations/All Frames";
    }

    using namespace tl;
    
    class AnnotationsTool : public ToolWidget
    {
        Fl_Button* penColor = nullptr;
    public:
        AnnotationsTool( ViewerUI* ui );
        virtual ~AnnotationsTool() {};

        void add_controls() override;

        void redraw();
    };


} // namespace mrv
