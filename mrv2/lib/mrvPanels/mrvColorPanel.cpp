// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <vector>

#include <FL/Fl_Input.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvHorSlider.h"
#include "mrvWidgets/mrvCollapsibleGroup.h"

#include "mrvPanels/mrvPanelsCallbacks.h"
#include "mrvPanels/mrvColorPanel.h"

#include "mrvApp/mrvSettingsObject.h"
#include "mrvApp/mrvFilesModel.h"

#include "mrViewer.h"

namespace mrv
{

    struct ColorPanel::Private
    {
        Fl_Input* lutFilename = nullptr;
        Fl_Choice* lutOrder = nullptr;

        Fl_Check_Button* colorOn = nullptr;
        HorSlider* addSlider = nullptr;
        HorSlider* contrastSlider = nullptr;
        HorSlider* saturationSlider = nullptr;
        HorSlider* tintSlider = nullptr;

        Fl_Check_Button* invert = nullptr;

        Fl_Check_Button* levelsOn = nullptr;
        HorSlider* inLow = nullptr;
        HorSlider* inHigh = nullptr;
        HorSlider* gamma = nullptr;
        HorSlider* outLow = nullptr;
        HorSlider* outHigh = nullptr;

        Fl_Check_Button* softClipOn = nullptr;
        HorSlider* softClip = nullptr;

        std::shared_ptr<
            observer::ListObserver<std::shared_ptr<FilesModelItem> > >
            activeObserver;
    };

