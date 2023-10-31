// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/StringFormat.h>

#include <FL/Fl_Input.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Int_Input.H>

#include "mrViewer.h"

#include "mrvCore/mrvMemory.h"

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvHorSlider.h"
#include <mrvWidgets/mrvSpinner.h>
#include "mrvWidgets/mrvCollapsibleGroup.h"

#include "mrvPanels/mrvPanelsCallbacks.h"
#include "mrvPanels/mrvSettingsPanel.h"

#include "mrvFl/mrvIO.h"

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/mrvSettingsObject.h"

namespace
{
    const char* kModule = "settings";
}

namespace mrv
{
    namespace panel
    {

        SettingsPanel::SettingsPanel(ViewerUI* ui) :
            PanelWidget(ui)
        {
            add_group("Settings");

            Fl_SVG_Image* svg = load_svg("Settings.svg");
            g->image(svg);

            g->callback(
                [](Fl_Widget* w, void* d)
                {
                    ViewerUI* ui = static_cast< ViewerUI* >(d);
                    delete settingsPanel;
                    settingsPanel = nullptr;
                    ui->uiMain->fill_menu(ui->uiMenuBar);
                },
                ui);
        }

        void SettingsPanel::add_controls()
        {
            TLRENDER_P();

            SettingsObject* settings = App::app->settings();
            const std::string& prefix = tab_prefix();

            auto cg = new CollapsibleGroup(g->x(), 20, g->w(), 20, _("Cache"));
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

                    const std::string& prefix = settingsPanel->tab_prefix();
                    const std::string key = prefix + "Cache";

                    App* app = App::app;
                    auto settings = app->settings();
                    settings->setValue(key, static_cast<int>(cg->is_open()));

                    settingsPanel->refresh();
                },
                cg);

            cg->begin();

            Fl_Check_Button* c;
            HorSlider* s;
            Spinner* sp;
            int digits;

            uint64_t totalVirtualMem, virtualMemUsed, virtualMemUsedByMe,
                totalPhysMem, physMemUsed, physMemUsedByMe;

            memory_information(
                totalVirtualMem, virtualMemUsed, virtualMemUsedByMe,
                totalPhysMem, physMemUsed, physMemUsedByMe);

            totalPhysMem /= 1024;

            auto sV = new Widget< HorSlider >(
                g->x(), 90, g->w(), 20, _("      Gigabytes"));
            s = sV;
            s->tooltip(
                _("Cache in Gigabytes.  When not 0, it uses the value to "
                  "automatically calculate the Read Ahead and Read Behind"));
            s->step(1.0);
            s->range(0.f, static_cast<double>(totalPhysMem));
            s->default_value(0.0f);
            s->value(settings->getValue<int>("Cache/GBytes"));
            sV->callback(
                [=](auto w)
                {
                    settings->setValue("Cache/GBytes", (int)w->value());
                    App::app->cacheUpdate();
                });

            sV = new Widget< HorSlider >(
                g->x(), 90, g->w(), 20, _("   Read Ahead"));
            s = sV;
            s->tooltip(_("Read Ahead in seconds"));
            s->step(0.1f);
            s->range(0.f, 100.0f);
            s->default_value(timeline::PlayerCacheOptions().readAhead.value());
            s->value(settings->getValue<double>("Cache/ReadAhead"));
            sV->callback(
                [=](auto w)
                {
                    settings->setValue("Cache/ReadAhead", (double)w->value());
                    App::app->cacheUpdate();
                });

            sV = new Widget< HorSlider >(
                g->x(), 90, g->w(), 20, _("Read Behind"));
            s = sV;
            s->tooltip(_("Read Behind in seconds"));
            s->step(0.1f);
            s->range(0.f, 100.0f);
            s->default_value(timeline::PlayerCacheOptions().readBehind.value());
            s->value(settings->getValue<double>("Cache/ReadBehind"));
            sV->callback(
                [=](auto w)
                {
                    settings->setValue("Cache/ReadBehind", (double)w->value());
                    App::app->cacheUpdate();
                });

            cg->end();
            std::string key = prefix + "Cache";
            std_any value = settings->getValue<std::any>(key);
            int open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
            if (!open)
                cg->close();

            cg = new CollapsibleGroup(
                g->x(), 110, g->w(), 20, _("File Sequences"));
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

                    const std::string& prefix = settingsPanel->tab_prefix();
                    const std::string key = prefix + "File Sequences";

