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
        PanelWidget::undock();
        PanelWindow* w = g->get_window();
        // Resize window to a good size
        w->resize(40, 40, 512, 512);
    }

    void LogsPanel::add_controls()
    {
        TLRENDER_P();

        g->remove(uiLogDisplay);

        g->clear();

        g->begin();

        g->add(uiLogDisplay);
        uiLogDisplay->resize(
            g->x(), g->y() + 20, g->w(), p.ui->uiViewGroup->h() - 50);

        _r->clearButton =
            new Fl_Button(g->x(), g->y() + uiLogDisplay->h(), 30, 30);
        _r->clearButton->image(load_svg("Clear.svg"));
        _r->clearButton->tooltip(_("Clear the messages"));
        _r->clearButton->callback(
            [](Fl_Widget* w, void* d)
            {
                LogDisplay* log = static_cast< LogDisplay* >(d);
                log->clear();
            },
            uiLogDisplay);

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
                        if ( Preferences::debug )
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
