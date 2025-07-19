// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrViewer.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvWidgets/mrvHorSlider.h"
#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvCollapsibleGroup.h"

#include "mrvIcons/EnvironmentMap.h"

#include <FL/Fl_Flex.H>
#include <FL/Fl_Check_Button.H>

namespace mrv
{
    namespace panel
    {
        struct EnvironmentMapPanel::Private
        {
            Fl_Radio_Round_Button* none = nullptr;
            Fl_Radio_Round_Button* spherical = nullptr;
            Fl_Radio_Round_Button* cubic = nullptr;
            HorSlider* hAperture = nullptr;
            HorSlider* vAperture = nullptr;
            HorSlider* focalLength = nullptr;
            HorSlider* rotateX = nullptr;
            HorSlider* rotateY = nullptr;
            Fl_Check_Button* spin = nullptr;
            HorSlider* subdivisionX = nullptr;
            HorSlider* subdivisionY = nullptr;
        };

        EnvironmentMapPanel::EnvironmentMapPanel(ViewerUI* ui) :
            PanelWidget(ui),
            _r(new Private)
        {

            auto view = ui->uiView;

            add_group("Environment Map");

            Fl_SVG_Image* svg = MRV2_LOAD_SVG(EnvironmentMap);
            g->bind_image(svg);

            g->callback(
                [](Fl_Widget* w, void* d)
                {
                    ViewerUI* ui = static_cast< ViewerUI* >(d);
                    delete environmentMapPanel;
                    environmentMapPanel = nullptr;
                    auto view = ui->uiView;
                    view->setActionMode(ActionMode::kScrub);
                    ui->uiMain->fill_menu(ui->uiMenuBar);
                },
                ui);
        }

        EnvironmentMapPanel::~EnvironmentMapPanel() {}

        void EnvironmentMapPanel::add_controls()
        {
            TLRENDER_P();

            auto settings = App::app->settings();
            const std::string& prefix = tab_prefix();

            std_any value;
            int v;

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
            b->callback(
                [](Fl_Widget* w, void* d)
                {
                    CollapsibleGroup* cg = static_cast<CollapsibleGroup*>(d);
                    if (cg->is_open())
                        cg->close();
                    else
                        cg->open();

                    const std::string& prefix =
                        environmentMapPanel->tab_prefix();
                    const std::string key = prefix + "Type";

                    App* app = App::ui->app;
                    auto settings = app->settings();
                    settings->setValue(key, static_cast<int>(cg->is_open()));

                    environmentMapPanel->refresh();
                },
                cg);

            cg->begin();

            Fl_Flex* flex = new Fl_Flex(g->x(), 20, g->w(), 20);
            flex->type(Fl_Flex::HORIZONTAL);

            flex->begin();

            auto rB = new Widget< Fl_Radio_Round_Button >(
                g->x(), 90, g->w(), 20, _("None"));
            r = _r->none = rB;
            r->labelsize(12);
            if (o.type == EnvironmentMapOptions::kNone)
                r->value(1);
            else
                r->value(0);
            r->tooltip(_("Turn off image warping."));
            rB->callback(
                [=](auto w)
                {
                    auto view = p.ui->uiView;
                    EnvironmentMapOptions o = view->getEnvironmentMapOptions();
                    o.type = EnvironmentMapOptions::kNone;
                    view->setEnvironmentMapOptions(o);
                    view->setActionMode(ActionMode::kScrub);
                });

            rB = new Widget< Fl_Radio_Round_Button >(
                g->x(), 90, g->w(), 20, _("Spherical"));
            r = _r->spherical = rB;
            r->labelsize(12);
            if (o.type == EnvironmentMapOptions::kSpherical)
                r->value(1);
            else
                r->value(0);
            r->tooltip(_("Wrap the image or images onto a sphere."));
            rB->callback(
                [=](auto w)
                {
                    auto view = p.ui->uiView;
                    EnvironmentMapOptions o = view->getEnvironmentMapOptions();
                    o.type = EnvironmentMapOptions::kSpherical;
                    view->setEnvironmentMapOptions(o);
                    view->setActionMode(ActionMode::kRotate);
                });

            rB = new Widget< Fl_Radio_Round_Button >(
                g->x(), 90, g->w(), 20, _("Cubic"));
            r = _r->cubic = rB;
            r->labelsize(12);
            if (o.type == EnvironmentMapOptions::kCubic)
                r->value(1);
            else
                r->value(0);
            r->tooltip(_("Wrap the image or images onto a cube."));
            rB->callback(
                [=](auto w)
                {
                    auto view = p.ui->uiView;
                    EnvironmentMapOptions o = view->getEnvironmentMapOptions();
                    o.type = EnvironmentMapOptions::kCubic;
                    view->setEnvironmentMapOptions(o);
                    view->setActionMode(ActionMode::kRotate);
                });

            flex->end();

            cg->end();

            std::string key = prefix + "Type";
            value = settings->getValue<std::any>(key);
            int open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
            if (!open)
                cg->close();

            cg = new CollapsibleGroup(g->x(), 20, g->w(), 20, _("Projection"));
            b = cg->button();
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

                    const std::string& prefix =
                        environmentMapPanel->tab_prefix();
                    const std::string key = prefix + "Projection";

                    App* app = App::ui->app;
                    auto settings = app->settings();
                    settings->setValue(key, static_cast<int>(cg->is_open()));

                    environmentMapPanel->refresh();
                },
                cg);

