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

        SettingsObject* settingsObject = p.ui->app->settingsObject();

        std::string prefix = "gui/" + label;
        std::string key;
        std_any value;

        key = prefix + "/WindowX";
        value = settingsObject->value(key);
        int X = std_any_empty(value) ? w->x() : std_any_cast<int>(value);

        key = prefix + "/WindowY";
        value = settingsObject->value(key);
        int Y = std_any_empty(value) ? w->y() : std_any_cast<int>(value);

        key = prefix + "/WindowW";
        value = settingsObject->value(key);
        int W = std_any_empty(value) ? 512 : std_any_cast<int>(value);

        key = prefix + "/WindowH";
        value = settingsObject->value(key);
        int H = std_any_empty(value) ? 512 : std_any_cast<int>(value);

        if (W < 512)
            W = 512;
        if (H < 512)
            H = 512;
        w->resize(X, Y, W, H);
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

        uiLogDisplay->resize(g->x(), Y, g->w(), g->h() + Y);

        g->resizable(uiLogDisplay);
        g->end();

        _r->logObserver = observer::ListObserver<log::Item>::create(
            p.ui->app->getContext()->getLogSystem()->observeLog(),
            [this](const std::vector<log::Item>& value)
            {
                for (const auto& i : value)
                {
                    switch (i.type)
                    {
                    case log::Type::Message:
                    {
                        if (Preferences::debug)
                        {
                            const std::string& msg =
                                string::Format("{0} {1}: {2}\n")
                                    .arg(i.time)
                                    .arg(i.prefix)
                                    .arg(i.message);
                            uiLogDisplay->info(msg.c_str());
                        }
                        break;
                    }
                    case log::Type::Warning:
                    {
                        const std::string& msg =
                            string::Format("{0} Warning {1}: {2}\n")
                                .arg(i.time)
                                .arg(i.prefix)
                                .arg(i.message);
                        uiLogDisplay->warning(msg.c_str());
                        break;
                    }
                    case log::Type::Error:
                    {
                        const std::string& msg =
                            string::Format("{0} ERROR {1}: {2}\n")
                                .arg(i.time)
                                .arg(i.prefix)
                                .arg(i.message);
                        uiLogDisplay->error(msg.c_str());
                        break;
                    }
                    }
                }
            });
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

} // namespace mrv
