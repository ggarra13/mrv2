// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#if defined(TLRENDER_NDI)

#    include <chrono>
#    include <regex>
#    include <thread>
#    include <mutex>
#    include <atomic>

#    include <tlCore/StringFormat.h>
#    include <tlCore/NDI.h>

#    include <FL/Fl_Choice.H>
#    include <FL/Fl_Check_Button.H>

#    include <Processing.NDI.Lib.h>

#    include "mrvCore/mrvHome.h"
#    include "mrvCore/mrvFile.h"
#    include "mrvCore/mrvMemory.h"

#    include "mrvWidgets/mrvFunctional.h"
#    include "mrvWidgets/mrvButton.h"
#    include "mrvWidgets/mrvHorSlider.h"
#    include "mrvWidgets/mrvCollapsibleGroup.h"

#    include "mrvPanels/mrvPanelsCallbacks.h"
#    include "mrvPanels/mrvNDIPanel.h"

#    include "mrvFl/mrvIO.h"

#    include "mrvApp/mrvSettingsObject.h"

#    include "mrViewer.h"

namespace
{
    const char* kModule = "ndi";
}

namespace mrv
{
    namespace panel
    {

        struct NDIPanel::Private
        {
            PopupMenu* source = nullptr;
            Fl_Choice* noAudio = nullptr;
            HorSlider* preroll = nullptr;

            NDIlib_find_instance_t NDI_find = nullptr;
            uint32_t no_sources = 0;
            const NDIlib_source_t* p_sources = NULL;

            std::atomic<bool> has_awake = false;
            static std::string lastStream;

            std::thread playThread;

            std::thread findThread;
            std::atomic<bool> running = false;
            std::mutex mutex;
        };

        std::string NDIPanel::Private::lastStream;

        void NDIPanel::refresh_sources_cb(void* v)
        {
            NDIPanel* self = (NDIPanel*)v;
            self->refresh_sources();
        }

        void NDIPanel::refresh_sources()
        {
            MRV2_R();

            std::unique_lock<std::mutex> lock(r.mutex);

            PopupMenu* m = r.source;

            if (m->popped() || !r.running)
            {
                r.has_awake = false;
                return;
            }

            std::string sourceName;
            bool changed = false;
            const Fl_Menu_Item* item = nullptr;

            // Empty menu returns 0, while all others return +1.
            size_t numSources = r.no_sources;

            // We substract 2: 1 for FLTK quirk and one for "No Source".
            int size = m->size() - 2;
            if (size < 0)
                size = 0;

            sourceName = r.lastStream;

            if (numSources != size)
            {
                changed = true;
            }
            else
            {
                // child(0) is "No Source".
                for (int i = 0; i < numSources; ++i)
                {
                    item = m->child(i + 1);
                    if (!item->label() ||
                        !strcmp(item->label(), r.p_sources[i].p_ndi_name))
                    {
                        changed = true;
                        break;
                    }
                }
            }

            if (!changed)
            {
                r.has_awake = false;
                return;
            }

            m->clear();
            m->add(_("No Source"));
            int selected = 0;
            for (int i = 0; i < r.no_sources; ++i)
            {
                const std::string ndiName = r.p_sources[i].p_ndi_name;
                m->add(ndiName.c_str());
                if (sourceName == ndiName)
                {
                    selected = i + 1;
                }
            }
            m->menu_end();
            if (selected >= 0 && selected < m->size())
                m->value(selected);

            r.has_awake = false;
        }

