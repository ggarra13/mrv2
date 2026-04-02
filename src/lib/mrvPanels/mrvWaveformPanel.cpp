// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrViewer.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrvPanels/mrvPanelsCallbacks.h"
#include "mrvPanels/mrvWaveformPanel.h"

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvHorSlider.h"
#include "mrvWidgets/mrvWaveform.h"

#include "mrvIcons/Waveform.h"

#include "mrvCore/mrvColorAreaInfo.h"

#include "FL/Fl_Check_Button.H"

namespace mrv
{
    namespace panel
    {
        struct WaveformPanel::Private
        {
            Waveform* waveform = nullptr;
            HorSlider* hdrMaxValue = nullptr;
            Fl_Check_Button* hdrLogScale = nullptr;
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

            SettingsObject* settings = App::app->settings();
            
            Pack* pack = g->get_pack();
            pack->spacing(5);

            g->clear();
            g->begin();

            int X = g->x();
            int Y = g->y();
            int W = g->w();

            std::string key;
            Fl_Group* cg;
            Fl_Box* b;
            Fl_Choice* c;
            HorSlider* s;
            cg = new Fl_Group(X, Y, W, 20);
            cg->begin();
            b = new Fl_Box(X, Y, 120, 20, _("Type"));
            auto cW = new Widget< Fl_Choice >(X + b->w(), Y, W - b->w(), 20);
            c = cW;
            c->add("SDR");
#ifdef VULKAN_BACKEND
            c->add("HDR");
            c->value(1);
#endif

#ifdef OPENGL_BACKEND
            c->value(0);
#endif
            c->tooltip(_("Type of Waveform Monitor."));
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

            auto sV = new Widget< HorSlider >(
                g->x(), 90, g->w(), 20, _("Max. Value"));
            r.hdrMaxValue = s = sV;
            s->tooltip(_("Maximum Value in F-Stops."));
            s->range(1.f, 30.f);
            s->step(0.1);

            // Get current value
            key = "Waveform/HDRMaxValue";

            std_any any = settings->getValue<std_any>(key);
            float v = std_any_empty(any) ? 12.F : std_any_cast<float>(any);
            
            s->default_value(12.f);
            s->value(v);
            sV->callback(
                [=](auto w)
                {
                    float value = w->value();
                    settings->setValue("Waveform/HDRMaxValue", value);
                    _r->waveform->setHDRMaxValue(value);
                });
            
            // Get current value
            key = "Waveform/HDRLogScale";
            any = settings->getValue<std_any>(key);
            bool bV = std_any_empty(any) ? true : std_any_cast<bool>(any);

            auto cB = new Widget< Fl_Check_Button >(
                g->x(), 90, g->w(), 20, _("Log Scale"));
            cB->tooltip(_("Use a Log Scale in HDR."));
            cB->value(bV);
            r.hdrLogScale = cB;
            cB->callback(
                [=](auto w)
                {
                    bool value = w->value();
                    settings->setValue("Waveform/HDRLogScale", value);
                    _r->waveform->setHDRLogScale(value);
                });
                            
            g->resizable(r.waveform);
        }

        void WaveformPanel::update(const area::Info& info)
        {
            _r->waveform->update(info);
        }

    } // namespace panel
} // namespace mrv
