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

#include "mrvApp/mrvFilesModel.h"

#include "mrViewer.h"

namespace mrv
{

    struct ColorPanel::Private
    {
        Fl_Input* lutFilename = nullptr;
        Fl_Choice*         lutOrder = nullptr;

        Fl_Check_Button* colorOn = nullptr;
        HorSlider*        addSlider = nullptr;
        HorSlider*   contrastSlider = nullptr;
        HorSlider* saturationSlider = nullptr;
        HorSlider*       tintSlider = nullptr;

        Fl_Check_Button*   invert   = nullptr;


        Fl_Check_Button* levelsOn = nullptr;
        HorSlider*         inLow    = nullptr;
        HorSlider*         inHigh   = nullptr;
        HorSlider*           gamma  = nullptr;
        HorSlider*         outLow   = nullptr;
        HorSlider*         outHigh  = nullptr;
        
        Fl_Check_Button* softClipOn = nullptr;
        HorSlider*        softClip  = nullptr;

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
                { refresh(); });
    }

    ColorPanel::~ColorPanel() {}

    void ColorPanel::add_controls()
    {
        TLRENDER_P();

        CollapsibleGroup* cg =
            new CollapsibleGroup(g->x(), 20, g->w(), 20, "LUT");
        Fl_Button* b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);

        cg->begin();

        Fl_Group* gb = new Fl_Group(g->x(), 20, g->w(), 20);
        gb->begin();

        Fl_Input* i;
        int X = 100 * g->w() / 270;
        auto iW = new Widget<Fl_Input>(
            g->x() + X, 20, g->w() - g->x() - X - 30, 20, "Filename");
        i = _r->lutFilename = iW;
        i->color((Fl_Color)0xf98a8a800);
        i->textcolor((Fl_Color)56);
        i->labelsize(12);
        iW->callback(
            [=](auto o)
            {
                std::string file = o->value();
                auto& lutOptions = p.ui->uiView->lutOptions();
                lutOptions.fileName = file;
                if (p.ui->uiSecondary)
                {
                    Viewport* view = p.ui->uiSecondary->viewport();
                    view->setLUTOptions(lutOptions);
                    view->redraw();
                }
                p.ui->uiView->redraw();
            });

        auto bW = new Widget<Fl_Button>(
            g->x() + g->w() - 30, 20, 30, 20, "@fileopen");
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
            g->x() + 100, 21, g->w() - 100, 20, "Order");
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
                auto& lutOptions = p.ui->uiView->lutOptions();
                lutOptions.order = order;
                if (p.ui->uiSecondary)
                {
                    Viewport* view = p.ui->uiSecondary->viewport();
                    view->setLUTOptions(lutOptions);
                    view->redraw();
                }
                p.ui->uiView->redraw();
            });

        gb->end();

        cg->end();

        cg = new CollapsibleGroup(g->x(), 20, g->w(), 20, "Color Controls");
        b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);

        cg->begin();

        Fl_Check_Button* c;
        HorSlider* s;
        auto cV = new Widget< Fl_Check_Button >(
            g->x() + 90, 50, g->w(), 20, "Enabled");
        c = _r->colorOn = cV;
        c->labelsize(12);
        cV->callback(
            [=](auto w)
            {
                timeline::DisplayOptions& o =
                    p.ui->uiView->getDisplayOptions(0);
                o.colorEnabled = w->value();
                if (p.ui->uiSecondary)
                {
                    Viewport* view = p.ui->uiSecondary->viewport();
                    timeline::DisplayOptions& o = view->getDisplayOptions(0);
                    o.colorEnabled = w->value();
                    view->redraw();
                }
                p.ui->uiView->redraw();
            });

        auto sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, "Add");
        s = _r->addSlider = sV;
        s->step(0.01f);
        s->range(0.f, 1.0f);
        s->default_value(0.0f);
        sV->callback(
            [=](auto w)
            {
                _r->colorOn->value(1);
                _r->colorOn->do_callback();
                timeline::DisplayOptions& o =
                    p.ui->uiView->getDisplayOptions(0);
                float f = w->value();
                o.color.add = math::Vector3f(f, f, f);
                if (p.ui->uiSecondary)
                {
                    Viewport* view = p.ui->uiSecondary->viewport();
                    timeline::DisplayOptions& o = view->getDisplayOptions(0);
                    o.color.add = math::Vector3f(f, f, f);
                    view->redraw();
                }
                p.ui->uiView->redraw();
            });

        sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, "Contrast");
        s = _r->contrastSlider = sV;
        s->range(0.f, 4.0f);
        s->default_value(1.0f);
        sV->callback(
            [=](auto w)
            {
                _r->colorOn->value(1);
                _r->colorOn->do_callback();
                timeline::DisplayOptions& o =
                    p.ui->uiView->getDisplayOptions(0);
                float f = w->value();
                o.color.contrast = math::Vector3f(f, f, f);
                if (p.ui->uiSecondary)
                {
                    Viewport* view = p.ui->uiSecondary->viewport();
                    timeline::DisplayOptions& o = view->getDisplayOptions(0);
                    o.color.contrast = math::Vector3f(f, f, f);
                    view->redraw();
                }
                p.ui->uiView->redraw();
            });

        sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, "Saturation");
        s = _r->saturationSlider = sV;
        s->range(0.f, 4.0f);
        s->default_value(1.0f);
        sV->callback(
            [=](auto w)
            {
                _r->colorOn->value(1);
                _r->colorOn->do_callback();
                timeline::DisplayOptions& o =
                    p.ui->uiView->getDisplayOptions(0);
                float f = w->value();
                o.color.saturation = math::Vector3f(f, f, f);
                if (p.ui->uiSecondary)
                {
                    Viewport* view = p.ui->uiSecondary->viewport();
                    timeline::DisplayOptions& o = view->getDisplayOptions(0);
                    o.color.saturation = math::Vector3f(f, f, f);
                    view->redraw();
                }
                p.ui->uiView->redraw();
            });

        sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, "Tint");
        s = _r->tintSlider = sV;
        s->range(0.f, 1.0f);
        s->step(0.01);
        s->default_value(0.0f);
        sV->callback(
            [=](auto w)
            {
                _r->colorOn->value(1);
                _r->colorOn->do_callback();
                timeline::DisplayOptions& o =
                    p.ui->uiView->getDisplayOptions(0);
                o.color.tint = w->value();
                if (p.ui->uiSecondary)
                {
                    Viewport* view = p.ui->uiSecondary->viewport();
                    timeline::DisplayOptions& o = view->getDisplayOptions(0);
                    o.color.tint = w->value();
                    view->redraw();
                }

                p.ui->uiView->redraw();
            });

        cV = new Widget< Fl_Check_Button >(
            g->x() + 90, 50, g->w(), 20, "Invert");
        c = _r->invert = cV;
        c->labelsize(12);
        cV->callback(
            [=](auto w)
            {
                _r->colorOn->value(1);
                _r->colorOn->do_callback();
                timeline::DisplayOptions& o =
                    p.ui->uiView->getDisplayOptions(0);
                o.color.invert = w->value();
                if (p.ui->uiSecondary)
                {
                    Viewport* view = p.ui->uiSecondary->viewport();
                    timeline::DisplayOptions& o = view->getDisplayOptions(0);
                    o.color.invert = w->value();
                    view->redraw();
                }
                p.ui->uiView->redraw();
            });

        cg->end();

        cg = new CollapsibleGroup(g->x(), 180, g->w(), 20, "Levels");
        b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);
        cg->layout();

        cg->begin();

        cV = new Widget< Fl_Check_Button >(
            g->x() + 90, 50, g->w(), 20, "Enabled");
        c = _r->levelsOn = cV;
        c->labelsize(12);
        cV->callback(
            [=](auto w)
            {
                timeline::DisplayOptions& o =
                    p.ui->uiView->getDisplayOptions(0);
                o.levelsEnabled = w->value();
                if (p.ui->uiSecondary)
                {
                    Viewport* view = p.ui->uiSecondary->viewport();
                    timeline::DisplayOptions& o = view->getDisplayOptions(0);
                    o.levelsEnabled = w->value();
                    view->redraw();
                }
                p.ui->uiView->redraw();
            });

        sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, "In Low");
        s = _r->inLow = sV;
        s->range(0.f, 1.0f);
        s->step(0.01);
        s->default_value(0.0f);
        sV->callback(
            [=](auto w)
            {
                _r->levelsOn->value(1);
                _r->levelsOn->do_callback();
                timeline::DisplayOptions& o =
                    p.ui->uiView->getDisplayOptions(0);
                o.levels.inLow = w->value();
                if (p.ui->uiSecondary)
                {
                    Viewport* view = p.ui->uiSecondary->viewport();
                    timeline::DisplayOptions& o = view->getDisplayOptions(0);
                    o.levels.inLow = w->value();
                    view->redraw();
                }
                p.ui->uiView->redraw();
            });

        sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, "In High");
        s = _r->inHigh = sV;
        s->range(0.f, 1.0f);
        s->step(0.01);
        s->default_value(1.0f);
        sV->callback(
            [=](auto w)
            {
                _r->levelsOn->value(1);
                _r->levelsOn->do_callback();
                timeline::DisplayOptions& o =
                    p.ui->uiView->getDisplayOptions(0);
                o.levels.inHigh = w->value();
                if (p.ui->uiSecondary)
                {
                    Viewport* view = p.ui->uiSecondary->viewport();
                    timeline::DisplayOptions& o = view->getDisplayOptions(0);
                    o.levels.inHigh = w->value();
                    view->redraw();
                }
                p.ui->uiView->redraw();
            });

        sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, "Gamma");
        s = _r->gamma = sV;
        s->range(0.f, 6.0f);
        s->step(0.01);
        s->default_value(1.0f);
        s->value(p.ui->uiGamma->value());

        sV->callback(
            [=](auto w)
            {
                _r->levelsOn->value(1);
                _r->levelsOn->do_callback();
                timeline::DisplayOptions& o =
                    p.ui->uiView->getDisplayOptions(0);
                float f = w->value();
                o.levels.gamma = f;
                if (p.ui->uiSecondary)
                {
                    Viewport* view = p.ui->uiSecondary->viewport();
                    timeline::DisplayOptions& o = view->getDisplayOptions(0);
                    o.levels.gamma = f;
                    view->redraw();
                }
                p.ui->uiGamma->value(f);
                p.ui->uiGammaInput->value(f);
                p.ui->uiView->redraw();
            });

        sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, "Out Low");
        s = _r->outLow = sV;
        s->range(0.f, 1.0f);
        s->step(0.01);
        s->default_value(0.0f);
        sV->callback(
            [=](auto w)
            {
                _r->levelsOn->value(1);
                _r->levelsOn->do_callback();
                timeline::DisplayOptions& o =
                    p.ui->uiView->getDisplayOptions(0);
                o.levels.outLow = w->value();
                if (p.ui->uiSecondary)
                {
                    Viewport* view = p.ui->uiSecondary->viewport();
                    timeline::DisplayOptions& o = view->getDisplayOptions(0);
                    o.levels.outLow = w->value();
                    view->redraw();
                }
                p.ui->uiView->redraw();
            });

        sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, "Out High");
        s = _r->outHigh = sV;
        s->range(0.f, 1.0f);
        s->step(0.01);
        s->default_value(1.0f);
        sV->callback(
            [=](auto w)
            {
                _r->levelsOn->value(1);
                _r->levelsOn->do_callback();
                timeline::DisplayOptions& o =
                    p.ui->uiView->getDisplayOptions(0);
                o.levels.outHigh = w->value();
                if (p.ui->uiSecondary)
                {
                    Viewport* view = p.ui->uiSecondary->viewport();
                    timeline::DisplayOptions& o = view->getDisplayOptions(0);
                    o.levels.outHigh = w->value();
                    view->redraw();
                }
                p.ui->uiView->redraw();
            });

        cg->end();

        cg = new CollapsibleGroup(g->x(), 180, g->w(), 20, "Soft Clip");
        b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);
        cg->layout();

        cg->begin();

        cV = new Widget< Fl_Check_Button >(
            g->x() + 90, 120, g->w(), 20, "Enabled");
        c = _r->softClipOn = cV;
        c->labelsize(12);
        cV->callback(
            [=](auto w)
            {
                timeline::DisplayOptions& o =
                    p.ui->uiView->getDisplayOptions(0);
                o.softClipEnabled = w->value();
                if (p.ui->uiSecondary)
                {
                    Viewport* view = p.ui->uiSecondary->viewport();
                    timeline::DisplayOptions& o = view->getDisplayOptions(0);
                    o.softClipEnabled = w->value();
                    view->redraw();
                }
                p.ui->uiView->redraw();
            });

        sV = new Widget< HorSlider >(g->x(), 140, g->w(), 20, "Soft Clip");
        s = _r->softClip = sV;
        s->range(0.f, 1.0f);
        s->step(0.01);
        s->default_value(0.0f);
        sV->callback(
            [=](auto w)
            {
                _r->softClipOn->value(1);
                _r->softClipOn->do_callback();
                timeline::DisplayOptions& o =
                    p.ui->uiView->getDisplayOptions(0);
                o.softClip = w->value();
                if (p.ui->uiSecondary)
                {
                    Viewport* view = p.ui->uiSecondary->viewport();
                    timeline::DisplayOptions& o = view->getDisplayOptions(0);
                    o.softClip = w->value();
                    view->redraw();
                }
                p.ui->uiView->redraw();
            });

        cg->end();
    }

    void ColorPanel::refresh() noexcept
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

    void ColorPanel::setColor(const timeline::Color& value)
    {        
        _r->addSlider->value(value.add.x);
        _r->contrastSlider->value(value.contrast.x);
        _r->saturationSlider->value(value.saturation.x);
        _r->tintSlider->value(value.tint);
        _r->invert->value(value.invert);
        
        if (_r->colorOn->value())
        {
            _r->addSlider->do_callback();
            _r->contrastSlider->do_callback();
            _r->saturationSlider->do_callback();
            _r->tintSlider->do_callback();
            _r->invert->do_callback();
        }
    }
    
    void ColorPanel::setLevels(const timeline::Levels& value)
    {        
        _r->inLow->value(value.inLow);
        _r->inHigh->value(value.inHigh);
        _r->inHigh->value(value.gamma);
        _r->outLow->value(value.outLow);
        _r->outHigh->value(value.outHigh);
            
        if (_r->levelsOn->value())
        {
            _r->inLow->do_callback();
            _r->inHigh->do_callback();
            _r->gamma->do_callback();
            _r->outLow->do_callback();
            _r->outHigh->do_callback();
        }
    }
    
    void ColorPanel::setDisplayOptions(const timeline::DisplayOptions& value)
    {
        _r->colorOn->value(value.colorEnabled);
        setColor(value.color);

        _r->levelsOn->value(value.levelsEnabled);
        setLevels(value.levels);

        _r->softClipOn->value(value.softClipEnabled);
        if (_r->softClipOn->value())
        {
            _r->softClip->value(value.softClip);
            _r->softClip->do_callback();
        }
    }
    
} // namespace mrv
