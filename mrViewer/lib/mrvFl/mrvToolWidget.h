#pragma once

#include <iostream>
#include <memory>
#include <tlCore/Util.h>

#include <FL/Fl_Widget.H>
#include <FL/Fl_SVG_Image.H>

#include "mrvCore/mrvI8N.h"

#include "mrvWidgets/mrvToolGroup.h"

#include <mrvPlayApp/App.h>

class ViewerUI;

namespace mrv
{
    class ToolWidget
    {
    protected:
        ToolGroup* g = nullptr;
        Fl_SVG_Image* svg = nullptr;
        std::string svg_root;
    public:
        ToolWidget( ViewerUI* ui );
        virtual ~ToolWidget();

        void add_group( const char* label );
        void end_group();

        void save();
        
        virtual void add_controls() = 0;
        
        TLRENDER_PRIVATE();
    };

    struct ToolWidget::Private
    {
        ViewerUI* ui;
    };

} // namespace mrv
