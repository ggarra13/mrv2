// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrViewer.h"

#include "mrvWidgets/mrvDockGroup.h"
#include "mrvWidgets/mrvResizableBar.h"
#include "mrvWidgets/mrvPanelConstants.h"
#include "mrvWidgets/mrvPanelGroup.h"

#include "mrvPanels/mrvPanelsAux.h"
#include "mrvPanels/mrvPanelWidget.h"

#include "mrvUI/mrvDesktop.h"

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

            SettingsObject* settings = App::app->settings();
            std::string key = "gui/" + label + "/Window/Visible";
            settings->setValue(key, 0);

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
            int W = dock->w();
            int H = dg->h();

            label = lbl;
            SettingsObject* settings = App::app->settings();
            std::string prefix = "gui/" + label;
            std::string key = prefix + "/Window";
            std_any value = settings->getValue<std::any>(key);
            int window = std_any_empty(value) ? 0 : std_any_cast<int>(value);

            if (window)
            {
                key = prefix + "/WindowX";
                value = settings->getValue<std::any>(key);
                X = std_any_empty(value) ? X : std_any_cast<int>(value);

                key = prefix + "/WindowY";
                value = settings->getValue<std::any>(key);
                Y = std_any_empty(value) ? Y : std_any_cast<int>(value);

                key = prefix + "/WindowW";
                value = settings->getValue<std::any>(key);
                W = std_any_empty(value) ? W : std_any_cast<int>(value);

                key = prefix + "/WindowH";
                value = settings->getValue<std::any>(key);
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

        math::Box2i PanelWidget::global_box() const
        {
            const Fl_Window* w = g->window();
            return math::Box2i(
                g->x() + w->x(), g->y() + w->y(), g->w(), g->h());
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

            if (g->docked())
            {
                // Adjust dock scrollbars for this new element
                p.ui->uiDock->pack->layout();
                p.ui->uiResizableBar->HandleDrag(0);

                // Adjust group (Search area in Media Info Panel)
                Fl_Group* group = g->get_group();
                if (group->visible())
                {
                    Pack* pack = g->get_pack();
                    group->size(pack->w(), group->h());
                }
            }
        }

        void PanelWidget::undock()
        {
            g->undock_grp();
        }

        void PanelWidget::dock()
        {
            g->dock_grp();
        }

        void PanelWidget::save()
        {
            TLRENDER_P();

            SettingsObject* settings = App::app->settings();

            std::string prefix = "gui/" + label;
            std::string key = prefix + "/Window";
            int window = !g->docked();

            // \@bug: Wayland currently cannot store properly the positions of
            //        panel windows.
            if (desktop::Wayland())
                window = 0;
            
            settings->setValue(key, window);

            key += "/Visible";
            settings->setValue(key, 1);

            if (window)
            {
                PanelWindow* w = g->get_window();

                key = prefix + "/WindowX";
                settings->setValue(key, w->x());

                key = prefix + "/WindowY";
                settings->setValue(key, w->y());

                key = prefix + "/WindowW";
                settings->setValue(key, w->w());

                key = prefix + "/WindowH";

                // Only store height if it is not a growing panel/window, else
                // store 0.
                int H = 0;
                if (isPanelWithHeight(_(label.c_str())))
                {
                    H = w->h();
                }
                settings->setValue(key, H);
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