            cg->begin();

            auto sV = new Widget< HorSlider >(
                g->x(), 90, g->w(), 20, _("   H. Aperture"));
            s = _r->hAperture = sV;
            s->tooltip(_("Horizontal Aperture of the Projection."));
            s->range(0.001f, 90.0f);
            s->step(0.01F);
            s->default_value(24.0f);
            sV->callback(
                [=](auto w)
                {
                    auto view = p.ui->uiView;
                    EnvironmentMapOptions o = view->getEnvironmentMapOptions();
                    o.horizontalAperture = w->value();
                    view->setEnvironmentMapOptions(o);
                });

            sV = new Widget< HorSlider >(
                g->x(), 90, g->w(), 20, _("    V. Aperture"));
            s = _r->vAperture = sV;
            s->tooltip(_("Vertical Aperture of the Projection."));
            s->range(0.0f, 90.0f);
            s->step(0.1F);
            s->default_value(0.f);
            sV->callback(
                [=](auto w)
                {
                    auto view = p.ui->uiView;
                    EnvironmentMapOptions o = view->getEnvironmentMapOptions();
                    o.verticalAperture = w->value();
                    view->setEnvironmentMapOptions(o);
                });

            sV = new Widget< HorSlider >(
                g->x(), 90, g->w(), 20, _("Focal Length"));
            s = _r->focalLength = sV;
            s->tooltip(_(
                "Focal Length of the Projection. Use Shift + left mouse button"
                " to change or the mousewheel."));
            s->range(0.0001f, 180.0f);
            s->step(0.1F);
            s->default_value(45.f);
            sV->callback(
                [=](auto w)
                {
                    auto view = p.ui->uiView;
                    EnvironmentMapOptions o = view->getEnvironmentMapOptions();
                    o.focalLength = w->value();
                    view->setEnvironmentMapOptions(o);
                });

            cg->end();

            key = prefix + "Projection";
            value = settings->getValue<std::any>(key);
            open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
            if (!open)
                cg->close();

            cg = new CollapsibleGroup(g->x(), 20, g->w(), 20, _("Rotation"));
            b = cg->button();
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

                    const std::string& prefix =
                        environmentMapPanel->tab_prefix();
                    const std::string key = prefix + "Rotation";

                    App* app = App::ui->app;
                    auto settings = app->settings();
                    settings->setValue(key, static_cast<int>(cg->is_open()));

