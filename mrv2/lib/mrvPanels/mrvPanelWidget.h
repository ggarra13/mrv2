// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <iostream>
#include <memory>

#include <tlCore/Box.h>
#include <tlCore/Util.h>

#include <FL/Fl_Widget.H>
#include <FL/Fl_SVG_Image.H>

#include "mrvWidgets/mrvPanelGroup.h"

#include "mrvUI/mrvUtil.h"

#include "mrvCore/mrvI8N.h"

//! Define a variable, "r", that references the private implementation.
#define MRV2_R() auto& r = *_r
#define MRV2_PRIVATE()                                                         \
    struct Private;                                                            \
    std::unique_ptr<Private> _r;

class ViewerUI;

namespace mrv
{
    using namespace tl;

    class PanelWidget
    {
    protected:
        PanelGroup* g = nullptr;
        std::string label;

    public:
        PanelWidget(ViewerUI* ui);
        virtual ~PanelWidget();

        virtual void add_group(const char* label);
        void begin_group();
        virtual void end_group();

        void clear_controls();
        void refresh();

        tl::math::Box2i box() const;

        bool is_panel() const { return g->docked(); };
        virtual void save();

        virtual void dock();
        virtual void undock();

        virtual void add_static_controls(){};
        virtual void add_controls() = 0;

        std::string tab_prefix() const { return "gui/" + label + "/Tab/"; }

        TLRENDER_PRIVATE();
    };

    struct PanelWidget::Private
    {
        ViewerUI* ui;
    };

} // namespace mrv
