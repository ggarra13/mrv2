// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "mrViewer.h"

#include "mrvWidgets/mrvAnnotationGroup.h"
#include "mrvWidgets/mrvAnnotationWidget.h"

#include "mrvIcons/Annotations.h"

#include "mrvPanels/mrvNotesPanel.h"
#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvApp/mrvSettingsObject.h"

namespace
{
    const char* kModule = "fonts";
}

namespace mrv
{
    namespace panel
    {

        NotesPanel::NotesPanel(ViewerUI* ui) :
            PanelWidget(ui)
        {
            add_group("Notes");

            Fl_SVG_Image* svg = MRV2_LOAD_SVG(Annotations);
            g->bind_image(svg);

            g->callback(
                [](Fl_Widget* w, void* d)
                {
                    ViewerUI* ui = static_cast< ViewerUI* >(d);
                    delete notesPanel;
                    notesPanel = nullptr;
                    ui->uiMain->fill_menu(ui->uiMenuBar);
                },
                ui);
        }

        NotesPanel::~NotesPanel() {}

        void NotesPanel::add_controls()
        {
            TLRENDER_P();

            SettingsObject* settings = App::app->settings();
            std::string prefix = tab_prefix();

            int X = g->x();
            int Y = 20;

            auto view = p.ui->uiView;

            TimelinePlayer* player;
            player = view->getTimelinePlayer();

            g->clear();
            g->begin();

            Fl_Button* b;
            std::string key;
            std::any value;
            bool open;

            auto ag = new AnnotationGroup(X, Y, g->w(), 20, _("Notes"));
            b = ag->button();
            b->callback(
                [](Fl_Widget* w, void* d)
                {
                    AnnotationGroup* ag = static_cast<AnnotationGroup*>(d);
                    ag->toggle_collapsed();

                    const std::string& prefix = notesPanel->tab_prefix();
                    const std::string key = prefix + "Notes";

                    App* app = App::ui->app;
                    auto settings = app->settings();
                    settings->setValue(key,
                                       static_cast<int>(!ag->is_collapsed()));

                    notesPanel->refresh();
                },
                ag);

            ag->setPlayer(nullptr);

            if (!player)
                return;

            ag->begin();

            auto time = player->currentTime();
            auto annotations = player->getAllAnnotations();
            for (auto annotation : annotations)
            {
                for (auto shape : annotation->shapes)
                {
                    if (auto s = std::dynamic_pointer_cast<tl::draw::NoteShape>(shape))
                    {
                        auto w = ag->add_annotation(annotation->time, s);
                        if (time != annotation->time)
                            w->set_collapsed(true);
                        else
                            w->set_collapsed(false);
                    }
                }
            }

            ag->end();

            ag->setPlayer(player);

            key = prefix + "Notes";
            value = settings->getValue<std::any>(key);
            open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
            if (!open)
                ag->set_collapsed(true);

        }


    } // namespace panel

} // namespace mrv
