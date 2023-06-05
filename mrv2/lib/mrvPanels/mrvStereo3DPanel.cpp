// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl_Group.H>

#include "mrvWidgets/mrvCollapsibleGroup.h"
#include "mrvWidgets/mrvHorSlider.h"
#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvPopupMenu.h"

#include "mrvPanels/mrvPanelsCallbacks.h"
#include "mrvPanels/mrvStereo3DPanel.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrViewer.h"

namespace mrv
{
    struct Stereo3DPanel::Private
    {
        PopupMenu* input = nullptr;
        PopupMenu* output = nullptr;
        HorSlider* eyeSeparation = nullptr;
    };

    Stereo3DPanel::Stereo3DPanel(ViewerUI* ui) :
        PanelWidget(ui),
        _r(new Private)
    {

        auto view = ui->uiView;

        add_group("Stereo 3D");

        // Fl_SVG_Image* svg = load_svg("Stereo3D.svg");
        // g->image(svg);

        g->callback(
            [](Fl_Widget* w, void* d)
            {
                delete stereo3DPanel;
                stereo3DPanel = nullptr;
            },
            ui);
    }

    Stereo3DPanel::~Stereo3DPanel() {}

    void Stereo3DPanel::add_controls()
    {
        TLRENDER_P();

        auto settingsObject = p.ui->app->settingsObject();

        auto model = p.ui->app->filesModel();

        std_any value;
        int v;

        g->begin();

        Stereo3DOptions o = model->observeStereo3DOptions()->get();

        HorSlider* s;
        Fl_Group* bg;
        PopupMenu* m;
        CollapsibleGroup* cg =
            new CollapsibleGroup(g->x(), 20, g->w(), 20, _("Stereo 3D"));
        auto b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);
        cg->layout();
        cg->begin();

        bg = new Fl_Group(g->x(), 20, g->w(), 20);
        bg->begin();

        Fl_Box* box = new Fl_Box(g->x(), 20, 150, 20, _("Input"));
        box->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);

        auto pW = new Widget< PopupMenu >(
            g->x() + 150, 20, g->w() - 150, 20, _("None"));
        m = _r->input = pW;
        m->add(_("None"));
        m->add(_("Image"));
        m->add(_("Horizontal"));
        m->add(_("Vertical"));
        pW->callback(
            [=](auto w)
            {
                Stereo3DOptions o = model->observeStereo3DOptions()->get();
                o.input = static_cast<Stereo3DOptions::Input>(w->value());
                model->setStereo3DOptions(o);
            });

        bg->end();

        bg = new Fl_Group(g->x(), 20, g->w(), 20);
        bg->begin();

        box = new Fl_Box(g->x(), 20, 150, 20, _("Output"));
        box->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);

        pW = new Widget< PopupMenu >(
            g->x() + 150, 20, g->w() - 150, 20, _("Anaglyph"));
        m = _r->output = pW;
        m->add(_("Anaglyph"));
        m->add(_("Right Anaglyph"));
        m->add(_("OpenGL"));
        pW->callback(
            [=](auto w)
            {
                Stereo3DOptions o = model->observeStereo3DOptions()->get();
                o.output = static_cast<Stereo3DOptions::Output>(w->value());
                model->setStereo3DOptions(o);
            });

        bg->end();

        cg->end();

        cg = new CollapsibleGroup(g->x(), 20, g->w(), 20, _("Adjustments"));
        b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);
        cg->layout();
        cg->begin();

        auto sV = new Widget< HorSlider >(
            g->x(), 90, g->w(), 20, _("Eye Separation"));
        s = _r->eyeSeparation = sV;
        s->tooltip(_("Eye separtion of left and right eye."));
        s->range(0.0f, 10.0f);
        s->step(0.01F);
        s->default_value(0.0f);
        sV->callback(
            [=](auto w)
            {
                auto model = p.ui->app->filesModel();
                Stereo3DOptions o = model->observeStereo3DOptions()->get();
                o.eyeSeparation = w->value();
                model->setStereo3DOptions(o);
            });

        cg->end();

        g->end();
    }

    void Stereo3DPanel::setStereo3DOptions(const Stereo3DOptions& value)
    {
        MRV2_R();
        r.input->value(static_cast<int>(value.input));
        r.output->value(static_cast<int>(value.output));
        r.eyeSeparation->value(value.eyeSeparation);
    }

} // namespace mrv
