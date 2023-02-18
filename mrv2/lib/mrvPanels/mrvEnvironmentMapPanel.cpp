// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl_Flex.H>
#include <FL/Fl_Check_Button.H>

#include "mrvWidgets/mrvHorSlider.h"
#include "mrvWidgets/mrvFunctional.h"

#include "mrvGL/mrvTimelineViewport.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrViewer.h"

namespace mrv
{

    EnvironmentMapPanel::EnvironmentMapPanel(ViewerUI* ui) :
        PanelWidget(ui)
    {

        auto view = ui->uiView;

        EnvironmentMapOptions o = view->getEnvironmentMapOptions();
        o.type                  = EnvironmentMapOptions::kSpherical;
        view->setEnvironmentMapOptions(o);

        add_group("Environment Map");

        Fl_SVG_Image* svg = load_svg("EnvironmentMap.svg");
        g->image(svg);

        view->setActionMode(ActionMode::kRotate);

        g->callback(
            [](Fl_Widget* w, void* d) {
                ViewerUI* ui = static_cast< ViewerUI* >(d);
                delete environmentMapPanel;
                environmentMapPanel = nullptr;
                auto view           = ui->uiView;
                view->setActionMode(ActionMode::kScrub);
                ui->uiMain->fill_menu(ui->uiMenuBar);
            },
            ui);
    }

    EnvironmentMapPanel::~EnvironmentMapPanel() { clear_controls(); }

    void EnvironmentMapPanel::clear_controls()
    {
        delete focalLength;
        delete rotateX;
        delete rotateY;
    }

    void EnvironmentMapPanel::add_controls()
    {
        TLRENDER_P();

        auto settingsObject = p.ui->app->settingsObject();

        std_any value;
        int v;

        g->clear();
        g->begin();

        EnvironmentMapOptions o = p.ui->uiView->getEnvironmentMapOptions();
        Fl_Radio_Round_Button* r;
        HorSlider* s;
        Fl_Check_Button* c;
        CollapsibleGroup* cg =
            new CollapsibleGroup(g->x(), 20, g->w(), 20, _("Type"));
        auto b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);
        cg->layout();
        cg->begin();

        Fl_Flex* flex = new Fl_Flex(g->x(), 20, g->w(), 20);
        flex->type(Fl_Flex::HORIZONTAL);

        flex->begin();

        auto rB = new Widget< Fl_Radio_Round_Button >(
            g->x(), 90, g->w(), 20, _("None"));
        r = rB;
        r->labelsize(12);
        if (o.type == EnvironmentMapOptions::kNone)
            r->value(1);
        else
            r->value(0);
        r->tooltip(_("Turn off image warping."));
        rB->callback([=](auto w) {
            auto view               = p.ui->uiView;
            EnvironmentMapOptions o = view->getEnvironmentMapOptions();
            o.type                  = EnvironmentMapOptions::kNone;
            view->setEnvironmentMapOptions(o);
            view->setActionMode(ActionMode::kScrub);
        });

        rB = new Widget< Fl_Radio_Round_Button >(
            g->x(), 90, g->w(), 20, _("Spherical"));
        r = rB;
        r->labelsize(12);
        if (o.type == EnvironmentMapOptions::kSpherical)
            r->value(1);
        else
            r->value(0);
        r->tooltip(_("Wrap the image or images onto a sphere."));
        rB->callback([=](auto w) {
            auto view               = p.ui->uiView;
            EnvironmentMapOptions o = view->getEnvironmentMapOptions();
            o.type                  = EnvironmentMapOptions::kSpherical;
            view->setEnvironmentMapOptions(o);
        });

        rB = new Widget< Fl_Radio_Round_Button >(
            g->x(), 90, g->w(), 20, _("Cubic"));
        r = rB;
        r->labelsize(12);
        if (o.type == EnvironmentMapOptions::kCubic)
            r->value(1);
        else
            r->value(0);
        r->tooltip(_("Wrap the image or images onto a cube."));
        rB->callback([=](auto w) {
            auto view               = p.ui->uiView;
            EnvironmentMapOptions o = view->getEnvironmentMapOptions();
            o.type                  = EnvironmentMapOptions::kCubic;
            view->setEnvironmentMapOptions(o);
        });

        flex->end();

        cg->end();

