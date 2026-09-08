// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl_Choice.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Radio_Round_Button.H>
#include <FL/Fl_Round_Button.H>

#include "mrViewer.h"


#include "mrvCore/mrvFonts.h"

#include "mrvIcons/Annotations.h"
#include "mrvIcons/HardBrush.h"
#include "mrvIcons/SoftBrush.h"

#include "mrvWidgets/mrvAnnotationGroup.h"
#include "mrvWidgets/mrvAnnotationWidget.h"
#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvHorSlider.h"
#include "mrvWidgets/mrvButton.h"
#include "mrvWidgets/mrvCollapsibleGroup.h"
#include "mrvWidgets/mrvDoubleSpinner.h"
#include "mrvWidgets/mrvMultilineInput.h"

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

        struct NotesPanel::Private
        {
        };

        NotesPanel::NotesPanel(ViewerUI* ui) :
            _r(new Private),
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
            if (!view) return;

            auto player = view->getTimelinePlayer();
            if (!player)
                return;

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
                                       static_cast<int>(ag->is_collapsed()));

                    notesPanel->refresh();
                },
                ag);

            ag->setPlayer(player);

            ag->begin();


            auto annotations = player->getAllAnnotations();
            for (auto annotation : annotations)
            {
                for (auto shape : annotation->shapes)
                {
                    if (auto s = std::dynamic_pointer_cast<tl::draw::NoteShape>(shape))
                    {
                        ag->add_annotation(annotation->time, s);
                    }
                }
            }

            ag->end();

            key = prefix + "Notes";
            value = settings->getValue<std::any>(key);
            open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
            if (!open)
                ag->close();

        }


    } // namespace panel

} // namespace mrv
