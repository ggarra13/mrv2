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

#    include <Processing.NDI.Lib.h>

#    include "mrvCore/mrvHome.h"
#    include "mrvCore/mrvFile.h"

#    include "mrvWidgets/mrvFunctional.h"
#    include "mrvWidgets/mrvButton.h"
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
            Fl_Choice* source = nullptr;

            NDIlib_find_instance_t NDI_find = nullptr;
            
            std::thread thread;
            std::mutex mutex;
            std::atomic<bool> running = false;
        };

        
        NDIPanel::NDIPanel(ViewerUI* ui) :
            _r(new Private),
            PanelWidget(ui)
        {
            MRV2_R();
            
            add_group("NDI");

            // @todo:
            // Fl_SVG_Image* svg = load_svg("NDI.svg");
            // g->image(svg);
            
            if(!NDIlib_initialize())
                throw std::runtime_error(_("Could not initialize NDI"));

            r.NDI_find = NDIlib_find_create_v2();
            if (!r.NDI_find)
                LOG_ERROR("Could not create NDI find");
                        
            // Run for one minute
            r.thread = std::thread(
                    [this]
                    {
                        MRV2_R();

                        r.running = true;
                        while (r.running)
                        {
                        
                            uint32_t no_sources = 0;
                            const NDIlib_source_t* p_sources = NULL;
                    
                            using namespace std::chrono;
                            for (const auto start = high_resolution_clock::now(); high_resolution_clock::now() - start < seconds(10);)
                            {
                                // Wait up till 1 second to check for new sources to be added or removed
                                if (!NDIlib_find_wait_for_sources(r.NDI_find, 1000 /* milliseconds */)) {
                                    break;
                                }
                        
                            }


                            while (!no_sources)
                            {
                                // Get the updated list of sources
                                p_sources = NDIlib_find_get_current_sources(r.NDI_find, &no_sources);
                            }

                            
                            if (!r.source) continue;
                        
                            {
                                std::unique_lock<std::mutex> lock(r.mutex);
                                Fl_Choice* m = r.source;


                                std::string sourceName;
                                int selected = m->value();
                                bool changed = false;
                                if (no_sources != m->size())
                                {
                                    changed = true;
                                }
                                else
                                {
                                    const Fl_Menu_Item* item = nullptr;
                                    if (selected >= 0)
                                    {
                                        item = &m->menu()[selected];
                                        sourceName = item->label();
                                    }
                                    for (int i = 0; i < no_sources; ++i)
                                    {
                                        item = &m->menu()[i];
                                        if (!strcmp(item->label(), p_sources[i].p_ndi_name))
                                        {
                                            changed = true;
                                            break;
                                        }
                                    }
                                }

                                if (changed)
                                {
                                    m->clear();
                                    for (int i = 0; i < no_sources; ++i)
                                    {
                                        const std::string ndiName = p_sources[i].p_ndi_name;
                                        m->add(ndiName.c_str());
                                        if (sourceName == ndiName)
                                            selected = i;
                                    }
                                    m->value(selected);
                                }
                            }
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
            if (r.thread.joinable())
                r.thread.join();
            
            NDIlib_find_destroy(r.NDI_find);
            r.NDI_find = nullptr; 
        }

        void NDIPanel::add_controls()
        {
            TLRENDER_P();
            MRV2_R();

            SettingsObject* settings = App::app->settings();
            const std::string& prefix = tab_prefix();

            Fl_Group* bg;
            Fl_Spinner* sp;
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

            {
                std::unique_lock<std::mutex> lock(r.mutex);
            
                auto mW = new Widget< Fl_Choice >(
                    g->x() + 60, Y, g->w() - 60, 20, _("Source"));
                Fl_Choice* m = _r->source = mW;
                m->labelsize(12);
                m->align(FL_ALIGN_LEFT);
                mW->callback(
                    [=](auto o)
                        {
                            _update();
                            _open_ndi();
                        });
            }
            
            Y += 44;
            
            auto bW = new Widget< Fl_Button >(
                g->x() + 60, Y, g->w() - 60, 20, _("Refresh Sources"));
            b = bW;
            b->labelsize(12);
            b->align(FL_ALIGN_CENTER|FL_ALIGN_INSIDE);
            b->box(FL_UP_BOX);

            bW->callback(
                [=](auto o)
                {
                    refresh();
                });

            Y += 22;


            
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

        // @todo: this does not work.  Options are used only when
        //        opening a file, not on changing on the fly.
        void NDIPanel::_update()
        {
            TLRENDER_P();
            MRV2_R();

            auto player = p.ui->uiView->getTimelinePlayer();
            if (!player)
                return;

            const Fl_Menu_Item* item = r.source->mvalue();
            const std::string sourceName = item->label();
            
            io::Options ioOptions;
            ioOptions["NDI/SourceName"] = string::Format("{0}").arg(sourceName);
            
            player->setIOOptions(ioOptions);
        }
        
        void NDIPanel::_open_ndi()
        {
            TLRENDER_P();
            MRV2_R();
            
            std::string ndiFile = file::NDI(p.ui);
            std::ofstream s(ndiFile);

            const Fl_Menu_Item* item = r.source->mvalue();
            const std::string sourceName = item->label();
            s << sourceName << std::endl;
            s.close();

            std::vector<std::string> ndiFiles;
            ndiFiles.push_back(ndiFile);
            open_files_cb(ndiFiles, p.ui);
        }

    } // namespace panel

} // namespace mrv

#endif // TLRENDER_NDI
