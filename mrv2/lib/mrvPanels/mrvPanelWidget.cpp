// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrViewer.h"

#include "mrvWidgets/mrvDockGroup.h"
#include "mrvWidgets/mrvResizableBar.h"
#include "mrvWidgets/mrvPanelGroup.h"

#include "mrvPanels/mrvPanelsAux.h"
#include "mrvPanels/mrvPanelWidget.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "panelwidget";
}

namespace mrv
{
    namespace panel
    {

        PanelWidget::PanelWidget(ViewerUI* ui) :
            _p(new Private)
        {
            _p->ui = ui;
        }

        PanelWidget::~PanelWidget()
        {
            TLRENDER_P();

            save();

            SettingsObject* settingsObject = p.ui->app->settingsObject();
            std::string key = "gui/" + label + "/Window/Visible";
            settingsObject->setValue(key, 0);

            delete g->image();
            g->image(nullptr);

            PanelGroup::cb_dismiss(NULL, g);
        }

        void PanelWidget::add_group(const char* lbl)
        {
            TLRENDER_P();

            Fl_Group* dg = p.ui->uiDockGroup;
            ResizableBar* bar = p.ui->uiResizableBar;
            DockGroup* dock = p.ui->uiDock;
            int X = dock->x();
            int Y = dock->y();
            // int W = dg->w() - bar->w();
            int W = dock->w();
            int H = dg->h();

            label = lbl;
            SettingsObject* settingsObject = p.ui->app->settingsObject();
            std::string prefix = "gui/" + label;
            std::string key = prefix + "/Window";
            std_any value = settingsObject->value(key);
            int window = std_any_empty(value) ? 0 : std_any_cast<int>(value);

            if (window)
            {
                key = prefix + "/WindowX";
                value = settingsObject->value(key);
                X = std_any_empty(value) ? X : std_any_cast<int>(value);

                key = prefix + "/WindowY";
                value = settingsObject->value(key);
                Y = std_any_empty(value) ? Y : std_any_cast<int>(value);

                key = prefix + "/WindowW";
                value = settingsObject->value(key);
                W = std_any_empty(value) ? W : std_any_cast<int>(value);

                key = prefix + "/WindowH";
                value = settingsObject->value(key);
                H = std_any_empty(value) ? H : std_any_cast<int>(value);
                if (H == 0)
                    H = 20 + 30;
            }
            else
            {
                if (panel::onlyOne())
                    removePanels(p.ui);
            }

            g = new PanelGroup(dock, window, X, Y, W, H, _(lbl));
            g->setLabel(label);

            begin_group();
            add_controls();
            end_group();
        }

        math::Box2i PanelWidget::box() const
        {
            math::Box2i b;
            if (!g->docked())
            {
                auto w = g->get_window();
                b = math::Box2i(
                    g->x() + w->x(), g->y() + w->y(), g->w(), g->h());
            }
            else
            {
                b = math::Box2i(g->x(), g->y(), g->w(), g->h());
            }
            return b;
        }
        void PanelWidget::begin_group()
        {
            g->clear();
            g->begin();
        }

        void PanelWidget::end_group()
        {
            TLRENDER_P();
            g->end();
            p.ui->uiDock->pack->layout();
            p.ui->uiResizableBar->HandleDrag(0);

            // Check if we are a panel in a window
            PanelWindow* w = g->get_window();
            if (w && !g->docked())
            {
                int H = g->get_pack()->h() + g->get_dragger()->h();
                Fl_Group* grp = g->get_group();
                if (grp && grp->visible())
                    H += grp->h();

                // Adjust window to packed size
                Fl_Widget* r = w->resizable();
                w->resizable(0);
                w->size(w->w(), H + 3);
                w->resizable(r);
            }
        }

        void PanelWidget::undock()
        {
            g->undock_grp(g);
        }

        void PanelWidget::dock()
        {
            g->dock_grp(g);
        }

        void PanelWidget::save()
        {
            TLRENDER_P();

            SettingsObject* settingsObject = p.ui->app->settingsObject();

            std::string prefix = "gui/" + label;
            std::string key = prefix + "/Window";
            int window = !g->docked();
            settingsObject->setValue(key, window);

            key += "/Visible";
            settingsObject->setValue(key, 1);

            if (window)
            {
                PanelWindow* w = g->get_window();

                key = prefix + "/WindowX";
                settingsObject->setValue(key, w->x());

                key = prefix + "/WindowY";
                settingsObject->setValue(key, w->y());

                key = prefix + "/WindowW";
                settingsObject->setValue(key, w->w());

                key = prefix + "/WindowH";

                // Only store height if it is not a growing panel/window, else
                // store 0.
                int H = 0;
                if (isPanelWithHeight(_(label.c_str())))
                {
                    H = w->h();
                }
                settingsObject->setValue(key, H);
            }
        }

        void PanelWidget::clear_controls()
        {
            g->clear();
        }

        void PanelWidget::refresh()
        {
            begin_group();
            add_controls();
            end_group();
        }

    } // namespace panel

} // namespace mrv
