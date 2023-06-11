// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl_Choice.H>
#include <FL/Fl_Radio_Round_Button.H>

#include "mrViewer.h"

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvHorSlider.h"
#include "mrvWidgets/mrvButton.h"
#include "mrvWidgets/mrvCollapsibleGroup.h"
#include "mrvWidgets/mrvDoubleSpinner.h"
#include "mrvWidgets/mrvMultilineInput.h"

#include "mrvPanels/mrvAnnotationsPanel.h"
#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvApp/mrvSettingsObject.h"

namespace mrv
{

    AnnotationsPanel::AnnotationsPanel(ViewerUI* ui) :
        PanelWidget(ui)
    {
        add_group("Annotations");

        Fl_SVG_Image* svg = load_svg("Annotations.svg");
        g->image(svg);

        g->callback(
            [](Fl_Widget* w, void* d)
            {
                ViewerUI* ui = static_cast< ViewerUI* >(d);
                delete annotationsPanel;
                annotationsPanel = nullptr;
                ui->uiMain->fill_menu(ui->uiMenuBar);
            },
            ui);
    }

    void AnnotationsPanel::add_controls()
    {
        TLRENDER_P();

        SettingsObject* settingsObject = p.ui->app->settingsObject();
        std::string prefix = tab_prefix();

        int X = g->x();
        int Y = 20;

        std_any value;
        Fl_Box* box;
        Pack* sg;
        CollapsibleGroup* cg;
        Fl_Round_Button* r;
        DoubleSpinner* d;
        Fl_Button* b;
        Toggle_Button* bt;
        HorSlider* s;
        Fl_Choice* c;
        Fl_Multiline_Input* mi;

        cg = new CollapsibleGroup(X, Y, g->w(), 40, _("Text"));
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

                const std::string& prefix = annotationsPanel->tab_prefix();
                const std::string key = prefix + "Text";

                App* app = App::application();
                auto settingsObject = app->settingsObject();
                settingsObject->setValue(key, static_cast<int>(cg->is_open()));

                annotationsPanel->refresh();
            },
            cg);

        cg->begin();

        Fl_Group* bg = new Fl_Group(g->x(), Y + 20, g->w(), 40);
        bg->begin();

        bg = new Fl_Group(g->x(), Y + 20, g->w(), 40);
        bg->begin();

        auto cW = new Widget< Fl_Choice >(
            g->x() + 100, Y + 20, g->w() - 100, 20, _("Font"));
        c = cW;
        c->labelsize(12);
        c->align(FL_ALIGN_LEFT);

#ifdef USE_OPENGL2
        auto numFonts = Fl::set_fonts("-*");
        for (unsigned i = 0; i < numFonts; ++i)
        {
            int attrs = 0;
            const char* fontName = Fl::get_font_name((Fl_Font)i, &attrs);
            c->add(fontName);
        }
#else
        const char* kFonts[3] = {
            "NotoSans-Regular", "NotoSans-Bold", "NotoMono-Regular"};

        int numFonts = sizeof(kFonts) / sizeof(char*);
        for (unsigned i = 0; i < numFonts; ++i)
        {
            c->add(kFonts[i]);
        }
