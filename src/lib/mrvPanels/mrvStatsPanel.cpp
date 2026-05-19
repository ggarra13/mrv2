// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrViewer.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrvPanels/mrvPanelsCallbacks.h"
#include "mrvPanels/mrvStatsPanel.h"

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvHorSlider.h"
#include "mrvWidgets/mrvEEGGraph.h"

// #include "mrvIcons/Stats.h"

#include "tlCore/StatsSystem.h"

#include "FL/Fl_Box.H"
#include "FL/Fl_Flex.H"

namespace mrv
{
    namespace panel
    {
        namespace
        {
            void timerEvent_cb(void* data)
            {
                StatsPanel* p = (StatsPanel*) data;
                p->tick();
            }
        }
        
        struct StatsPanel::Private
        {
            std::shared_ptr<system::StatsSystem> stats;
            std::map<std::string, EEGGraph*> widget;
            std::shared_ptr<observer::MapObserver<std::string, int64_t> > sampleIncObserver;
        };

        StatsPanel::StatsPanel(ViewerUI* ui) :
            _r(new Private),
            PanelWidget(ui)
        {

            //Fl_SVG_Image* svg = MRV2_LOAD_SVG(Stats);
            //g->bind_image(svg);

            const auto context = App::app->getContext();
            _r->stats = context->getSystem<system::StatsSystem>();
            
            add_group("Statistics");

            g->callback(
                [](Fl_Widget* w, void* d)
                {
                    ViewerUI* ui = static_cast< ViewerUI* >(d);
                    delete statsPanel;
                    statsPanel = nullptr;
                    ui->uiMain->fill_menu(ui->uiMenuBar);
                },
                ui);

            Fl::add_timeout(3.0, (Fl_Timeout_Handler) timerEvent_cb, this);
        }

        StatsPanel::~StatsPanel()
        {
            Fl::remove_timeout((Fl_Timeout_Handler) timerEvent_cb, this);
        }

        void StatsPanel::add_controls()
        {
            TLRENDER_P();
            MRV2_R();

            SettingsObject* settings = App::app->settings();
            
            Pack* pack = g->get_pack();
            pack->spacing(5);

            g->clear();
            g->begin();

            int X = g->x();
            int Y = g->y() + 20;
            int W = g->w();

            Fl_Flex* cg;

            auto groups = _r->stats->getGroups();
            
            cg = new Fl_Flex(X, Y, W, groups.size() * 130);
            cg->type(Fl_Flex::VERTICAL);
            
            cg->gap(40);
            cg->margin(5, 20, 5, 10);
            cg->begin();

            for (auto group : groups)
            {
                EEGGraph* w = new EEGGraph(0, 0, 30, 30);
                w->align(FL_ALIGN_LEFT | FL_ALIGN_TOP);
                w->copy_label(group.c_str());
                _r->widget[group] = w;
            }

            
            _r->sampleIncObserver =
                observer::MapObserver<std::string, int64_t>::create(
                    _r->stats->observeSamplesInc(),
                    [this](const std::map<std::string, int64_t>& value)
                        {
                            MRV2_R();
                            
                            for (const auto& i : value)
                            {
                                const auto& keys = string::split(i.first, '/');
                                const auto j = r.widget.find(keys[0]);
                                if (j != r.widget.end())
                                {
                                    j->second->push_sample(keys[1], i.second);
                                }
                            }
                        });
                
            cg->end();
            cg->layout();

            
            g->resizable(g);
        }

        void StatsPanel::tick()
        {
            MRV2_R();
            r.stats->tick();
            Fl::repeat_timeout(3.0, (Fl_Timeout_Handler) timerEvent_cb, this);
        }

    } // namespace panel
} // namespace mrv
