// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvGL/mrvGLViewport.h"

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvColorInfo.h"
#include "mrvWidgets/mrvHorSlider.h"

#include "mrvPanelsCallbacks.h"

#include "mrvFl/mrvColorAreaInfo.h"

#include "mrvPanels/mrvColorAreaPanel.h"

#include "mrViewer.h"

#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "carea";
}

namespace mrv
{
    namespace panel
    {
        struct ColorAreaPanel::Private
        {
            ColorInfo* colorInfo = nullptr;
        };

        ColorAreaPanel::ColorAreaPanel(ViewerUI* ui) :
            _r(new Private),
            PanelWidget(ui)
        {
            add_group("Color Area");

            Fl_SVG_Image* svg = load_svg("ColorArea.svg");
            g->image(svg);

            g->callback(
                [](Fl_Widget* w, void* d)
                {
                    ViewerUI* ui = static_cast< ViewerUI* >(d);
                    delete colorAreaPanel;
                    colorAreaPanel = nullptr;
                    ui->uiMain->fill_menu(ui->uiMenuBar);
                },
                ui);
        }

        ColorAreaPanel::~ColorAreaPanel() {}

        void ColorAreaPanel::add_controls()
        {
            TLRENDER_P();
            MRV2_R();

            g->clear();
            g->begin();

            int X = g->x();
            int Y = g->y() + 20;

            r.colorInfo = new ColorInfo(X, Y, g->w(), 280);
            r.colorInfo->main(p.ui);

            g->end();
        }

        void ColorAreaPanel::update(const area::Info& info)
        {
            MRV2_R();
            r.colorInfo->update(info);
        }

    } // namespace panel
} // namespace mrv
