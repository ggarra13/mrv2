#pragma once

#include <iostream>
#include <memory>
#include <tlCore/Util.h>

#include <FL/Fl_Widget.H>
#include <FL/Fl_SVG_Image.H>

#include "mrvWidgets/mrvToolGroup.h"

#include "mrvFl/mrvUtil.h"

#include "mrvPlayApp/App.h"

#include "mrvCore/mrvI8N.h"

//! Define a variable, "r", that references the private implementation.
#define TLRENDER_R()                           \
    auto& r = *_r

class ViewerUI;

namespace mrv
{
    
    class ToolWidget
    {
    protected:
        ToolGroup* g = nullptr;
      
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
