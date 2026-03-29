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
            int W = g->w();

            Fl_Group* cg;
            Fl_Box* b;
            Fl_Choice* c;
            cg = new Fl_Group(X, Y, W, 20);
            cg->begin();
            b = new Fl_Box(X, Y, 120, 20, _("Type"));
            auto cW = new Widget< Fl_Choice >(X + b->w(), Y, W - b->w(), 20);
            c = cW;
            c->add("SDR");
            c->add("HDR");
            c->value(1);
            cW->callback(
                [=](auto o)
                {
                    bool isSDR = (o->value() == 0);
                    if (isSDR)
                    {
                        _r->waveform->setHDRMode(false);
                    }
                    else
                    {
                        _r->waveform->setHDRMode(true);
                    }
                });
            cg->end();
            
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
