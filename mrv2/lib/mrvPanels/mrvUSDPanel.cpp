// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#if defined(TLRENDER_USD)

#    include <tlCore/StringFormat.h>

#    include <FL/Fl_Check_Button.H>
#    include <FL/Fl_Choice.H>
#    include <FL/Fl_Spinner.H>

#    include "mrvCore/mrvUSD.h"

#    include "mrvWidgets/mrvFunctional.h"
#    include "mrvWidgets/mrvHorSlider.h"
#    include "mrvWidgets/mrvCollapsibleGroup.h"

#    include "mrvPanels/mrvPanelsCallbacks.h"
#    include "mrvPanels/mrvUSDPanel.h"

#    include "mrvFl/mrvIO.h"

#    include "mrvApp/mrvSettingsObject.h"

#    include "mrViewer.h"

namespace
{
    const char* kModule = "usd";
}

namespace mrv
{
    namespace panel
    {

        USDPanel::USDPanel(ViewerUI* ui) :
            PanelWidget(ui)
        {
            add_group("USD");

            Fl_SVG_Image* svg = load_svg("USD.svg");
            g->image(svg);

            g->callback(
                [](Fl_Widget* w, void* d)
                {
                    ViewerUI* ui = static_cast< ViewerUI* >(d);
                    delete usdPanel;
                    usdPanel = nullptr;
                    ui->uiMain->fill_menu(ui->uiMenuBar);
                },
                ui);
        }