    ColorPanel::ColorPanel(ViewerUI* ui) :
        _r(new Private),
        PanelWidget(ui)
    {

        add_group("Color");

        Fl_SVG_Image* svg = load_svg("Color.svg");
        g->image(svg);

        g->callback(
            [](Fl_Widget* w, void* d)
            {
                ViewerUI* ui = static_cast< ViewerUI* >(d);
                delete colorPanel;
                colorPanel = nullptr;
                ui->uiMain->fill_menu(ui->uiMenuBar);
            },
            ui);

        _r->activeObserver =
            observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                ui->app->filesModel()->observeActive(),
                [this](
                    const std::vector< std::shared_ptr<FilesModelItem> >& value)
                { redraw(); });
    }

    ColorPanel::~ColorPanel() {}

    void ColorPanel::add_controls()
    {
        TLRENDER_P();

        g->clear();
        g->begin();

        SettingsObject* settingsObject = p.ui->app->settingsObject();
        std::string prefix = tab_prefix();

        // ---------------------------- LUT

        CollapsibleGroup* cg =
            new CollapsibleGroup(g->x(), 20, g->w(), 20, _("LUT"));
        Fl_Button* b = cg->button();
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

                const std::string& prefix = colorPanel->tab_prefix();
                const std::string key = prefix + "LUT";

                App* app = App::application();
                auto settingsObject = app->settingsObject();
                settingsObject->setValue(key, static_cast<int>(cg->is_open()));

                colorPanel->refresh();
            },
            cg);

        cg->begin();

        Fl_Group* gb = new Fl_Group(g->x(), 40, g->w(), 20);
        gb->begin();

        Fl_Input* i;
        int X = 100 * g->w() / 270;
        auto iW = new Widget<Fl_Input>(
            g->x() + X, 40, g->w() - X - 30, 20, _("Filename"));
        i = _r->lutFilename = iW;
        i->color((Fl_Color)0xf98a8a800);
        i->textcolor((Fl_Color)56);
        i->labelsize(12);
        iW->callback(
            [=](auto o)
            {
                auto lutOptions = p.ui->app->lutOptions();
                std::string file = o->value();
                lutOptions.fileName = file;
                p.ui->app->setLUTOptions(lutOptions);
            });

        auto bW = new Widget<Fl_Button>(
            g->x() + g->w() - 30, 40, 30, 20, "@fileopen");
        b = bW;
        b->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
        bW->callback(
            [=](auto t)
            {
                std::string file =
                    open_lut_file(_r->lutFilename->value(), p.ui);
                if (!file.empty())
                {
                    _r->lutFilename->value(file.c_str());
                    _r->lutFilename->do_callback();
                }
            });

        gb->end();

        gb = new Fl_Group(g->x(), 20, g->w(), 20);
        gb->begin();
        auto mW = new Widget< Fl_Choice >(
            g->x() + 100, 21, g->w() - 100, 20, _("Order"));
        Fl_Choice* m = _r->lutOrder = mW;
        m->labelsize(12);
        m->align(FL_ALIGN_LEFT);
        m->add("PostColorConfig");
        m->add("PreColorConfig");
        m->value(0);
        mW->callback(
            [=](auto o)
            {
                timeline::LUTOrder order = (timeline::LUTOrder)o->value();
                auto lutOptions = p.ui->app->lutOptions();
                lutOptions.order = order;
                p.ui->app->setLUTOptions(lutOptions);
            });

        gb->end();

        cg->end();

        std::string key = prefix + "LUT";
        std_any value = settingsObject->value(key);
        int open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
        if (!open)
            cg->close();

        timeline::DisplayOptions o = p.ui->app->displayOptions();

        // ---------------------------- Color Controls

        cg = new CollapsibleGroup(g->x(), 20, g->w(), 20, _("Color Controls"));
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

                const std::string& prefix = colorPanel->tab_prefix();
                const std::string key = prefix + "Color Controls";

                App* app = App::application();
                auto settingsObject = app->settingsObject();
                settingsObject->setValue(key, static_cast<int>(cg->is_open()));

                colorPanel->refresh();
            },
            cg);

        cg->begin();

        Fl_Check_Button* c;
        HorSlider* s;
        auto cV = new Widget< Fl_Check_Button >(
            g->x() + 90, 50, g->w(), 20, _("Enabled"));
        c = _r->colorOn = cV;
        c->value(o.colorEnabled);
        c->labelsize(12);
        cV->callback(
            [=](auto w)
            {
                timeline::DisplayOptions o = p.ui->app->displayOptions();
                o.colorEnabled = w->value();
                p.ui->app->setDisplayOptions(o);
            });

        auto sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, _("Add"));
        s = _r->addSlider = sV;
        s->step(0.01f);
        s->range(0.f, 1.0f);
        s->default_value(0.0f);
        s->value(o.color.add.x);
        sV->callback(
            [=](auto w)
            {
                timeline::DisplayOptions o = p.ui->app->displayOptions();
                float f = w->value();
                o.colorEnabled = true;
                o.color.add = math::Vector3f(f, f, f);
                p.ui->app->setDisplayOptions(o);
            });

        sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, _("Contrast"));
        s = _r->contrastSlider = sV;
        s->range(0.f, 4.0f);
        s->default_value(1.0f);
        s->value(o.color.contrast.x);
        sV->callback(
            [=](auto w)
            {
                timeline::DisplayOptions o = p.ui->app->displayOptions();
                float f = w->value();
                o.colorEnabled = true;
                o.color.contrast = math::Vector3f(f, f, f);
                p.ui->app->setDisplayOptions(o);
            });

        sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, _("Saturation"));
        s = _r->saturationSlider = sV;
        s->range(0.f, 4.0f);
        s->default_value(1.0f);
        s->value(o.color.saturation.x);
        sV->callback(
            [=](auto w)
            {
                timeline::DisplayOptions o = p.ui->app->displayOptions();
                float f = w->value();
                o.colorEnabled = true;
                o.color.saturation = math::Vector3f(f, f, f);
                p.ui->app->setDisplayOptions(o);
            });

        sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, _("Tint"));
        s = _r->tintSlider = sV;
        s->range(0.f, 1.0f);
        s->step(0.01);
        s->default_value(0.0f);
        s->value(o.color.tint);
        sV->callback(
            [=](auto w)
            {
                timeline::DisplayOptions o = p.ui->app->displayOptions();
                o.colorEnabled = true;
                o.color.tint = w->value();
                p.ui->app->setDisplayOptions(o);
            });

        cV = new Widget< Fl_Check_Button >(
            g->x() + 90, 50, g->w(), 20, _("Invert"));
        c = _r->invert = cV;
        c->labelsize(12);
        c->value(o.color.invert);
        cV->callback(
            [=](auto w)
            {
                timeline::DisplayOptions o = p.ui->app->displayOptions();
                o.colorEnabled = true;
                o.color.invert = w->value();
                p.ui->app->setDisplayOptions(o);
            });

        cg->end();

        key = prefix + "Color Controls";
        value = settingsObject->value(key);
        open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
        if (!open)
            cg->close();

        // ---------------------------- Levels

        cg = new CollapsibleGroup(g->x(), 180, g->w(), 20, _("Levels"));
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

                const std::string& prefix = colorPanel->tab_prefix();
                const std::string key = prefix + "Levels";

                App* app = App::application();
                auto settingsObject = app->settingsObject();
                settingsObject->setValue(key, static_cast<int>(cg->is_open()));

                colorPanel->refresh();
            },
            cg);

        cg->begin();

        cV = new Widget< Fl_Check_Button >(
            g->x() + 90, 50, g->w(), 20, _("Enabled"));
        c = _r->levelsOn = cV;
        c->value(o.levelsEnabled);
        c->labelsize(12);
        cV->callback(
            [=](auto w)
            {
                timeline::DisplayOptions o = p.ui->app->displayOptions();
                o.levelsEnabled = w->value();
                p.ui->app->setDisplayOptions(o);
            });

        sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, _("In Low"));
        s = _r->inLow = sV;
        s->range(0.f, 1.0f);
        s->step(0.01);
        s->default_value(0.0f);
        s->value(o.levels.inLow);
        sV->callback(
            [=](auto w)
            {
                timeline::DisplayOptions o = p.ui->app->displayOptions();
                o.levelsEnabled = true;
                o.levels.inLow = w->value();
                p.ui->app->setDisplayOptions(o);
            });

        sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, _("In High"));
        s = _r->inHigh = sV;
        s->range(0.f, 1.0f);
        s->step(0.01);
        s->default_value(1.0f);
        s->value(o.levels.inHigh);
        sV->callback(
            [=](auto w)
            {
                timeline::DisplayOptions o = p.ui->app->displayOptions();
                o.levelsEnabled = true;
                o.levels.inHigh = w->value();
                p.ui->app->setDisplayOptions(o);
            });

        sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, _("Gamma"));
        s = _r->gamma = sV;
        s->range(0.f, 6.0f);
        s->step(0.01);
        s->default_value(1.0f);
        s->value(p.ui->uiGamma->value());

        sV->callback(
            [=](auto w)
            {
                timeline::DisplayOptions o = p.ui->app->displayOptions();
                float f = w->value();
                p.ui->uiGamma->value(f);
                p.ui->uiGammaInput->value(f);
                o.levelsEnabled = true;
                o.levels.gamma = f;
                p.ui->app->setDisplayOptions(o);
            });

        sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, _("Out Low"));
        s = _r->outLow = sV;
        s->range(0.f, 1.0f);
        s->step(0.01);
        s->default_value(0.0f);
        s->value(o.levels.outLow);
        sV->callback(
            [=](auto w)
            {
                timeline::DisplayOptions o = p.ui->app->displayOptions();
                o.levelsEnabled = true;
                o.levels.outLow = w->value();
                p.ui->app->setDisplayOptions(o);
            });

        sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, _("Out High"));
        s = _r->outHigh = sV;
        s->range(0.f, 1.0f);
        s->step(0.01);
        s->default_value(1.0f);
        s->value(o.levels.outHigh);
        sV->callback(
            [=](auto w)
            {
                timeline::DisplayOptions o = p.ui->app->displayOptions();
                o.levelsEnabled = true;
                o.levels.outHigh = w->value();
                p.ui->app->setDisplayOptions(o);
            });

        cg->end();

        key = prefix + "Levels";
        value = settingsObject->value(key);
        open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
        if (!open)
            cg->close();

        cg = new CollapsibleGroup(g->x(), 180, g->w(), 20, _("Soft Clip"));
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

                const std::string& prefix = colorPanel->tab_prefix();
                const std::string key = prefix + "Soft Clip";

                App* app = App::application();
                auto settingsObject = app->settingsObject();
                settingsObject->setValue(key, static_cast<int>(cg->is_open()));

                colorPanel->refresh();
            },
            cg);

        cg->begin();

        cV = new Widget< Fl_Check_Button >(
            g->x() + 90, 120, g->w(), 20, _("Enabled"));
        c = _r->softClipOn = cV;
        c->labelsize(12);
        c->value(o.softClipEnabled);
        cV->callback(
            [=](auto w)
            {
                timeline::DisplayOptions o = p.ui->app->displayOptions();
                o.softClipEnabled = w->value();
                p.ui->app->setDisplayOptions(o);
            });

        sV = new Widget< HorSlider >(g->x(), 140, g->w(), 20, _("Soft Clip"));
        s = _r->softClip = sV;
        s->range(0.f, 1.0f);
        s->step(0.01);
        s->default_value(0.0f);
        s->value(o.softClip);
        sV->callback(
            [=](auto w)
            {
                timeline::DisplayOptions o = p.ui->app->displayOptions();
                o.softClipEnabled = true;
                o.softClip = w->value();
                p.ui->app->setDisplayOptions(o);
            });

        cg->end();

        key = prefix + "Soft Clip";
        value = settingsObject->value(key);
        open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
        if (!open)
            cg->close();
    }

    void ColorPanel::redraw() noexcept
    {
        // Change of movie file.  Refresh colors by calling all widget callbacks

        std::string lutFile = _r->lutFilename->value();
        if (!lutFile.empty())
        {
            _r->lutOrder->do_callback();
        }

        if (_r->colorOn->value())
        {
            _r->addSlider->do_callback();
            _r->contrastSlider->do_callback();
            _r->saturationSlider->do_callback();
            _r->tintSlider->do_callback();
        }

        if (_r->levelsOn->value())
        {
            _r->inLow->do_callback();
            _r->inHigh->do_callback();
            _r->gamma->do_callback();
            _r->outLow->do_callback();
            _r->outHigh->do_callback();
        }

        if (_r->softClipOn->value())
        {
            _r->softClip->do_callback();
        }
    }

    void ColorPanel::setLUTOptions(const timeline::LUTOptions& value)
    {
        std::string lutFile = value.fileName;
        _r->lutFilename->value(lutFile.c_str());
        if (!lutFile.empty())
        {
            _r->lutOrder->value(static_cast<int>(value.order));
            _r->lutOrder->do_callback();
        }
    }

    void ColorPanel::setDisplayOptions(const timeline::DisplayOptions& value)
    {
        _r->colorOn->value(value.colorEnabled);
        _r->addSlider->value(value.color.add.x);
        _r->contrastSlider->value(value.color.contrast.x);
        _r->saturationSlider->value(value.color.saturation.x);
        _r->tintSlider->value(value.color.tint);
        _r->invert->value(value.color.invert);

        _r->levelsOn->value(value.levelsEnabled);
        _r->inLow->value(value.levels.inLow);
        _r->inHigh->value(value.levels.inHigh);
        _r->gamma->value(value.levels.gamma);
        _r->outLow->value(value.levels.outLow);
        _r->outHigh->value(value.levels.outHigh);

        _r->softClipOn->value(value.softClipEnabled);
        _r->softClip->value(value.softClip);
    }

} // namespace mrv
