// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#if defined(TLRENDER_NDI)

#include <chrono>
#include <thread>
#include <mutex>
#include <atomic>

#    include <tlCore/StringFormat.h>

#    include <FL/Fl_Choice.H>
#    include <FL/Fl_Check_Button.H>

#    include <Processing.NDI.Lib.h>

#    include "mrvCore/mrvHome.h"
#    include "mrvCore/mrvFile.h"

#    include "mrvWidgets/mrvFunctional.h"
#    include "mrvWidgets/mrvButton.h"
#    include "mrvWidgets/mrvSpinner.h"
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
            Fl_Check_Button* noAudio = nullptr;
            Spinner*         preroll = nullptr;

            NDIlib_find_instance_t NDI_find = nullptr;
            uint32_t no_sources = 0;
            const NDIlib_source_t* p_sources = NULL;

            std::thread playThread;
            
            std::thread findThread;
            std::atomic<bool> running = false;
        };


        void NDIPanel::refresh_sources_cb(void* v)
        {
            NDIPanel* self = (NDIPanel*) v;
            self->refresh_sources();
        }
        
        void NDIPanel::refresh_sources()   
        {
            MRV2_R();
                                
            PopupMenu* m = r.source;
            if (!m) return;

            if (m->popped())
                return;

            std::string sourceName;
            int selected = m->value();
            bool changed = false;
            if (r.no_sources != m->children())
            {
                changed = true;
            }
            else
            {
                const Fl_Menu_Item* item = nullptr;
                if (selected >= 0 && selected < m->size())
                {
                    item = m->child(selected);
                    if (item->label())
                        sourceName = item->label();
                }
                for (int i = 0; i < r.no_sources; ++i)
                {
                    item = m->child(i);
                    if (item->label() &&
                        !strcmp(item->label(), r.p_sources[i].p_ndi_name))
                    {
                        changed = true;
                        break;
                    }
                }
            }

            if (!changed || !r.p_sources)
                return;

            m->clear();
            int idx = 0;
            for (int i = 0; i < r.no_sources; ++i)
            {
                if (r.p_sources[i].p_ndi_name)
                {
                    const std::string ndiName = r.p_sources[i].p_ndi_name;
                    m->add(ndiName.c_str());
                    if (sourceName == ndiName)
                        selected = idx;
                    ++idx;
                }
            }
            m->menu_end();
            if (selected >= 0 && selected < m->size())
                m->value(selected);

        }
                                
        NDIPanel::NDIPanel(ViewerUI* ui) :
            _r(new Private),
            PanelWidget(ui)
        {
            MRV2_R();

            Fl::lock(); // needed
            
            add_group("NDI");

            // @todo:
            // Fl_SVG_Image* svg = load_svg("NDI.svg");
            // g->image(svg);

                            
            r.NDI_find = NDIlib_find_create_v2();
            if (!r.NDI_find)
                LOG_ERROR("Could not create NDI find");
                            
                        
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
                                 high_resolution_clock::now() - start < seconds(10);)
                            {
                                // Wait up till 1 second to check for new sources to be added or removed
                                if (!NDIlib_find_wait_for_sources(r.NDI_find, 1000 /* milliseconds */)) {
                                    break;
                                }
                        
                            }


                            r.no_sources = 0;
                            while (!r.no_sources && r.running)
                            {
                                // Get the updated list of sources
                                r.p_sources = NDIlib_find_get_current_sources(
                                    r.NDI_find, &r.no_sources);
                            }
                            
                            if (!r.source) continue;

                            Fl::awake( (Fl_Awake_Handler) refresh_sources_cb,
                                       this );
            
                            
                        }
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
            
            r.running = false;
            if (r.findThread.joinable())
                r.findThread.join();
            
            if (r.playThread.joinable())
                r.playThread.join();

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

            SettingsObject* settings = App::app->settings();
            const std::string& prefix = tab_prefix();

            Fl_Group* bg;
            Spinner* sp;
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

            Y += 30;
            bg = new Fl_Group(g->x(), Y, g->w(), 22 * 8);
            bg->box(FL_NO_BOX);
            bg->begin();
            
            auto mW = new Widget< PopupMenu >(
                g->x() + 60, Y, g->w() - 60, 20, _("Source"));
            PopupMenu* m = _r->source = mW;
            m->labelsize(12);
            m->align(FL_ALIGN_CENTER);
            mW->callback(
                [=](auto o)
                    {
                        const Fl_Menu_Item* item = o->mvalue();
                        if (!item) return;
                        _open_ndi(item);
                        });
            
                
            Y += 22;

            auto spW = new Widget< Spinner >(
                g->x() + 60, Y, g->w() - 60, 20, _("Preroll"));
            sp = _r->preroll = spW;
            sp->step(1);
            sp->range(1, 10);
            sp->tooltip(_("Preroll in seconds"));
            sp->value(3);
            
            Y += 22;

            
            auto cW = new Widget< Fl_Check_Button >(
                g->x() + 60, Y, g->w() - 60, 20, _("No Audio"));
            Fl_Check_Button* c = _r->noAudio = cW;
            c->labelsize(12);
            c->align(FL_ALIGN_LEFT);
            c->value(0);
            
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
            LOG_INFO("Opened stream " << sourceName);

            // Create an ndi file 
            std::string ndiFile = file::NDI(p.ui);
            
            std::ofstream s(ndiFile);
            s << sourceName << std::endl;
            s << (int)r.noAudio->value() << std::endl;
            s.close();

            open_file_cb(ndiFile, p.ui);

            auto player = p.ui->uiView->getTimelinePlayer();
            if (player)
            {
                LOG_INFO("Waiting for player cache to fill up...");
                player->stop();
                r.playThread = std::thread(
                    [this, player]
                        {
                            MRV2_R();
                            
                            // Sleep so the cache fills up
                            int seconds = r.preroll->value();
                            std::this_thread::sleep_for(
                                std::chrono::seconds(seconds));
                            player->start();
                            player->forward();
                        });
            }
        }

    } // namespace panel

} // namespace mrv

#endif // TLRENDER_NDI
