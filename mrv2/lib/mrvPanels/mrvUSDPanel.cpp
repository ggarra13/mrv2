// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#if defined(TLRENDER_USD)
#    include <tlIO/USD.h>

#    include <tlCore/StringFormat.h>

#    include <FL/Fl_Check_Button.H>
#    include <FL/Fl_Choice.H>
#    include <FL/Fl_Spinner.H>

#    include "mrViewer.h"

#    include "mrvWidgets/mrvFunctional.h"
#    include "mrvWidgets/mrvHorSlider.h"
#    include "mrvWidgets/mrvCollapsibleGroup.h"

#    include "mrvPanels/mrvPanelsCallbacks.h"
#    include "mrvPanels/mrvUSDPanel.h"

#    include "mrvFl/mrvIO.h"

#    include "mrvApp/mrvSettingsObject.h"

namespace mrv
{

    static const char* kModule = "usd";

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

        SettingsObject* settingsObject = p.ui->app->settingsObject();
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

                App* app = App::application();
                auto settingsObject = app->settingsObject();
                settingsObject->setValue(key, static_cast<int>(cg->is_open()));

                usdPanel->refresh();
            },
            cg);

        cg->begin();

        Y += 30;
        bg = new Fl_Group(g->x(), Y, g->w(), 22 * 8);
        bg->box(FL_NO_BOX);
        bg->begin();

        Fl_Box* box = new Fl_Box(
            g->x(), Y, g->w(), 40,
            _("Changes are applied to "
              "newly opened files."));
        box->labelsize(12);
        box->align(FL_ALIGN_WRAP);

        Y += 50;
        auto spW = new Widget< Fl_Spinner >(
            g->x() + 160, Y, g->w() - 160, 20, _("Render Width"));
        sp = spW;
        sp->format("%4.4g");
        sp->labelsize(12);
        sp->color((Fl_Color)-1733777408);
        sp->textcolor(FL_BLACK);
        sp->step(1);
        sp->range(32, 4096);
        sp->align(FL_ALIGN_LEFT);
        int v = std_any_cast<int>(settingsObject->value("usd/renderWidth"));
        sp->value(v);

        spW->callback(
            [=](auto o)
            {
                int v = static_cast<int>(o->value());
                settingsObject->setValue("usd/renderWidth", v);
            });

        Y += 22;
        spW = new Widget< Fl_Spinner >(
            g->x() + 160, Y, g->w() - 160, 20, _("Complexity"));
        sp = spW;
        sp->format("%4.4g");
        sp->labelsize(12);
        sp->color((Fl_Color)-1733777408);
        sp->textcolor(FL_BLACK);
        sp->step(1);
        sp->range(1, 12);
        sp->align(FL_ALIGN_LEFT);
        v = std_any_cast<int>(settingsObject->value("usd/complexity"));
        sp->value(v);

        spW->callback(
            [=](auto o)
            {
                int v = static_cast<int>(o->value());
                settingsObject->setValue("usd/complexity", v);
            });

        Y += 22;
        auto mW = new Widget< Fl_Choice >(
            g->x() + 130, Y, g->w() - 130, 20, _("Draw Mode"));
        Fl_Choice* m = mW;
        m->labelsize(12);
        m->align(FL_ALIGN_LEFT);
        for (const auto& i : usd::getDrawModeLabels())
        {
            m->add(_(i.c_str()));
        }
        m->value(std_any_cast<int>(settingsObject->value("usd/drawMode")));
        mW->callback(
            [=](auto o)
            {
                int v = o->value();
                settingsObject->setValue("usd/drawMode", v);
            });

        Y += 22;
        auto cV = new Widget< Fl_Check_Button >(
            g->x() + 90, Y, g->w(), 20, _("Enable Lighting"));
        Fl_Check_Button* c = cV;
        c->labelsize(12);
        c->value(
            std_any_cast<int>(settingsObject->value("usd/enableLighting")));

        cV->callback(
            [=](auto w)
            {
                int v = w->value();
                settingsObject->setValue("usd/enableLighting", v);
            });

        Y += 22;
        spW = new Widget< Fl_Spinner >(
            g->x() + 160, Y, g->w() - 160, 20, _("Stage Cache"));
        sp = spW;
        sp->format("%4.4g");
        sp->labelsize(12);
        sp->color((Fl_Color)-1733777408);
        sp->textcolor(FL_BLACK);
        sp->step(1);
        sp->range(32, 4096);
        sp->align(FL_ALIGN_LEFT);
        v = std_any_cast<int>(settingsObject->value("usd/stageCacheCount"));
        sp->value(1);

        spW->callback(
            [=](auto o)
            {
                int v = static_cast<int>(o->value());
                settingsObject->setValue("usd/stageCacheCount", v);
            });

        Y += 22;
        spW = new Widget< Fl_Spinner >(
            g->x() + 160, Y, g->w() - 160, 20, _("Disk Cache"));
        sp = spW;
        sp->format("%4.4g");
        sp->labelsize(12);
        sp->color((Fl_Color)-1733777408);
        sp->textcolor(FL_BLACK);
        sp->step(1);
        sp->range(32, 4096);
        sp->align(FL_ALIGN_LEFT);
        v = std_any_cast<int>(settingsObject->value("usd/diskCacheByteCount"));
        sp->value(1);

        spW->callback(
            [=](auto o)
            {
                int v = static_cast<int>(o->value());
                settingsObject->setValue("usd/diskCacheByteCount", v);
            });

        bg->end();

        cg->end();

        std::string key = prefix + "USD";
        value = settingsObject->value(key);
        open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
        if (!open)
            cg->close();
    }

} // namespace mrv

#endif // TLRENDER_USD
