// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrViewer.h"

#include "mrvCore/mrvColorAreaInfo.h"

#include "mrvIcons/Waveform.h"

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvWaveform.h"

#include "mrvPanels/mrvPanelsCallbacks.h"
#include "mrvPanels/mrvWaveformPanel.h"

namespace mrv
{
    namespace panel
    {
        struct WaveformPanel::Private
        {
            Waveform* waveform = nullptr;
        };

        WaveformPanel::WaveformPanel(ViewerUI* ui) :
            _r(new Private),
            PanelWidget(ui)
        {
            add_group("Waveform");

            Fl_SVG_Image* svg = MRV2_LOAD_SVG(Waveform);
            g->bind_image(svg);

            g->callback(
                [](Fl_Widget* w, void* d)
                {
                    ViewerUI* ui = static_cast< ViewerUI* >(d);
                    delete waveformPanel;
                    waveformPanel = nullptr;
                    ui->uiMain->fill_menu(ui->uiMenuBar);
                },
                ui);
        }

        WaveformPanel::~WaveformPanel() {}

        void WaveformPanel::add_controls()
        {
            TLRENDER_P();
            MRV2_R();

            Pack* pack = g->get_pack();
            pack->spacing(5);

            g->clear();
            g->begin();

            int X = g->x();
            int Y = g->y();

            // Create a rectangular waveform
            r.waveform = new Waveform(X, Y, g->w(), g->w());
            r.waveform->main(p.ui);

            g->resizable(r.waveform);
        }

        void WaveformPanel::update(const area::Info& info)
        {
            _r->waveform->update(info);
        }

    } // namespace panel
} // namespace mrv