                    App* app = App::app;
                    auto settings = app->settings();
                    settings->setValue(key, static_cast<int>(cg->is_open()));

                    settingsPanel->refresh();
                },
                cg);

            cg->begin();

            Fl_Group* bg = new Fl_Group(g->x(), 130, g->w(), 80);
            bg->box(FL_NO_BOX);
            bg->begin();
            auto mW = new Widget< Fl_Choice >(
                g->x() + 130, 130, g->w() - 130, 20, _("Audio"));
            Fl_Choice* m = mW;
            m->labelsize(12);
            m->align(FL_ALIGN_LEFT);
            for (const auto& i : timeline::getFileSequenceAudioLabels())
            {
                m->add(_(i.c_str()));
            }
            m->value(settings->getValue<int>("FileSequence/Audio"));
            mW->callback(
                [=](auto o)
                {
                    int v = o->value();
                    settings->setValue("FileSequence/Audio", v);
                });

            Fl_Input* i;
            auto iW = new Widget<Fl_Input>(
                g->x() + 130, 150, g->w() - 130, 20, _("Audio file name"));
            i = iW;
            i->labelsize(12);
            i->textcolor(FL_BLACK);
            i->cursor_color(FL_RED);
            std::string file =
                settings->getValue<std::string>("FileSequence/AudioFileName");

            i->value(file.c_str());
            iW->callback(
                [=](auto o)
                {
                    std::string file = o->value();
                    settings->setValue("FileSequence/AudioFileName", file);
                });

            iW = new Widget<Fl_Input>(
                g->x() + 130, 170, g->w() - 130, 20, "Audio directory");
            i = iW;
            i->labelsize(12);
            i->textcolor(FL_BLACK);
            i->cursor_color(FL_RED);
            i->value(
                settings->getValue<std::string>("FileSequence/AudioDirectory")
                    .c_str());
            iW->callback(
                [=](auto o)
                {
                    std::string dir = o->value();
                    settings->setValue("FileSequence/AudioDirectory", dir);
                });

            DBG;
            auto inW = new Widget<Fl_Int_Input>(
                g->x() + 130, 190, g->w() - 130, 20, _("Maximum Digits"));
            i = inW;
            i->labelsize(12);
            i->textcolor(FL_BLACK);
            i->cursor_color(FL_RED);
            digits = settings->getValue<int>("Misc/MaxFileSequenceDigits");
            std::string text = string::Format("{0}").arg(digits);
            i->value(text.c_str());
            inW->callback(
                [=](auto o)
                {
                    int digits = atoi(o->value());
                    settings->setValue("Misc/MaxFileSequenceDigits", digits);
                });

            bg->end();

            cg->end();

            key = prefix + "File Sequences";
            value = settings->getValue<std::any>(key);
            open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
            if (!open)
                cg->close();

            cg =
                new CollapsibleGroup(g->x(), 210, g->w(), 20, _("Performance"));
            cg->spacing(2);
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

                    const std::string& prefix = settingsPanel->tab_prefix();
                    const std::string key = prefix + "Performance";

                    App* app = App::app;
                    auto settings = app->settings();
                    settings->setValue(key, static_cast<int>(cg->is_open()));

                    settingsPanel->refresh();
                },
                cg);

            cg->begin();

            bg = new Fl_Group(g->x(), 230, g->w(), 22 * 7);
            bg->box(FL_NO_BOX);
            bg->begin();

            Fl_Box* box = new Fl_Box(
                g->x(), 230, g->w(), 40,
                _("Changes are applied to "
                  "newly opened files."));
            box->labelsize(12);
            box->align(FL_ALIGN_WRAP);

            mW = new Widget< Fl_Choice >(
                g->x() + 130, 270, g->w() - 130, 20, _("Timer mode"));
            m = mW;
            m->labelsize(12);
            m->align(FL_ALIGN_LEFT);
            for (const auto& i : timeline::getTimerModeLabels())
            {
                m->add(i.c_str());
            }

            m->value(settings->getValue<int>("Performance/TimerMode"));

            mW->callback(
                [=](auto o)
                {
                    int v = o->value();
                    settings->setValue("Performance/TimerMode", v);
                });

            auto spW = new Widget< Spinner >(
                g->x() + 160, 294, g->w() - 160, 20, _("Audio buffer frames"));
            sp = spW;
            sp->step(1);
            sp->range(1024, 4096);
            sp->align(FL_ALIGN_LEFT);
            int v =
                settings->getValue<int>("Performance/AudioBufferFrameCount");
            sp->value(v);

            spW->callback(
                [=](auto o)
                {
                    int v = static_cast<int>(o->value());
                    settings->setValue("Performance/AudioBufferFrameCount", v);
                });

            spW = new Widget< Spinner >(
                g->x() + 160, 318, g->w() - 160, 20, _("Video Requests"));
            sp = spW;
            // sp->range( 1, 64 );
            digits = settings->getValue<int>("Performance/VideoRequestCount");
            sp->value(digits);

            spW->callback(
                [=](auto o)
                {
                    int requests = static_cast<int>(o->value());
                    settings->setValue(
                        "Performance/VideoRequestCount", requests);
                    App::app->cacheUpdate();
                });

            spW = new Widget<Spinner>(
                g->x() + 160, 342, g->w() - 160, 20, _("Audio Requests"));
            sp = spW;
            // sp->irange( 1, 64 );
            digits = settings->getValue<int>("Performance/AudioRequestCount");
            sp->value(digits);
            spW->callback(
                [=](auto o)
                {
                    int requests = static_cast<int>(o->value());
                    settings->setValue(
                        "Performance/AudioRequestCount", requests);
                    App::app->cacheUpdate();
                });

            spW = new Widget<Spinner>(
                g->x() + 160, 366, g->w() - 160, 20, _("Sequence I/O threads"));
            sp = spW;
            // sp->irange( 1, 64 );
            digits = settings->getValue<int>("SequenceIO/ThreadCount");
            sp->value(digits);
            spW->callback(
                [=](auto o)
                {
                    int requests = static_cast<int>(o->value());
                    settings->setValue("SequenceIO/ThreadCount", requests);
                    App::app->cacheUpdate();
                });

            bg->end();

            auto cV = new Widget< Fl_Check_Button >(
                g->x() + 90, 398, g->w(), 20,
                _("FFmpeg YUV to RGB conversion"));
            c = cV;
            c->labelsize(12);
            c->value(settings->getValue<bool>(
                "Performance/FFmpegYUVToRGBConversion"));

            cV->callback(
                [=](auto w)
                {
                    int v = w->value();
                    settings->setValue(
                        "Performance/FFmpegYUVToRGBConversion", v);
                });

            bg = new Fl_Group(g->x(), 420, g->w(), 30);
            bg->box(FL_NO_BOX);
            bg->begin();

            spW = new Widget<Spinner>(
                g->x() + 160, 420, g->w() - 160, 20, _("FFmpeg I/O threads"));
            sp = spW;
            digits = settings->getValue<int>("Performance/FFmpegThreadCount");
            sp->value(digits);
            sp->minimum(0);
            sp->maximum(16);
            sp->tooltip(_(
                "This value controls the number of threads that FFmpeg uses.  "
                "For most movies, it should be left at 0.  Some movies will "
                "show "
                "black frames.  For any like that, you should set them to 1, "
                "press Enter and reload the movie file."));
            spW->callback(
                [=](auto o)
                {
                    int requests = static_cast<int>(o->value());
                    settings->setValue(
                        "Performance/FFmpegThreadCount", requests);
                    App::app->cacheUpdate();
                });

            bg->end();

            cg->end();

            key = prefix + "Performance";
            value = settings->getValue<std::any>(key);
            open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
            if (!open)
                cg->close();

            auto bW = new Widget< Fl_Button >(
                g->x(), g->y(), g->w(), 20, _("Default Hotkeys"));
            b = bW;
            b->box(FL_UP_BOX);
            bW->callback(
                [=](auto o)
                {
                    Preferences prefs(p.ui->uiPrefs, false, true);
                    p.ui->uiMain->fill_menu(p.ui->uiMenuBar);
                });

            // This does not work properly, and it is counter intuitive as
            // it hides the tool docks.
            bW = new Widget< Fl_Button >(
                g->x(), g->y(), g->w(), 20, _("Default Settings"));
            b = bW;
            b->box(FL_UP_BOX);
            bW->callback(
                [=](auto o)
                {
                    settings->reset();
                    save();
                    refresh();
                });
            // To refresh the timeline caching bars after a reset of settings.
            p.ui->uiTimeline->redraw();
        }

        void SettingsPanel::refresh()
        {
            begin_group();
            add_controls();
            end_group();
        }

    } // namespace panel

} // namespace mrv