        void USDPanel::add_controls()
        {
            TLRENDER_P();

            SettingsObject* settings = App::app->settings();
            const std::string& prefix = tab_prefix();

            Fl_Group* bg;
            Fl_Spinner* sp;
            std_any value;
            int open;

            int Y = g->y();

            auto cg = new CollapsibleGroup(g->x(), Y, g->w(), 20, _("USD"));
            cg->spacing(2);
            auto b = cg->button();
            b->labelsize(14);
            b->size(b->w(), 18);
            b->callback(
                [](Fl_Widget* w, void* d)
                {
                    CollapsibleGroup* cg = static_cast<CollapsibleGroup*>(d);
                    if (cg->is_open())
                        cg->close();
                    else
                        cg->open();

                    const std::string& prefix = usdPanel->tab_prefix();
                    const std::string key = prefix + "USD";

                    auto settings = App::app->settings();
                    settings->setValue(key, static_cast<int>(cg->is_open()));

                    usdPanel->refresh();
                },
                cg);

            cg->begin();

            Y += 30;
            bg = new Fl_Group(g->x(), Y, g->w(), 22 * 8);
            bg->box(FL_NO_BOX);
            bg->begin();

            auto mW = new Widget< Fl_Choice >(
                g->x() + 130, Y, g->w() - 130, 20, _("Renderer"));
            Fl_Choice* m = mW;
            m->labelsize(12);
            m->align(FL_ALIGN_LEFT);

            auto player = p.ui->uiView->getTimelinePlayer();
            if (player)
            {
                const auto& inPlayer = player->player();
                const auto& info = inPlayer->getIOInfo();
                for (const auto& tag : info.tags)
                {
                    const std::string& key = tag.first;
                    const std::string rendererKey = "Renderer ";
                    if (key.compare(0, rendererKey.size(), rendererKey) == 0)
                    {
                        m->add(tag.second.c_str());
                    }
                }
            }
            std::string rendererName =
                settings->getValue<std::string>("USD/rendererName");
            int index = m->find_index(rendererName.c_str());
            if (index < 0)
                index = 0;
            m->value(index);
            mW->callback(
                [=](auto o)
                {
                    const Fl_Menu_Item* item = o->mvalue();
                    std::string renderName = item->label();
                    settings->setValue("USD/rendererName", renderName);
                    _update();
                });

            Y += 22;

            auto spW = new Widget< Fl_Spinner >(
                g->x() + 160, Y, g->w() - 160, 20, _("Render Width"));
            sp = spW;
            sp->format("%4.4g");
            sp->labelsize(12);
            sp->textcolor(FL_BLACK);
            sp->step(1);
            sp->range(32, 4096);
            sp->align(FL_ALIGN_LEFT);
            int v = settings->getValue<int>("USD/renderWidth");
            sp->value(v);

            spW->callback(
                [=](auto o)
                {
                    int v = static_cast<int>(o->value());
                    settings->setValue("USD/renderWidth", v);
                    _update();
                });

            Y += 22;
            spW = new Widget< Fl_Spinner >(
                g->x() + 160, Y, g->w() - 160, 20, _("Complexity"));
            sp = spW;
            sp->format("%4.4g");
            sp->labelsize(12);
            sp->textcolor(FL_BLACK);
            sp->step(0.001);
            // sp->range(1, 12);
            sp->range(1, 2);
            sp->align(FL_ALIGN_LEFT);

            float complexity = settings->getValue<float>("USD/complexity");
            sp->value(complexity);

            spW->callback(
                [=](auto o)
                {
                    float v = static_cast<float>(o->value());
                    settings->setValue("USD/complexity", v);
                    _update();
                });

            Y += 22;

            mW = new Widget< Fl_Choice >(
                g->x() + 130, Y, g->w() - 130, 20, _("Draw Mode"));
            m = mW;
            m->labelsize(12);
            m->align(FL_ALIGN_LEFT);
            for (const auto& i : tl::usd::getDrawModeLabels())
            {
                m->add(_(i.c_str()));
            }
            m->value(settings->getValue<int>("USD/drawMode"));
            mW->callback(
                [=](auto o)
                {
                    int v = o->value();
                    settings->setValue("USD/drawMode", v);
                    _update();
                });

            Y += 22;
            auto cV = new Widget< Fl_Check_Button >(
                g->x() + 90, Y, g->w(), 20, _("Enable Lighting"));
            Fl_Check_Button* c = cV;
            c->labelsize(12);
            c->value(settings->getValue<bool>("USD/enableLighting"));

            cV->callback(
                [=](auto w)
                {
                    int v = w->value();
                    settings->setValue("USD/enableLighting", v);
                    _update();
                });

            Y += 22;

            cV = new Widget< Fl_Check_Button >(
                g->x() + 90, Y, g->w(), 20, _("Enable sRGB"));
            c = cV;
            c->labelsize(12);
            c->value(settings->getValue<bool>("USD/sRGB"));

            cV->callback(
                [=](auto w)
                {
                    int v = w->value();
                    settings->setValue("USD/sRGB", v);
                    _update();
                });

            Y += 22;
            spW = new Widget< Fl_Spinner >(
                g->x() + 160, Y, g->w() - 160, 20, _("Stage Cache"));
            sp = spW;
            sp->format("%4.4g");
            sp->labelsize(12);
            sp->textcolor(FL_BLACK);
            sp->step(1);
            sp->range(32, 4096);
            sp->align(FL_ALIGN_LEFT);
            v = settings->getValue<int>("USD/stageCacheByteCount");
            sp->value(v);

            spW->callback(
                [=](auto o)
                {
                    int v = static_cast<int>(o->value());
                    settings->setValue("USD/stageCacheByteCount", v);
                    _update();
                });

            Y += 22;
            spW = new Widget< Fl_Spinner >(
                g->x() + 160, Y, g->w() - 160, 20, _("Disk Cache in GB"));
            sp = spW;
            sp->format("%4.4g");
            sp->labelsize(12);
            sp->textcolor(FL_BLACK);
            sp->step(1);
            sp->range(32, 4096);
            sp->align(FL_ALIGN_LEFT);
            v = settings->getValue<int>("USD/diskCacheByteCount");
            sp->value(v);

            spW->callback(
                [=](auto o)
                {
                    int v = static_cast<int>(o->value());
                    settings->setValue("USD/diskCacheByteCount", v);
                    _update();
                });

            bg->end();

            cg->end();

            std::string key = prefix + "USD";
            value = settings->getValue<std::any>(key);
            open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
            if (!open)
                cg->close();
        }

        void USDPanel::_update()
        {
            usd::sendIOOptions();
        }

    } // namespace panel

} // namespace mrv

#endif // TLRENDER_USD
