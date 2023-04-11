// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvWidgets/mrvFunctional.h"

#include "mrvPanelsCallbacks.h"

#include "mrViewer.h"

namespace mrv
{

    struct NetworkPanel::Private
    {
    };

    NetworkPanel::NetworkPanel(ViewerUI* ui) :
        _r(new Private),
        PanelWidget(ui)
    {
        add_group("Network");

        // Fl_SVG_Image* svg = load_svg("Network.svg");
        // g->image(svg);

        g->callback(
            [](Fl_Widget* w, void* d)
            {
                ViewerUI* ui = static_cast< ViewerUI* >(d);
                delete networkPanel;
                networkPanel = nullptr;
                ui->uiMain->fill_menu(ui->uiMenuBar);
            },
            ui);
    }

    NetworkPanel::~NetworkPanel() {}

    void NetworkPanel::add_controls()
    {
        TLRENDER_P();

        g->clear();

        g->begin();

        g->end();
    }

} // namespace mrv
