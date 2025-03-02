// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#if defined(TLRENDER_NDI)

#    include <chrono>
#    include <regex>
#    include <thread>
#    include <mutex>
#    include <atomic>

#    include <tlCore/ListObserver.h>
#    include <tlCore/StringFormat.h>

#    include <tlDevice/OutputData.h>
#    include <tlDevice/NDI/NDIOptions.h>
#    include <tlDevice/NDI/NDIOutputDevice.h>
#    include <tlDevice/NDI/NDISystem.h>
#    include <tlDevice/NDI/NDIUtil.h>

#    include <FL/Fl_Choice.H>

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
    const char* kModule = "ndi ";
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
            
            std::shared_ptr<observer::ListObserver<device::DeviceInfo> > deviceObserver;

            static std::string lastStream;

            std::shared_ptr<observer::ValueObserver<timeline::PlayerCacheInfo> >
                cacheInfoObserver;

            struct PlayThread
            {
                std::atomic<bool> running = false;
                std::thread thread;
            };
            PlayThread play;
        };

        std::string NDIPanel::Private::lastStream;

        NDIPanel::NDIPanel(ViewerUI* ui) :
            _r(new Private),
            PanelWidget(ui)
        {
            MRV2_R();

            add_group("NDI");

            Fl_SVG_Image* svg = load_svg("NDI.svg");
            g->bind_image(svg);
                        
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

            r.play.running = false;
            std::cerr << "~NDIPanel" << std::endl;
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

            
            auto context = App::app->getContext();
            auto NDISystem = context->getSystem<ndi::System>();
            
            r.deviceObserver =
                observer::ListObserver<device::DeviceInfo>::create(
                        NDISystem->observeDeviceInfo(),
                        [this](const std::vector<device::DeviceInfo>& devices)
                            {
                                MRV2_R();

                                r.sourceMenu->clear();
                                r.sourceMenu->add(_("None"));
                                
                                for (auto& device : devices)
                                {
                                    r.sourceMenu->add(device.name.c_str());
                                }
                                r.sourceMenu->menu_end();
                                r.sourceMenu->redraw();
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
            mW->callback([=](auto b)
                {
                    int value = b->value();
                    settings->setValue("NDI/Input/Format", value);
                });

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
            mW->callback([=](auto b)
                {
                    int value = b->value();
                    settings->setValue("NDI/Input/Audio", value);
                });

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
                    int fltk_value = b->value();
                    settings->setValue("NDI/Output/Format", fltk_value);
                    
                    device::PixelType pixelType = _ndi_fourCC(fltk_value);
                
                    auto outputDevice = App::app->outputDevice();
                    if (!outputDevice)
                        return;

                    auto config = outputDevice->getConfig();
                    config.pixelType = pixelType;
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
                
                int fltk_value = r.outputFormatMenu->value();
                settings->setValue("NDI/Output/Format", fltk_value);

                device::PixelType pixelType = _ndi_fourCC(fltk_value);
                    
                const Fl_Menu_Item* audioItem = r.outputAudioMenu->mvalue();
                if (!audioItem || !audioItem->label())
                    return;
                
                int noAudio = r.outputAudioMenu->value();
                settings->setValue("NDI/Output/Audio", noAudio);
                
                int noMetadata = r.outputMetadataMenu->value();
                
                device::DeviceConfig config;
                config.deviceIndex = 0;
                config.displayModeIndex = 0;
                config.pixelType = pixelType;
                config.noAudio = noAudio;
                config.noMetadata = noMetadata;
                
                
                std::string format = item->label();
                
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
            r.play.running = !options.noAudio;
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
                                    r.play.running = true;
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
                    while (!r.play.running &&
                           std::chrono::steady_clock::now() - start <=
                               std::chrono::seconds(seconds))
                    {
                    }
                    player->forward();
                });

            // This thread will onlyh run for 4 seconds.
            r.play.thread.detach();
        }
        
        device::PixelType  NDIPanel::_ndi_fourCC(int fltk_value)
        {
            MRV2_R();
            
            const Fl_Menu_Item* item = r.outputFormatMenu->mvalue();
            const std::string format = item->label();

            
            if (format == _("Best Format"))
            {
#ifdef NDI_SDK_ADVANCED
                return device::PixelType::_16BitPA16;
#else
                return device::PixelType::_8BitRGBA;
#endif
            }
            else if (format == _("Fast Format"))
            {
                return device::PixelType::_8BitUYVA;
            }

            int idx = -1;
                
            for (const auto& label :
                     tl::device::getPixelTypeLabels())
            {
                ++idx;
                if (label == format)
                    break;
            }
            
            return static_cast<device::PixelType>(idx);
        }
        
    } // namespace panel

} // namespace mrv

#endif // TLRENDER_NDI