#endif

        value = settingsObject->value(kTextFont);
        int font =
            std_any_empty(value) ? FL_HELVETICA : std_any_cast<int>(value);
        if (font > numFonts)
            font = 0;
        c->value(font);
        c->tooltip(_("Selects the current font from the list"));
        cW->callback(
            [=](auto o)
            {
                int font = o->value();
                auto numFonts = Fl::set_fonts("-*");
                settingsObject->setValue(kTextFont, font);
                auto view = p.ui->uiView;
                MultilineInput* w = view->getMultilineInput();
                if (!w)
                    return;
                if (font >= numFonts)
                    font = FL_HELVETICA;
#ifdef USE_OPENGL2
                int attrs = 0;
                const char* fontName = Fl::get_font_name((Fl_Font)font, &attrs);
#else
                const Fl_Menu_Item* item = c->mvalue();
                std::string fontName = item->label();
                w->fontFamily = fontName;
#endif
                w->textfont((Fl_Font)font);
                w->redraw();
                view->redraw();
            });

        auto sV = new Widget< HorSlider >(X, Y + 40, g->w(), 20, _("Size:"));
        s = sV;
        s->range(12, 100);
        s->step(1);
        s->tooltip(_("Selects the current font size."));
        value = settingsObject->value(kFontSize);
        s->default_value(std_any_cast< int >(value));
        sV->callback(
            [=](auto o)
            {
                settingsObject->setValue(
                    kFontSize, static_cast<int>(o->value()));
                const auto& viewportSize = p.ui->uiView->getViewportSize();
                float pct = viewportSize.h / 1024.F;
                MultilineInput* w = p.ui->uiView->getMultilineInput();
                if (!w)
                    return;
                int fontSize = o->value() * pct * p.ui->uiView->viewZoom();
                w->textsize(fontSize);
                w->redraw();
                p.ui->uiView->redrawWindows();
            });

        bg->end();

        cg->end();

        std::string key = prefix + "Text";
        value = settingsObject->value(key);
        int open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
        if (!open)
            cg->close();

        cg = new CollapsibleGroup(X, Y, g->w(), 65, _("Pen"));
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

                const std::string& prefix = annotationsPanel->tab_prefix();
                const std::string key = prefix + "Pen";

                App* app = App::application();
                auto settingsObject = app->settingsObject();
                settingsObject->setValue(key, static_cast<int>(cg->is_open()));

                annotationsPanel->refresh();
            },
            cg);

        cg->begin();

        Fl_Group* pg = new Fl_Group(X, Y, g->w(), 30);
        pg->begin();

        b = penColor = new Fl_Button(X + 100, Y, 25, 25, _("Color:"));
        b->tooltip(_("Selects the current pen color."));
        b->box(FL_EMBOSSED_BOX);
        b->align(FL_ALIGN_LEFT);
        b->color(p.ui->uiPenColor->color());
        b->labelsize(11);
        b->callback((Fl_Callback*)set_pen_color_cb, p.ui);

        value = settingsObject->value(kSoftBrush);
        int soft = std_any_cast<int>(value);

        auto bW = new Widget< Toggle_Button >(X + 150, Y, 25, 25);
        bt = hardBrush = bW;
        bt->selection_color(FL_YELLOW);
        bt->down_box(FL_EMBOSSED_BOX);
        bt->box(FL_FLAT_BOX);
        bt->tooltip(_("Selects a hard brush."));
        Fl_SVG_Image* svg = load_svg("HardBrush.svg");
        bt->image(svg);
        if (!soft)
        {
            bt->value(1);
        }

        bW->callback(
            [=](auto o)
            {
                settingsObject->setValue(kSoftBrush, 0);
                redraw();
            });

        bt = softBrush = bW = new Widget< Toggle_Button >(X + 200, Y, 25, 25);
        bt->tooltip(_("Selects a soft brush."));
        bt->selection_color(FL_YELLOW);
        bt->down_box(FL_EMBOSSED_BOX);
        bt->box(FL_FLAT_BOX);
        svg = load_svg("SoftBrush.svg");
        bt->image(svg);
        if (soft)
        {
            bt->value(1);
        }

        bW->callback(
            [=](auto o)
            {
                settingsObject->setValue(kSoftBrush, 1);
                redraw();
            });

        pg->resizable(0);
        pg->end();

        sV = new Widget< HorSlider >(X, Y + 40, g->w(), 20, _("Pen Size:"));
        s = sV;
        s->range(2, 150);
        s->step(1);
        s->tooltip(_("Selects the current pen size."));
        value = settingsObject->value(kPenSize);
        s->default_value(std_any_cast< int >(value));
        sV->callback(
            [=](auto o)
            {
                settingsObject->setValue(
                    kPenSize, static_cast<int>(o->value()));
                p.ui->uiView->redrawWindows();
            });

        cg->end();

        key = prefix + "Pen";
        value = settingsObject->value(key);
        open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
        if (!open)
            cg->close();

        cg = new CollapsibleGroup(X, Y, g->w(), 20, _("Ghosting"));
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

                const std::string& prefix = annotationsPanel->tab_prefix();
                const std::string key = prefix + "Ghosting";

                App* app = App::application();
                auto settingsObject = app->settingsObject();
                settingsObject->setValue(key, static_cast<int>(cg->is_open()));

                annotationsPanel->refresh();
            },
            cg);

        cg->begin();

        sg = new Pack(X, Y, g->w(), 25);
        sg->type(Pack::HORIZONTAL);
        sg->spacing(5);
        sg->begin();

        box = new Fl_Box(X, Y, 120, 20, _("Previous:"));
        box->labelsize(12);
        box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

        auto dV = new Widget< DoubleSpinner >(X, 40, 50, 20);
        d = dV;
        d->range(1, 50);
        d->step(1);
        d->tooltip(_("Selects the number of fading frames previous to the "
                     "frame of the annotation."));
        value = settingsObject->value(kGhostPrevious);
        d->value(std_any_empty(value) ? 5 : std_any_cast< int >(value));
        dV->callback(
            [=](auto w)
            {
                settingsObject->setValue(
                    kGhostPrevious, static_cast<int>(w->value()));
                p.ui->uiView->setGhostPrevious(static_cast<int>(w->value()));
                p.ui->uiView->redrawWindows();
            });
        sg->end();

        sg = new Pack(X, Y, g->w(), 25);
        sg->type(Pack::HORIZONTAL);
        sg->spacing(5);
        sg->begin();

        box = new Fl_Box(X, Y, 120, 20, _("Next:"));
        box->labelsize(12);
        box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
        dV = new Widget< DoubleSpinner >(X, 60, 50, 20);
        d = dV;
        d->range(1, 50);
        d->step(1);
        d->tooltip(_("Selects the number of fading frames following the frame "
                     "of the annotation."));
        value = settingsObject->value(kGhostNext);
        d->value(std_any_empty(value) ? 5 : std_any_cast< int >(value));
        dV->callback(
            [=](auto w)
            {
                settingsObject->setValue(
                    kGhostNext, static_cast<int>(w->value()));
                p.ui->uiView->setGhostNext(static_cast<int>(w->value()));
                p.ui->uiView->redrawWindows();
            });

        sg->end();

        cg->end();

        key = prefix + "Ghosting";
        value = settingsObject->value(key);
        open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
        if (!open)
            cg->close();

        cg = new CollapsibleGroup(X, 20, g->w(), 20, _("Frames"));
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

                const std::string& prefix = annotationsPanel->tab_prefix();
                const std::string key = prefix + "Frames";

                App* app = App::application();
                auto settingsObject = app->settingsObject();
                settingsObject->setValue(key, static_cast<int>(cg->is_open()));

                annotationsPanel->refresh();
            },
            cg);

        cg->begin();

        auto rV =
            new Widget< Fl_Radio_Round_Button >(X, 40, 50, 20, _("Current"));
        r = rV;
        value = settingsObject->value(kAllFrames);
        r->tooltip(_("Makes the following annotation "
                     "show on this frame only."));
        r->value(std_any_empty(value) ? 1 : !std_any_cast< int >(value));
        rV->callback(
            [=](auto w) {
                settingsObject->setValue(
                    kAllFrames, static_cast<int>(!w->value()));
            });

        rV = new Widget< Fl_Radio_Round_Button >(X, 40, 50, 20, _("All"));
        r = rV;
        r->tooltip(_("Makes the following annotation "
                     "show on all frames."));
        value = settingsObject->value(kAllFrames);
        r->value(std_any_empty(value) ? 0 : std_any_cast< int >(value));
        rV->callback(
            [=](auto w) {
                settingsObject->setValue(
                    kAllFrames, static_cast<int>(w->value()));
            });

        cg->end();

        key = prefix + "Frames";
        value = settingsObject->value(key);
        open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
        if (!open)
            cg->close();

        auto nV =
            new Widget<Fl_Multiline_Input>(X, 40, g->w(), 200, _("Notes"));
        notes = nV;
        notes->align(FL_ALIGN_CENTER | FL_ALIGN_TOP);
        notes->textfont(FL_HELVETICA);
        notes->textsize(16);
        notes->textcolor(FL_BLACK);
        notes->when(FL_WHEN_CHANGED);
        nV->callback(
            [=](auto o)
            {
                const std::string& text = o->value();
                if (text.empty())
                {
                    clear_note_annotation_cb(p.ui);
                }
                else
                {
                    add_note_annotation_cb(p.ui, text);
                }
            });
    }

    void AnnotationsPanel::redraw()
    {
        TLRENDER_P();
        penColor->color(p.ui->uiPenColor->color());
        penColor->redraw();

        std_any value;
        SettingsObject* settingsObject = p.ui->app->settingsObject();
        value = settingsObject->value(kSoftBrush);
        int soft = std_any_cast<int>(value);
        softBrush->value(0);
        hardBrush->value(0);
        if (soft)
        {
            softBrush->value(1);
        }
        else
        {
            hardBrush->value(1);
        }
    }

} // namespace mrv
