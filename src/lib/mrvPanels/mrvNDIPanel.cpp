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
#    include <tlCore/NDIOptions.h>

#    include <tlDevice/OutputData.h>
#    include <tlDevice/IOutput.h>

#    include <FL/Fl_Choice.H>

#    include <Processing.NDI.Lib.h>

#    include "mrvCore/mrvHome.h"
#    include "mrvCore/mrvFile.h"
#    include "mrvCore/mrvMemory.h"

#    include "mrvWidgets/mrvToggleButton.h"
#    include "mrvWidgets/mrvFunctional.h"
#    include "mrvWidgets/mrvHorSlider.h"
#    include "mrvWidgets/mrvInput.h"
#    include "mrvWidgets/mrvCollapsibleGroup.h"

#    include "mrvPanels/mrvPanelsCallbacks.h"
#    include "mrvPanels/mrvNDIPanel.h"

#    include "mrvFl/mrvIO.h"

#    include "mrvApp/mrvSettingsObject.h"

#    include "mrViewer.h"

namespace
{
    const char* kModule = "NDI";
}

namespace mrv
{
    namespace panel
    {

        struct NDIPanel::Private
        {

            PopupMenu* sourceMenu = nullptr;
            PopupMenu* inputAudioMenu = nullptr;
            PopupMenu* inputFormatMenu = nullptr;

            PopupMenu* outputAudioMenu = nullptr;
            PopupMenu* outputMetadataMenu = nullptr;
            PopupMenu* outputFormatMenu = nullptr;
            

            uint32_t no_sources = 0;
            const NDIlib_source_t* p_sources = NULL;

            static std::string lastStream;

            std::shared_ptr<observer::ValueObserver<timeline::PlayerCacheInfo> >
                cacheInfoObserver;

            struct PlayThread
            {
                std::atomic<bool> found = false;
                std::thread thread;
            };
            PlayThread play;

            struct FindThread
            {
                NDIlib_find_instance_t NDI = nullptr;
                std::atomic<bool> running = false;
                std::atomic<bool> awake = false;
                std::thread thread;
            };
            FindThread find;

            std::atomic<bool> running = false;
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
            if (!_r)
                return;

            PopupMenu* m = r.sourceMenu;

            if (m->popped() || !r.find.running)
            {
                r.find.awake = false;
                return;
            }

            std::string sourceName;
            bool changed = false;
            const Fl_Menu_Item* item = nullptr;

            // Empty menu returns 0, while all others return +1.
            size_t numSources = r.no_sources;

            // We substract 2: 1 for FLTK quirk and one for "None".
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
                // child(0) is "None".
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
                r.find.awake = false;
                return;
            }

            m->clear();
            m->add(_("None"));
            int selected = 0;
            for (int i = 0; i < r.no_sources; ++i)
            {
                const std::string ndiName = r.p_sources[i].p_ndi_name;

                // Windows has weird items called REMOTE CONNECTION.
                // We don't allow selecting them.
                const std::regex pattern(
                    "remote connection", std::regex_constants::icase);
                if (std::regex_search(ndiName, pattern))
                    continue;

            
                m->add(ndiName.c_str());
                if (sourceName == ndiName)
                {
                    selected = i + 1;
                }
            }
            m->menu_end();
            if (selected >= 0 && selected < m->size())
                m->value(selected);

            r.find.awake = false;
        }