        NDIPanel::NDIPanel(ViewerUI* ui) :
            _r(new Private),
            PanelWidget(ui)
        {
            MRV2_R();

            Fl::lock(); // needed

            add_group("NDI");

            Fl_SVG_Image* svg = load_svg("NDI.svg");
            g->image(svg);

            r.NDI_find = NDIlib_find_create_v2();

            // Run for one minute
            r.findThread = std::thread(
                [this]
                {
                    MRV2_R();

                    r.running = true;
                    while (r.running)
                    {
                        using namespace std::chrono;
                        for (const auto start = high_resolution_clock::now();
                             high_resolution_clock::now() - start <
                             seconds(10);)
                        {
                            // Wait up till 1 second to check for new sources to
                            // be added or removed
                            if (!NDIlib_find_wait_for_sources(
                                    r.NDI_find, 1000 /* milliseconds */))
                            {
                                break;
                            }
                        }

                        if (!r.source)
                            continue;

                        std::unique_lock<std::mutex> lock(r.mutex);

                        r.no_sources = std::numeric_limits<uint32_t>::max();
                        while (r.no_sources ==
                                   std::numeric_limits<uint32_t>::max() &&
                               r.running)
                        {
                            // Get the updated list of sources
                            r.p_sources = NDIlib_find_get_current_sources(
                                r.NDI_find, &r.no_sources);
                        }

                        if (!r.has_awake)
                        {
                            Fl::awake(
                                (Fl_Awake_Handler)refresh_sources_cb, this);
                            r.has_awake = true;
                        }
                    }
                    r.no_sources = 0;
                });

            g->callback(
                [](Fl_Widget* w, void* d)
                {
                    ViewerUI* ui = static_cast< ViewerUI* >(d);
                    delete ndiPanel;
                    ndiPanel = nullptr;
                    ui->uiMain->fill_menu(ui->uiMenuBar);
                },
                ui);
        }

        NDIPanel::~NDIPanel()
        {
            MRV2_R();

            r.has_awake = true;
            r.running = false;

            if (r.findThread.joinable())
                r.findThread.join();

            if (r.NDI_find)
            {
                NDIlib_find_destroy(r.NDI_find);
                r.NDI_find = nullptr;
            }
        }

        void NDIPanel::add_controls()
        {
            TLRENDER_P();
            MRV2_R();

            int LM = 70; // left margin

            SettingsObject* settings = App::app->settings();
            const std::string& prefix = tab_prefix();

            HorSlider* s;
            Fl_Group *bg, *bg2;
            std_any value;
            int open;

            int Y = g->y();

            auto cg = new CollapsibleGroup(g->x(), Y, g->w(), 20, _("NDI"));
            cg->spacing(2);
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

                    const std::string& prefix = ndiPanel->tab_prefix();
                    const std::string key = prefix + "NDI";

                    auto settings = App::app->settings();
                    settings->setValue(key, static_cast<int>(cg->is_open()));
                },
                cg);

            cg->begin();

            bg = new Fl_Group(g->x(), Y, g->w(), 22 * 8);
            bg->box(FL_NO_BOX);
            bg->begin();

            Y += 22;

            bg2 = new Fl_Group(g->x(), Y, g->w(), 22 * 6);
            bg2->box(FL_NO_BOX);
            bg2->begin();

            auto mW = new Widget< PopupMenu >(
                g->x() + 10, Y, g->w() - 10, 20, _("No Source"));
            PopupMenu* m = _r->source = mW;
            m->disable_submenus();
            m->labelsize(12);
            m->align(FL_ALIGN_CENTER);

            mW->callback(
                [=](auto o)
                {
                    const Fl_Menu_Item* item = o->mvalue();
                    if (!item)
                        return;
                    _open_ndi(item);
                });

            r.has_awake = false;

            Y += 30;

            auto spW =
                new Widget< HorSlider >(g->x(), Y, g->w(), 20, _("Preroll"));
            s = _r->preroll = spW;
            s->step(1);
            s->range(1, 10);
            s->default_value(3);
            s->tooltip(_("Preroll in seconds to synchronize audio."));
            s->value(settings->getValue<int>("NDI/Preroll"));
            spW->callback(
                [=](auto w)
                { settings->setValue("NDI/Preroll", (int)w->value()); });

            Y += 30;

            uint64_t totalVirtualMem, virtualMemUsed, virtualMemUsedByMe,
                totalPhysMem, physMemUsed, physMemUsedByMe;

            memory_information(
                totalVirtualMem, virtualMemUsed, virtualMemUsedByMe,
                totalPhysMem, physMemUsed, physMemUsedByMe);

            totalPhysMem /= 1024;

