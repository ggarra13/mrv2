// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <string>
#include <vector>
#include <map>

#include <tlCore/StringFormat.h>

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvLogDisplay.h"

#include "mrvPanelsCallbacks.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrViewer.h"

namespace mrv
{
    namespace panel
    {

        struct LogsPanel::Private
        {
            App* app;
            Fl_Button* clearButton;
            std::shared_ptr<observer::ListObserver<log::Item> > logObserver;
        };

        LogsPanel::LogsPanel(ViewerUI* ui) :
            _r(new Private),
            PanelWidget(ui)
        {
            add_group("Logs");

            Fl_SVG_Image* svg = load_svg("Logs.svg");
            g->image(svg);

            g->callback(
                [](Fl_Widget* w, void* d)
                {
                    ViewerUI* ui = static_cast< ViewerUI* >(d);
                    delete logsPanel;
                    logsPanel = nullptr;
                    ui->uiMain->fill_menu(ui->uiMenuBar);
                },
                ui);
        }

        LogsPanel::~LogsPanel()
        {
            g->remove(uiLogDisplay);
        }

        void LogsPanel::dock()
        {
            PanelWidget::dock();
            // @todo: avoid scrolling issues
        }

        void LogsPanel::undock()
        {
            TLRENDER_P();

            PanelWidget::undock();
            PanelWindow* w = g->get_window();

            SettingsObject* settings = App::app->settings();

            std::string prefix = "gui/" + label;
            std::string key;
            std_any value;

            key = prefix + "/WindowX";
            value = settings->getValue<std::any>(key);
            int X = std_any_empty(value) ? w->x() : std_any_cast<int>(value);

            key = prefix + "/WindowY";
            value = settings->getValue<std::any>(key);
            int Y = std_any_empty(value) ? w->y() : std_any_cast<int>(value);

            key = prefix + "/WindowW";
            value = settings->getValue<std::any>(key);
            int W = std_any_empty(value) ? 512 : std_any_cast<int>(value);

            key = prefix + "/WindowH";
            value = settings->getValue<std::any>(key);
            int H = std_any_empty(value) ? 512 : std_any_cast<int>(value);

            if (W < 512)
                W = 512;
            if (H < 512)
                H = 512;

            w->resize(X, Y, W, H);
        }

        void LogsPanel::save()
        {
            TLRENDER_P();

            PanelWidget::save();

            // We make the log panel save as hidden, never visible.
            SettingsObject* settings = App::app->settings();
            const std::string key = "gui/" + label + "/Window/Visible";
            settings->setValue(key, 0);
        }

        void LogsPanel::add_controls()
        {
            TLRENDER_P();

            g->remove(uiLogDisplay);

            Fl_Group* controls = g->get_group();
            controls->size(g->w(), 30); // needed
            controls->show();

            controls->begin();

            assert(controls->h() >= 0);

            int Y = controls->y();
            _r->clearButton = new Fl_Button(g->x(), Y, g->w(), 30);
            _r->clearButton->image(load_svg("Clear.svg"));
            _r->clearButton->tooltip(_("Clear the messages"));
            _r->clearButton->callback(
                [](Fl_Widget* w, void* d)
                {
                    LogDisplay* log = static_cast< LogDisplay* >(d);
                    log->clear();
                },
                uiLogDisplay);

            controls->end();

            g->clear();

            g->begin();

            g->add(uiLogDisplay);

            Y = controls->y() + controls->h();

            Fl_Scroll* scroll = g->get_scroll();
            scroll->position(scroll->x(), Y);

            Pack* pack = g->get_pack();
            pack->position(pack->x(), Y);
            pack->layout();

            uiLogDisplay->resize(g->x(), Y, g->w(), g->h());

            g->resizable(uiLogDisplay);
            g->end();
        }

        void LogsPanel::info(const std::string& msg) const
        {
            auto context = _p->ui->app->getContext();
            context->log("", msg, log::Type::Message);
        }
        void LogsPanel::warning(const std::string& msg) const
        {
            auto context = _p->ui->app->getContext();
            context->log("", msg, log::Type::Warning);
        }

        void LogsPanel::error(const std::string& msg) const
        {
            auto context = _p->ui->app->getContext();
            context->log("", msg, log::Type::Error);
        }

    } // namespace panel

} // namespace mrv