        NDIPanel::NDIPanel(ViewerUI* ui) :
            _r(new Private),
            PanelWidget(ui)
        {
            MRV2_R();

            add_group("NDI");

            Fl_SVG_Image* svg = load_svg("NDI.svg");
            g->bind_image(svg);

            r.find.NDI = NDIlib_find_create_v2();

            // Run for one minute
            r.find.thread = std::thread(
                [this]
                {
                    MRV2_R();

                    r.find.running = true;
                    while (r.find.running)
                    {
                        using namespace std::chrono;
                        for (const auto start = high_resolution_clock::now();
                             high_resolution_clock::now() - start <
                             seconds(10);)
                        {
                            // Wait up till 1 second to check for new sources to
                            // be added or removed
                            if (!NDIlib_find_wait_for_sources(
                                    r.find.NDI, 1000 /* milliseconds */))
                            {
                                break;
                            }
                        }

                        if (!r.sourceMenu)
                            continue;

                        r.no_sources = std::numeric_limits<uint32_t>::max();
                        while (r.no_sources ==
                                   std::numeric_limits<uint32_t>::max() &&
                               r.find.running)
                        {
                            // Get the updated list of sources
                            r.p_sources = NDIlib_find_get_current_sources(
                                r.find.NDI, &r.no_sources);
                        }

                        if (!r.find.awake)
                        {
                            Fl::awake(
                                (Fl_Awake_Handler)refresh_sources_cb, this);
                            r.find.awake = true;
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

            r.find.awake = true;
            r.find.running = false;

            if (r.find.thread.joinable())
                r.find.thread.join();

            if (r.find.NDI)
            {
                NDIlib_find_destroy(r.find.NDI);
                r.find.NDI = nullptr;
            }
        }

        void NDIPanel::add_controls()
        {
            TLRENDER_P();
            MRV2_R();

            SettingsObject* settings = App::app->settings();
            const std::string& prefix = tab_prefix();

            HorSlider* s;
            Fl_Group* ig;
            std_any value;
            int val;
            int open;

            int Y = g->y();

            auto cg = new CollapsibleGroup(g->x(), Y, g->w(), 20, _("Input"));
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
                    const std::string key = prefix + "NDI Input";

                    auto settings = App::app->settings();
                    settings->setValue(key, static_cast<int>(cg->is_open()));
                },
                cg);

            cg->begin();

            Fl_Box* title = new Fl_Box(
                g->x() + 10, Y, g->w() - 20, 20, _("NDI Connection"));
            title->align(FL_ALIGN_CENTER);
            title->labelsize(12);

            auto mW = new Widget< PopupMenu >(
                g->x() + 10, Y, g->w() - 20, 30, _("None"));
            PopupMenu* m = r.sourceMenu = mW;
            m->disable_submenus();
            m->labelsize(12);
            m->align(FL_ALIGN_CENTER | FL_ALIGN_CLIP);

            mW->callback(
                [=](auto o)
                {
                    if (o->size() < 3)
                        return;
                    const Fl_Menu_Item* item = o->mvalue();
                    if (!item || !item->label())
                        return;
                    _ndi_input(item);
                });

            mW = new Widget< PopupMenu >(
                g->x() + 10, Y, g->w() - 20, 20, _("Fast Format"));
            m = r.inputFormatMenu = mW;
            m->disable_submenus();
            m->labelsize(12);
            m->align(FL_ALIGN_CENTER | FL_ALIGN_CLIP);
            m->add(_("Fast Format"));
            m->add(_("Best Format"));
            val = settings->getValue<int>("NDI/Input/Format");
            m->value(val);

            mW = new Widget< PopupMenu >(
                g->x() + 10, Y, g->w() - 20, 20, _("With Audio"));
            m = r.inputAudioMenu = mW;
            m->disable_submenus();
            m->labelsize(12);
            m->align(FL_ALIGN_CENTER | FL_ALIGN_CLIP);
            m->add(_("With Audio"));
            m->add(_("Without Audio"));
            
            val = settings->getValue<int>("NDI/Input/Audio");
            m->value(val);

            r.find.awake = false;

            cg->end();

            std::string key = prefix + "NDI Input";
            value = settings->getValue<std::any>(key);
            open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
            if (!open)
            {
                cg->close();
            }

            cg = new CollapsibleGroup(g->x(), Y, g->w(), 20, _("Output"));
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

                    const std::string& prefix = ndiPanel->tab_prefix();
                    const std::string key = prefix + "NDI Output";

                    auto settings = App::app->settings();
                    settings->setValue(key, static_cast<int>(cg->is_open()));
                },
                cg);

            cg->begin();

            auto bW = new Widget<ToggleButton>(
                g->x() + 10, Y, g->w() - 20, 30, _("Start streaming"));
            bW->align(FL_ALIGN_CENTER);
            bW->labelsize(12);
            bW->selection_color(FL_YELLOW);
            bW->callback(
                [=](auto b)
                    {
                        _ndi_output(b);
                    });

            mW = new Widget< PopupMenu >(
                g->x() + 10, Y, g->w() - 20, 20, _("Fast Format"));
            m = r.outputFormatMenu = mW;
            m->disable_submenus();
            m->labelsize(12);
            m->align(FL_ALIGN_CENTER | FL_ALIGN_CLIP);
            m->add(_("Fast Format"));
            m->add(_("Best Format"));
            for (const auto& i : tl::device::getPixelTypeLabels())
            {
                // 10 and 12 bit formats are not supported by NDI
                if (i == "None" || i.substr(0, 2) == "12" ||
                    i.substr(0, 2) == "10")
                    continue;
                m->add(i.c_str());
            }
            val = settings->getValue<int>("NDI/Output/Format");
            m->value(val);
            mW->callback([=](auto b)
                {
                    int value = b->value();
                    settings->setValue("NDI/Output/Format", value);
                    
                    const Fl_Menu_Item* item = b->mvalue();
                    if (!item || !item->label())
                        return;
                
                    std::string format = item->label();
                    int idx = -1;
                    for (const auto& label :
                             tl::device::getPixelTypeLabels())
                    {
                        ++idx;
                        if (label == format)
                            break;
                    }
                
                    auto outputDevice = App::app->outputDevice();
                    if (!outputDevice)
                        return;

                    auto config = outputDevice->getConfig();
                    config.pixelType = static_cast<device::PixelType>(idx);
                    outputDevice->setConfig(config);
                });

            mW = new Widget< PopupMenu >(
                g->x() + 10, Y, g->w() - 20, 20, _("With Audio"));
            m = r.outputAudioMenu = mW;
            m->disable_submenus();
            m->labelsize(12);
            m->align(FL_ALIGN_CENTER | FL_ALIGN_CLIP);
            m->add(_("With Audio"));
            m->add(_("Without Audio"));
            val = settings->getValue<int>("NDI/Output/Audio");
            m->value(val);
            mW->callback([=](auto b)
                {
                    int value = b->value();
                    settings->setValue("NDI/Output/Audio", value);
                    
                    auto outputDevice = App::app->outputDevice();
                    if (!outputDevice)
                        return;

                    auto config = outputDevice->getConfig();
                    config.noAudio = value;
                    outputDevice->setConfig(config);
                });

            mW = new Widget< PopupMenu >(
                g->x() + 10, Y, g->w() - 20, 20, _("With Metadata"));
            m = r.outputMetadataMenu = mW;
            m->disable_submenus();
            m->labelsize(12);
            m->align(FL_ALIGN_CENTER | FL_ALIGN_CLIP);
            m->add(_("With Metadata"));
            m->add(_("Without Metadata"));
            val = settings->getValue<int>("NDI/Output/Metadata");
            m->value(val);
            mW->callback([=](auto b)
                {
                    int value = b->value();
                    settings->setValue("NDI/Output/Metatada", value);
                    
                    auto outputDevice = App::app->outputDevice();
                    if (!outputDevice)
                        return;

                    auto config = outputDevice->getConfig();
                    config.noMetadata = value;
                    outputDevice->setConfig(config);
                });
            
            cg->end();

            key = prefix + "NDI Output";
            value = settings->getValue<std::any>(key);
            open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
            if (!open)
            {
                cg->close();
            }
        }