            auto sV =
                new Widget< HorSlider >(g->x(), Y, g->w(), 20, _("Gigabytes"));
            s = sV;
            s->tooltip(
                _("Cache in Gigabytes for NDI streams.  For most HD streams, "
                  "this should be set to 1.  For higher resolutions and audio "
                  "channels, you might want to increase this to 3 or higher "
                  "for proper audio sync."));
            s->step(1.0);
            s->range(1.f, static_cast<double>(totalPhysMem));
            s->default_value(1.0f);
            s->value(settings->getValue<int>("NDI/GBytes"));
            sV->callback(
                [=](auto w)
                { settings->setValue("NDI/GBytes", (int)w->value()); });

            Y += 30;

            auto cW = new Widget< Fl_Choice >(
                g->x() + LM, Y, g->w() - LM, 20, _("Audio"));
            Fl_Choice* c = _r->noAudio = cW;
            c->labelsize(12);
            c->align(FL_ALIGN_LEFT);
            c->add(_("Play"));
            c->add(_("Ignore"));
            c->tooltip(_("Whether to ignore or play the stream with audio if it"
                         " has at least one audio track."));
            c->value(settings->getValue<int>("NDI/Audio"));
            cW->callback([=](auto w)
                         { settings->setValue("NDI/Audio", (int)w->value()); });

            bg2->end();

            bg->end();

            cg->end();

            std::string key = prefix + "NDI";
            value = settings->getValue<std::any>(key);
            open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
            if (!open)
            {
                cg->close();
            }
        }

        void NDIPanel::_open_ndi(const Fl_Menu_Item* item)
        {
            TLRENDER_P();
            MRV2_R();

            // Get the NDI name from the menu item
            const std::string sourceName = item->label();

            if (r.lastStream == sourceName)
                return;

            if (!r.lastStream.empty())
                LOG_INFO("Close stream " << r.lastStream);

            auto model = p.ui->app->filesModel();
            if (model)
            {
                auto aItem = model->observeA()->get();
                if (aItem && file::isTemporaryNDI(aItem->path))
                    model->close();
            }

            std::regex pattern(
                "remote connection", std::regex_constants::icase);
            if (sourceName == _("No Source") ||
                std::regex_search(sourceName, pattern))
                return;

            r.lastStream = sourceName;

            // Create an ndi file
            std::string ndiFile = file::NDI(p.ui);

            ndi::Options options;
            options.sourceName = sourceName;
            options.noAudio = (int)r.noAudio->value();

            nlohmann::json j;
            j = options;

            std::ofstream s(ndiFile);
            s << j << std::endl;
            s.close();

            LOG_INFO("Opened stream " << sourceName);

            open_file_cb(ndiFile, p.ui);

            auto player = p.ui->uiView->getTimelinePlayer();
            if (player)
            {
                int noAudio = r.noAudio->value();

                int seconds = 0;
                if (!noAudio)
                {
                    LOG_INFO(_("Waiting for player cache to fill up..."));
                    p.ui->uiStatusBar->label(
                        _("Waiting for player cache to fill up..."));
                    player->stop();
                    seconds = r.preroll->value();
                }

                r.playThread = std::thread(
                    [this, player, seconds, noAudio]
                    {
                        MRV2_R();

                        if (!noAudio)
                        {
                            auto start = std::chrono::steady_clock::now();
                            auto startTime = player->currentTime();
                            auto endTime =
                                startTime +
                                otime::RationalTime(2.0, 1.0).rescaled_to(
                                    startTime.rate());

                            bool found = false;
                            while (!found &&
                                   std::chrono::steady_clock::now() - start <=
                                       std::chrono::seconds(seconds))
                            {
                                const auto observer = player->player()->observeCacheInfo();
                                const auto cache = observer->get();
                                
                                // Make a copy of the audioFrames vector
                                // to avoid concurrent issues.
                                const auto audioFramesCopy = cache.audioFrames;
                                
                                for (const auto& t : audioFramesCopy)
                                {
                                    if (t.start_time() <= startTime &&
                                        t.end_time_exclusive() >= endTime)
                                    {
                                        found = true;
                                        break;
                                    }
                                }
                            }
                        }
                        // player->start();
                        LOG_INFO(_("Starting playback..."));
                        player->forward();
                    });
                r.playThread.detach();
            }
            p.ui->uiStatusBar->label(_("Everything OK."));
        }

    } // namespace panel

} // namespace mrv

#endif // TLRENDER_NDI