                    environmentMapPanel->refresh();
                },
                cg);

            cg->begin();

            auto cB = new Widget< Fl_Check_Button >(
                g->x(), 90, g->w(), 20, _("Spin"));
            c = _r->spin = cB;
            c->labelsize(12);
            c->tooltip(
                _("Spin with middle mouse instead of rotating with it."));

            value = settings->getValue<std::any>("EnvironmentMap/Spin");
            v = std_any_empty(value) ? 0 : std_any_cast<int>(value);
            c->value(v);
            auto view = p.ui->uiView;

            o.spin = v;
            view->setEnvironmentMapOptions(o);

            cB->callback(
                [=](auto w)
                {
                    auto view = p.ui->uiView;
                    EnvironmentMapOptions o = view->getEnvironmentMapOptions();
                    o.spin = w->value();
                    settings->setValue("EnvironmentMap/Spin", (int)o.spin);
                    view->setEnvironmentMapOptions(o);
                });

            sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, "X");
            s = _r->rotateX = sV;
            s->tooltip(_("Rotation in X of the projection.  Use middle mouse "
                         "button to move around."));
            s->range(-90.f, 90.0f);
            s->default_value(0.0f);
            sV->callback(
                [=](auto w)
                {
                    auto view = p.ui->uiView;
                    EnvironmentMapOptions o = view->getEnvironmentMapOptions();
                    o.rotateX = w->value();
                    view->setEnvironmentMapOptions(o);
                });

            sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, "Y");
            s = _r->rotateY = sV;
            s->tooltip(_("Rotation in Y of the projection.  Use middle mouse "
                         "button to move around."));
            s->range(-180.f, 180.0f);
            s->default_value(0.0f);
            sV->callback(
                [=](auto w)
                {
                    auto view = p.ui->uiView;
                    EnvironmentMapOptions o = view->getEnvironmentMapOptions();
                    o.rotateY = w->value();
                    view->setEnvironmentMapOptions(o);
                });

            cg->end();

            key = prefix + "Rotation";
            value = settings->getValue<std::any>(key);
            open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
            if (!open)
                cg->close();

            cg =
                new CollapsibleGroup(g->x(), 20, g->w(), 20, _("Subdivisions"));
            b = cg->button();
            b->labelsize(14);
            b->size(b->w(), 18);
            b->tooltip(_(
                "Subdivision of the sphere when doing spherical projections"));
            b->callback(
                [](Fl_Widget* w, void* d)
                {
                    CollapsibleGroup* cg = static_cast<CollapsibleGroup*>(d);
                    if (cg->is_open())
                        cg->close();
                    else
                        cg->open();

                    const std::string& prefix =
                        environmentMapPanel->tab_prefix();
                    const std::string key = prefix + "Subdivisions";

                    App* app = App::ui->app;
                    auto settings = app->settings();
                    settings->setValue(key, static_cast<int>(cg->is_open()));

                    environmentMapPanel->refresh();
                },
                cg);

            cg->begin();

            sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, "X");
            s = _r->subdivisionX = sV;
            s->tooltip(_("Subdivision of the sphere in X."));
            s->range(4.0f, 90.0f);
            s->step(1);

            value = settings->getValue<std::any>(
                "EnvironmentMap/Sphere/SubdivisionX");
            v = std_any_empty(value) ? 36 : std_any_cast<int>(value);

            s->default_value(v);
            sV->callback(
                [=](auto w)
                {
                    auto view = p.ui->uiView;
                    EnvironmentMapOptions o = view->getEnvironmentMapOptions();
                    int v = static_cast<int>(w->value());
                    o.subdivisionX = v;
                    settings->setValue("EnvironmentMap/Sphere/SubdivisionX", v);
                    view->setEnvironmentMapOptions(o);
                });

            value = settings->getValue<std::any>(
                "EnvironmentMap/Sphere/SubdivisionY");
            v = std_any_empty(value) ? 36 : std_any_cast<int>(value);

            sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, "Y");
            s = _r->subdivisionY = sV;
            s->tooltip(_("Subdivision of the sphere in Y."));
            s->range(4.0f, 90.0f);
            s->step(1);

            value = settings->getValue<std::any>(
                "EnvironmentMap/Sphere/SubdivisionY");
            v = std_any_empty(value) ? 36 : std_any_cast<int>(value);

            s->default_value(v);
            sV->callback(
                [=](auto w)
                {
                    auto view = p.ui->uiView;
                    EnvironmentMapOptions o = view->getEnvironmentMapOptions();
                    int v = static_cast<int>(w->value());
                    o.subdivisionY = v;
                    settings->setValue("EnvironmentMap/Sphere/SubdivisionY", v);
                    view->setEnvironmentMapOptions(o);
                });

            cg->end();

            key = prefix + "Subdivisions";
            value = settings->getValue<std::any>(key);
            open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
            if (!open)
                cg->close();

            g->end();
        }

        void EnvironmentMapPanel::setEnvironmentMapOptions(
            const EnvironmentMapOptions& value)
        {
            MRV2_R();
            r.none->value(0);
            r.spherical->value(0);
            r.cubic->value(0);
            switch (value.type)
            {
            case EnvironmentMapOptions::kNone:
                r.none->value(1);
                break;
            case EnvironmentMapOptions::kSpherical:
                r.spherical->value(1);
                break;
            case EnvironmentMapOptions::kCubic:
                r.cubic->value(1);
                break;
            }
            r.hAperture->value(value.horizontalAperture);
            r.vAperture->value(value.verticalAperture);
            r.focalLength->value(value.focalLength);
            r.rotateX->value(value.rotateX);
            r.rotateY->value(value.rotateY);
            r.subdivisionX->value(value.subdivisionX);
            r.subdivisionY->value(value.subdivisionY);
            r.spin->value(value.spin);
        }

    } // namespace panel
} // namespace mrv
