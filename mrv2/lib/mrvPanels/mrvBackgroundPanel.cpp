// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>

#include <tlTimeline/BackgroundOptions.h>

#include "mrViewer.h"

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvHorSlider.h"
#include "mrvWidgets/mrvCollapsibleGroup.h"

#include "mrvPanels/mrvBackgroundPanel.h"
#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvApp/mrvSettingsObject.h"

namespace mrv
{
    namespace panel
    {
        struct BackgroundPanel::Private
        {
        };

        BackgroundPanel::BackgroundPanel(ViewerUI* ui) :
            PanelWidget(ui)
        {
            add_group("Background");

            // Fl_SVG_Image* svg = load_svg("Background.svg");
            // g->image(svg);

            g->callback(
                [](Fl_Widget* w, void* d)
                {
                    ViewerUI* ui = static_cast< ViewerUI* >(d);
                    delete backgroundPanel;
                    backgroundPanel = nullptr;
                    ui->uiMain->fill_menu(ui->uiMenuBar);
                },
                ui);
        }

        BackgroundPanel::~BackgroundPanel() {}

        void BackgroundPanel::add_controls()
        {
            TLRENDER_P();

            SettingsObject* settings = App::app->settings();

            int X = g->x();
            int Y = 20;

            Fl_Button* b;
            HorSlider* s;
            Fl_Choice* c;
            CollapsibleGroup* cg;

            g->clear();

            auto options = p.ui->uiView->getBackgroundOptions();

            Fl_Group* bg = new Fl_Group(X, Y, g->w(), 40);
            bg->begin();

            auto cW = new Widget< Fl_Choice >(
                g->x() + 100, Y + 10, g->w() - 100, 20, _("Type"));
            c = cW;
            c->labelsize(12);
            c->align(FL_ALIGN_LEFT);
            for (const auto& type : tl::timeline::getBackgroundLabels())
            {
                c->add(type.c_str());
            }

            c->value(static_cast<int>(options.type));
            c->tooltip(_("Selects the current background type from the list"));
            cW->callback(
                [=](auto o)
                {
                    int type = o->value();
                    settings->setValue("Background/Type", type);
                    auto view = p.ui->uiView;
                    auto options = view->getBackgroundOptions();
                    options.type = static_cast<tl::timeline::Background>(type);
                    view->setBackgroundOptions(options);
                    view->redrawWindows();
                    refresh();
                });

            Y += 10;

            bg->end();

            if (options.type == tl::timeline::Background::Solid)
            {
                bg = new Fl_Group(X, Y, g->w(), 80);
                bg->begin();

                Y += 10;
                auto bW = new Widget< Fl_Button >(
                    X + 100, Y, 25, 25, _("Solid Color:"));
                b = bW;
                b->tooltip(_("Selects the solid color."));
                b->box(FL_EMBOSSED_BOX);
                b->align(FL_ALIGN_LEFT);
                Fl_Color color = to_fltk_color(options.color0);
                b->color(color);
                b->labelsize(11);
                bW->callback(
                    [=](auto o)
                    {
                        auto color = get_color_cb(o->color(), p.ui);
                        Fl_Color c = to_fltk_color(color);
                        settings->setValue(
                            "Background/color0", static_cast<int>(c));
                        o->color(c);
                        o->redraw();
                        auto view = p.ui->uiView;
                        auto options = view->getBackgroundOptions();
                        options.color0 = color;
                        options.color0.a = 1;
                        view->setBackgroundOptions(options);
                        view->redrawWindows();
                    });
                bg->end();
            }
            else if (options.type == tl::timeline::Background::Checkers)
            {
                bg = new Fl_Group(X, Y, g->w(), 55);
                bg->begin();

                Y += 10;

                auto sV = new Widget< HorSlider >(X, Y, g->w(), 20, _("Size:"));
                s = sV;
                s->range(2, 300);
                s->step(1);
                s->tooltip(_("Selects the checker size."));
                s->default_value(options.checkersSize.w);
                sV->callback(
                    [=](auto o)
                    {
                        settings->setValue(
                            "Background/CheckersSize",
                            static_cast<int>(o->value()));
                        auto view = p.ui->uiView;
                        auto options = view->getBackgroundOptions();
                        options.checkersSize =
                            math::Size2i(o->value(), o->value());
                        view->setBackgroundOptions(options);
                        view->redrawWindows();
                    });

                Y += 25;

                bg->end();
            }

            if (options.type == tl::timeline::Background::Checkers ||
                options.type == tl::timeline::Background::Gradient)
            {
                bg = new Fl_Group(X, Y, g->w(), 80);
                bg->begin();
                
                auto bW = new Widget< Fl_Button >(
                    X + 100, Y, 25, 25, _("First Color:"));
                b = bW;
                b->tooltip(_("Selects the first color."));
                b->box(FL_EMBOSSED_BOX);
                b->align(FL_ALIGN_LEFT);
                Fl_Color color = to_fltk_color(options.color1);
                b->color(color);
                b->labelsize(11);
                bW->callback(
                    [=](auto o)
                    {
                        auto color = get_color_cb(o->color(), p.ui);
                        Fl_Color c = to_fltk_color(color);
                        settings->setValue(
                            "Background/CheckersColor1", static_cast<int>(c));
                        o->color(c);
                        o->redraw();
                        auto view = p.ui->uiView;
                        auto options = view->getBackgroundOptions();
                        options.color1 = color;
                        view->setBackgroundOptions(options);
                        view->redrawWindows();
                    });

                Y += 25;

                bW = new Widget< Fl_Button >(
                    X + 100, Y, 25, 25, _("Second Color:"));
                b = bW;
                b->tooltip(_("Selects the second color in Checkers."));
                b->box(FL_EMBOSSED_BOX);
                b->align(FL_ALIGN_LEFT);
                color = to_fltk_color(options.color0);
                b->color(color);
                b->labelsize(11);
                bW->callback(
                    [=](auto o)
                    {
                        auto color = get_color_cb(o->color(), p.ui);
                        Fl_Color c = to_fltk_color(color);
                        settings->setValue(
                            "Background/CheckersColor0", static_cast<int>(c));
                        o->color(c);
                        o->redraw();
                        auto view = p.ui->uiView;
                        auto options = view->getBackgroundOptions();
                        options.color0 = color;
                        view->setBackgroundOptions(options);
                        view->redrawWindows();
                    });
                bg->end();
            }
        }

    } // namespace panel

} // namespace mrv
