// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrViewer.h"

#include "mrvCore/mrvColorAreaInfo.h"

#include "mrvIcons/Vectorscope.h"

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvVectorscope.h"

#include "mrvPanels/mrvPanelsCallbacks.h"
#include "mrvPanels/mrvVectorscopePanel.h"

#include "FL/Fl_Choice.H"

namespace mrv
{
    namespace panel
    {
        struct VectorscopePanel::Private
        {
            Vectorscope* vectorscope = nullptr;
        };

        VectorscopePanel::VectorscopePanel(ViewerUI* ui) :
            _r(new Private),
            PanelWidget(ui)
        {
            add_group("Vectorscope");

            Fl_SVG_Image* svg = MRV2_LOAD_SVG(Vectorscope);
            g->bind_image(svg);

            g->callback(
                [](Fl_Widget* w, void* d)
                {
                    ViewerUI* ui = static_cast< ViewerUI* >(d);
                    delete vectorscopePanel;
                    vectorscopePanel = nullptr;
                    ui->uiMain->fill_menu(ui->uiMenuBar);
                },
                ui);
        }

        VectorscopePanel::~VectorscopePanel() {}

        void VectorscopePanel::add_controls()
        {
            TLRENDER_P();
            MRV2_R();

            Pack* pack = g->get_pack();
            pack->spacing(5);

            g->clear();
            g->begin();

            int X = g->x();
            int Y = g->y();
            int W = g->w();
            int H = g->h();
            
            Fl_Group* cg;
            Fl_Box* b;
            Fl_Choice* c;
            cg = new Fl_Group(X, Y, W, 20);
            cg->begin();
            b = new Fl_Box(X, Y, 120, 20, _("Type"));
            auto cW = new Widget< Fl_Choice >(X + b->w(), Y, W - b->w(), 20);
            c = cW;
            c->add("BT.601");
            c->add("BT.709");
            c->value(1);
#ifdef VULKAN_BACKEND
            c->add("Rec.2020");
            c->value(2);
#endif
            c->tooltip(_("Type of Vectorscope."));
            cW->callback(
                [=](auto o)
                {
                    VectorscopeMethod c =
                        static_cast<VectorscopeMethod>(o->value());
                    _r->vectorscope->setMethod(c);
                });
            cg->end();

            // Create a square vectorscope
            r.vectorscope = new Vectorscope(X, Y, g->w(), g->w());
            r.vectorscope->main(p.ui);

            g->resizable(r.vectorscope);
        }

        void VectorscopePanel::update(const area::Info& info)
        {
            _r->vectorscope->update(info);
        }

    } // namespace panel
} // namespace mrv