        cg = new CollapsibleGroup(g->x(), 20, g->w(), 20, _("Projection"));
        b  = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);
        cg->layout();
        cg->begin();

        auto sV =
            new Widget< HorSlider >(g->x(), 90, g->w(), 20, _("H. Aperture"));
        s = sV;
        s->tooltip(_("Horizontal Aperture of the Projection."));
        s->range(0.001f, 90.0f);
        s->step(0.01F);
        s->default_value(24.0f);
        sV->callback([=](auto w) {
            auto view               = p.ui->uiView;
            EnvironmentMapOptions o = view->getEnvironmentMapOptions();
            o.horizontalAperture    = w->value();
            view->setEnvironmentMapOptions(o);
        });

        sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, _("V. Aperture"));
        s  = sV;
        s->tooltip(_("Vertical Aperture of the Projection."));
        s->range(0.f, 90.0f);
        s->step(0.1F);
        s->default_value(0.f);
        sV->callback([=](auto w) {
            auto view               = p.ui->uiView;
            EnvironmentMapOptions o = view->getEnvironmentMapOptions();
            o.verticalAperture      = w->value();
            view->setEnvironmentMapOptions(o);
        });

        sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, _("Focal Length"));
        s = focalLength = sV;
        s->tooltip(
            _("Focal Length of the Projection. Use Shift + left mouse button"
              " to change or the mousewheel."));
        s->range(0.0001f, 180.0f);
        s->step(0.1F);
        s->default_value(45.f);
        sV->callback([=](auto w) {
            auto view               = p.ui->uiView;
            EnvironmentMapOptions o = view->getEnvironmentMapOptions();
            o.focalLength           = w->value();
            view->setEnvironmentMapOptions(o);
        });

        cg->end();

        cg = new CollapsibleGroup(g->x(), 20, g->w(), 20, _("Rotation"));
        b  = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);
        cg->layout();
        cg->begin();

        auto cB =
            new Widget< Fl_Check_Button >(g->x(), 90, g->w(), 20, _("Spin"));
        c = cB;
        c->labelsize(12);
        c->tooltip(_("Spin with middle mouse instead of rotating with it."));

        value = settingsObject->value("EnvironmenºtMap/Spin");
        v     = std_any_empty(value) ? 0 : std_any_cast<int>(value);
        c->value(v);
        auto view = p.ui->uiView;

        o.spin = v;
        view->setEnvironmentMapOptions(o);

        cB->callback([=](auto w) {
            auto view               = p.ui->uiView;
            EnvironmentMapOptions o = view->getEnvironmentMapOptions();
            o.spin                  = w->value();
            settingsObject->setValue("EnvironmentMap/Spin", (int)o.spin);
            view->setEnvironmentMapOptions(o);
        });

        sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, "X");
        s = rotateX = sV;
        s->tooltip(_("Rotation in X of the projection.  Use middle mouse "
                     "button to move around."));
        s->range(-90.f, 90.0f);
        s->default_value(0.0f);
        sV->callback([=](auto w) {
            auto view               = p.ui->uiView;
            EnvironmentMapOptions o = view->getEnvironmentMapOptions();
            o.rotateX               = w->value();
            view->setEnvironmentMapOptions(o);
        });

        sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, "Y");
        s = rotateY = sV;
        s->tooltip(_("Rotation in Y of the projection.  Use middle mouse "
                     "button to move around."));
        s->range(-180.f, 180.0f);
        s->default_value(0.0f);
        sV->callback([=](auto w) {
            auto view               = p.ui->uiView;
            EnvironmentMapOptions o = view->getEnvironmentMapOptions();
            o.rotateY               = w->value();
            view->setEnvironmentMapOptions(o);
        });

        cg->end();

        cg = new CollapsibleGroup(g->x(), 20, g->w(), 20, _("Subdivisions"));
        b  = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);
        b->tooltip(
            _("Subdivision of the sphere when doing spherical projections"));
        cg->layout();
        cg->begin();

        sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, "X");
        s  = sV;
        s->tooltip(_("Subdivision of the sphere in X."));
        s->range(4.0f, 90.0f);
        s->step(1);

        value = settingsObject->value("EnvironmentMap/Sphere/SubdivisionX");
        v     = std_any_empty(value) ? 36 : std_any_cast<int>(value);

        s->default_value(v);
        sV->callback([=](auto w) {
            auto view               = p.ui->uiView;
            EnvironmentMapOptions o = view->getEnvironmentMapOptions();
            int v                   = static_cast<int>(w->value());
            o.subdivisionX          = v;
            settingsObject->setValue("EnvironmentMap/Sphere/SubdivisionX", v);
            view->setEnvironmentMapOptions(o);
        });

        value = settingsObject->value("EnvironmentMap/Sphere/SubdivisionY");
        v     = std_any_empty(value) ? 36 : std_any_cast<int>(value);

        sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, "Y");
        s  = sV;
        s->tooltip(_("Subdivision of the sphere in Y."));
        s->range(4.0f, 90.0f);
        s->step(1);

        value = settingsObject->value("EnvironmentMap/Sphere/SubdivisionY");
        v     = std_any_empty(value) ? 36 : std_any_cast<int>(value);

        s->default_value(v);
        sV->callback([=](auto w) {
            auto view               = p.ui->uiView;
            EnvironmentMapOptions o = view->getEnvironmentMapOptions();
            int v                   = static_cast<int>(w->value());
            o.subdivisionY          = v;
            settingsObject->setValue("EnvironmentMap/Sphere/SubdivisionY", v);
            view->setEnvironmentMapOptions(o);
        });

        cg->end();

        g->end();

        p.ui->uiView->redrawWindows();
    }

} // namespace mrv