        void NDIPanel::_ndi_output(ToggleButton* b)
        {
            MRV2_R();
            
            SettingsObject* settings = App::app->settings();
            
            if (!App::ui->uiView->getTimelinePlayer())
            {
                b->value(0);
                b->redraw();
                return;
            }
                    
            if (!b->value())
            {
                
                App::app->beginNDIOutputStream();

                
                const Fl_Menu_Item* item = r.outputFormatMenu->mvalue();
                if (!item || !item->label())
                    return;
                
                int val = r.outputFormatMenu->value();
                settings->setValue("NDI/Output/Format", val);

                std::string format = item->label();
                int idx = -1;
                for (const auto& label :
                         tl::device::getPixelTypeLabels())
                {
                    ++idx;
                    if (label == format)
                        break;
                }
                
                const Fl_Menu_Item* audioItem = r.outputAudioMenu->mvalue();
                if (!audioItem || !audioItem->label())
                    return;
                
                int noAudio = r.outputAudioMenu->value();
                settings->setValue("NDI/Output/Audio", noAudio);
                
                int noMetadata = r.outputMetadataMenu->value();
                
                device::DeviceConfig config;
                config.deviceIndex = 0;
                config.displayModeIndex = 0;
                config.pixelType = static_cast<device::PixelType>(idx);
                config.noAudio = noAudio;
                config.noMetadata = noMetadata;
                
                if (format == _("Best Format"))
                {
#ifdef NDI_SDK_ADVANCED
                    config.pixelType = device::PixelType::_16BitPA16;
#else
                    config.pixelType = device::PixelType::_8BitRGBA;
#endif
                }
                else if (format == _("Fast Format"))
                {
                    config.pixelType = device::PixelType::_8BitYUV;
                }
                
                const std::string msg =
                    string::Format(_("Streaming {0} {1}...")).
                    arg(config.pixelType).
                    arg(audioItem->label());
                LOG_STATUS(msg);
                            
                auto outputDevice = App::app->outputDevice();
                if (!outputDevice)
                    return;
                outputDevice->setConfig(config);
                b->copy_label(_("Stop streaming"));
                b->value(1);
                b->redraw();
            }
            else
            {
                App::app->endNDIOutputStream();
                b->copy_label(_("Start streaming"));
                b->value(0);
                b->redraw();
            }
        }
        
