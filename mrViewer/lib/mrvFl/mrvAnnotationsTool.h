// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once


#include "mrvToolWidget.h"

class ViewerUI;
class Fl_Button;

namespace mrv
{
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