        void NDIPanel::_ndi_input(const Fl_Menu_Item* item)
        {
            TLRENDER_P();
            MRV2_R();

            // Get the NDI name from the menu item
            const std::string sourceName = item->label();

            if (r.lastStream == sourceName)
                return;

            if (!r.lastStream.empty())
                LOG_STATUS("Close stream " << r.lastStream);

            auto model = p.ui->app->filesModel();
            if (model)
            {
                auto aItem = model->observeA()->get();
                if (aItem && file::isTemporaryNDI(aItem->path))
                    model->close();
            }

            r.lastStream = sourceName;

            if (sourceName == _("None"))
                return;

            // Create an ndi file
            std::string ndiFile = file::NDI(p.ui);

            ndi::Options options;
            options.sourceName = sourceName;
            options.bestFormat = r.inputFormatMenu->value();
            options.noAudio = r.inputAudioMenu->value();

            nlohmann::json j;
            j = options;

            std::ofstream s(ndiFile);
            s << j << std::endl;
            s.close();

            LOG_STATUS("Opened stream " << sourceName);

            open_file_cb(ndiFile, p.ui);

            auto player = p.ui->uiView->getTimelinePlayer();
            if (!player)
                return;

            int seconds = 4;
            r.play.found = !options.noAudio;
            player->stop();

            if (!options.noAudio)
            {
                r.cacheInfoObserver =
                    observer::ValueObserver<timeline::PlayerCacheInfo>::create(
                        player->player()->observeCacheInfo(),
                        [this, player,
                         options](const timeline::PlayerCacheInfo& value)
                        {
                            MRV2_R();
                            auto startTime = player->currentTime();
                            auto endTime =
                                startTime + options.audioBufferSize.rescaled_to(
                                                startTime.rate());

                            for (const auto& t : value.audioFrames)
                            {
                                if (t.start_time() <= startTime &&
                                    t.end_time_exclusive() >= endTime)
                                {
                                    r.play.found = true;
                                    r.cacheInfoObserver.reset();
                                    break;
                                }
                            }
                        },
                        observer::CallbackAction::Suppress);
            }

            r.play.thread = std::thread(
                [this, player, seconds]
                {
                    MRV2_R();
                    auto start = std::chrono::steady_clock::now();
                    while (!r.play.found &&
                           std::chrono::steady_clock::now() - start <=
                               std::chrono::seconds(seconds))
                    {
                    }
                    player->forward();
                });

            r.play.thread.detach();
        }

    } // namespace panel

} // namespace mrv

#endif // TLRENDER_NDI
