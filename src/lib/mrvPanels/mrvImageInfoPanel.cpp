// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrViewer.h"
#include "mrvPreferencesUI.h"

#include "mrvApp/mrvSettingsObject.h"
#include "mrvApp/mrvFilesModel.h"


#include "mrvFl/mrvIO.h"
#include "mrvFl/mrvHotkey.h"
#include "mrvFl/mrvPreferences.h"

#include "mrvPanels/mrvPanelsCallbacks.h"
#include "mrvPanels/mrvImageInfoPanel.h"

#include "mrvWidgets/mrvHorSlider.h"
#include "mrvWidgets/mrvInput.h"
#include "mrvWidgets/mrvIntInput.h"
#include "mrvWidgets/mrvHorSlider.h"
#include "mrvWidgets/mrvPack.h"
#include "mrvWidgets/mrvCollapsibleGroup.h"
#include "mrvWidgets/mrvTable.h"

#include "mrvCore/mrvColorSpaces.h"
#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvUtil.h"
#include "mrvCore/mrvSequence.h"
#include "mrvCore/mrvString.h"
#include "mrvCore/mrvMath.h"


#include <cinttypes>
#include <locale>

#include <tlCore/HDR.h>

#include <FL/Fl_Pack.H>
#include <FL/Fl_Flex.H>
#include <FL/Fl_Int_Input.H>
#include <FL/fl_draw.H>

#ifdef TLRENDER_EXR
#include <OpenEXR/ImfTileDescription.h>
#endif

#include <algorithm>
#include <ctime>
#include <iostream>
#include <filesystem>
#include <regex>
namespace fs = std::filesystem;


namespace
{
    const char* kModule = "info";
}

void attach_ocio_ics_cb(Fl_Widget* o, ViewerUI* v) {}

namespace tl
{

    //! Audio and video I/O.
    namespace io
    {
        typedef std::map< std::string, std::string > Attribute;
    }
} // namespace tl

namespace mrv
{
    namespace panel
    {

        static const int kLineHeight = 20;
        static const int kLabelSize = 12;

        static const Fl_Color kTitleColors[] = {
            0x608080ff, 0x808060ff, 0x606080ff, 0x608060ff, 0x806080ff,
        };

        static const unsigned int kSizeOfTitleColors =
            (sizeof(kTitleColors) / sizeof(Fl_Color));

        static const Fl_Color kRowColors[] = {
            0x80808000,
            0xa0a0a000,
        };

        static const unsigned int kSizeOfRowColors =
            (sizeof(kRowColors) / sizeof(Fl_Color));

        enum MatchType { kMatchAll, kMatchAttribute, kMatchValue };

        int idx = -1;
        std::string old_match;
        MatchType old_type = kMatchAll;
        int num_matches = 0;
        int match_goal = 1;

        static bool
        regex_match(int row, const std::string& regex, const std::string& text)
        {
            try
            {
                std::regex expr{regex};
                if (std::regex_search(text, expr))
                {
                    ++num_matches;
                    if (match_goal == num_matches)
                    {
                        idx = row;
                        return true;
                    }
                }
            }
            catch (const std::regex_error& e)
            {
            }
            return false;
        }

        static bool process_row(
            int row, Fl_Widget* w, const std::string& match, MatchType type)
        {
            if ((type == kMatchValue || type == kMatchAll) &&
                dynamic_cast< HorSlider* >(w) != nullptr)
            {
                HorSlider* input = (HorSlider*)w;
                if (regex_match(row, match, input->uiValue->value()))
                    return true;
            }
            else if (
                (type == kMatchValue || type == kMatchAll) &&
                dynamic_cast< Input* >(w) != nullptr)
            {
                Input* input = (Input*)w;
                if (regex_match(row, match, input->value()))
                    return true;
            }
            else if (dynamic_cast< Fl_Group* >(w) != nullptr)
            {
                Fl_Group* g = (Fl_Group*)w;
                for (int c = 0; c < g->children(); ++c)
                {
                    w = (Fl_Widget*)g->child(c);
                    bool ok = process_row(row, w, match, type);
                    if (ok)
                        return ok;
                }
            }
            else
            {
                if (type != kMatchAttribute && type != kMatchAll)
                    return false;
                if (!w->label())
                    return false;
                if (regex_match(row, match, w->label()))
                    return true;
            }
            return false;
        }

        static int
        search_table(mrv::Table* t, const std::string& match, MatchType type)
        {
            int rows = t->children() -
                       2; // we skip the last two as they are scrollbars
            idx = -1;
            for (int i = 0; i < rows; i += 2) // +2 for each column
            {
                Fl_Widget* w = t->child(i);
                if (!w->visible_r())
                    continue;
                int row = i / 2;
                if (process_row(row, w, match, type))
                    break;
                w = t->child(i + 1);
                if (process_row(row, w, match, type))
                    break;
            }
            return idx;
        }

        static void search_cb(Fl_Widget* o, panel::ImageInfoPanel* info)
        {
            const char* s = info->m_entry->value();

            std::string match = s;
            if (match.empty())
            {
                old_match.clear();
                idx = -1;
                info->scroll_to(0, 0);
                return;
            }
            MatchType type = (MatchType)info->m_type->value();
            num_matches = 0;

            if (match == old_match && type == old_type)
            {
                ++match_goal;
            }
            else
            {
                match_goal = 1;
            }

            old_match = match;
            old_type = type;
            idx = -1;

            int H = kLineHeight + 6;

            Pack* pack = info->get_pack();
            Pack* p = info->m_image->contents();
            Fl_Button* b = info->m_image->button();
            if (!p->children())
                return; // No video/image Loaded

            Table* t = (Table*)p->child(0);

            int start = info->m_image->y() - pack->y();
            start += b->h();
            start += info->flex->h();

            int idx = search_table(t, match, type);
            if (idx >= 0)
            {
                info->scroll_to(0, start + H * idx);
                return;
            }

            p = info->m_video->contents();
            b = info->m_video->button();

            start = info->m_video->y() - pack->y();
            start += b->h();
            start += info->flex->h();

            for (int i = 0; i < p->children(); ++i)
            {
                t = dynamic_cast< Table* >(p->child(i));
                if (!t)
                {
                    start += p->child(i)->h();
                    continue;
                }
                idx = search_table(t, match, type);
                if (idx >= 0)
                {
                    info->scroll_to(0, start + H * idx);
                    return;
                }
                start += t->h();
            }

            p = info->m_audio->contents();
            b = info->m_audio->button();

            start = info->m_audio->y() - pack->y();
            start += b->h();
            start += info->flex->h();

            for (int i = 0; i < p->children(); ++i)
            {
                t = dynamic_cast< Table* >(p->child(i));
                if (!t)
                {
                    start += p->child(i)->h();
                    continue;
                }

                idx = search_table(t, match, type);
                if (idx >= 0)
                {
                    info->scroll_to(0, start + H * idx);
                    return;
                }
                start += t->h();
            }

            p = info->m_subtitle->contents();
            b = info->m_subtitle->button();

            start = info->m_subtitle->y() - pack->y();
            start += b->h();
            start += info->flex->h();

            for (int i = 0; i < p->children(); ++i)
            {
                t = dynamic_cast< Table* >(p->child(i));
                if (!t)
                {
                    start += p->child(i)->h();
                    continue;
                }

                idx = search_table(t, match, type);
                if (idx >= 0)
                {
                    info->scroll_to(0, start + H * idx);
                    return;
                }
                start += t->h();
            }

            p = info->m_attributes->contents();
            b = info->m_attributes->button();

            start = info->m_attributes->y() - pack->y();
            start += b->h();
            start += info->flex->h();

            t = (Table*)p->child(0);

            idx = search_table(t, match, type);
            if (idx >= 0)
            {
                info->scroll_to(0, start + H * idx);
                return;
            }

            match_goal = 0;
        }

        struct ImageInfoPanel::Private
        {
            std::shared_ptr<
                observer::ListObserver<std::shared_ptr<FilesModelItem> > >
                activeObserver;
        };

        ImageInfoPanel::ImageInfoPanel(ViewerUI* ui) :
            _r(new Private),
            PanelWidget(ui)
        {
            add_group("Media Information");

            Fl_SVG_Image* svg = load_svg("Info.svg");
            g->bind_image(svg);

            g->callback(
                [](Fl_Widget* w, void* d)
                {
                    ViewerUI* ui = static_cast< ViewerUI* >(d);
                    delete imageInfoPanel;
                    imageInfoPanel = nullptr;
                    ui->uiMain->fill_menu(ui->uiMenuBar);
                },
                ui);

            _r->activeObserver = observer::
                ListObserver<std::shared_ptr<FilesModelItem> >::create(
                    ui->app->filesModel()->observeActive(),
                    [this](const std::vector< std::shared_ptr<FilesModelItem> >&
                               value)
                    {
                        const auto player = _p->ui->uiView->getTimelinePlayer();
                        setTimelinePlayer(player);
                    });
        }

        void ImageInfoPanel::scroll_to(int X, int Y)
        {
            Fl_Scroll* scroll = g->get_scroll();
            scroll->scroll_to(X, Y);
        }

        void ImageInfoPanel::add_controls()
        {
            TLRENDER_P();

            // Add "Search" "Input" and "Both/Attribute/Value" controls
            Fl_Group* controls = g->get_group();
            controls->size(g->w(), 30); // needed

            controls->begin();

            assert(controls->h() >= 0);

            flex = new Fl_Flex(
                controls->x(), controls->y(), controls->w(), controls->h());
            flex->type(Fl_Flex::HORIZONTAL);
            flex->begin();

            int Y = controls->y();

            Fl_Box* box = new Fl_Box(controls->x(), Y, 80, 30, _("Search"));
            flex->fixed(box, 80);
            m_entry =
                new Input(controls->x() + box->w(), Y, controls->w() - 200, 30);
            m_entry->when(
                FL_WHEN_CHANGED | FL_WHEN_NOT_CHANGED | FL_WHEN_ENTER_KEY);
            m_entry->callback((Fl_Callback*)search_cb, this);

            m_type = new Fl_Choice(m_entry->x() + m_entry->w(), Y, 120, 30);
            flex->fixed(m_type, 100);
            m_type->add(_("Both"));
            m_type->add(_("Attribute"));
            m_type->add(_("Value"));
            m_type->value(0);

            flex->resizable(m_entry);
            flex->end();

            controls->resizable(flex);
            controls->end();

            controls->show();
            assert(controls->h() >= 0);

            Y = controls->y() + controls->h();

            // Reposition scroll and pack below control group.  Needed.
            Fl_Scroll* scroll = g->get_scroll();
            scroll->position(scroll->x(), Y);

            Pack* pack = g->get_pack();
            pack->position(pack->x(), Y);

            // @todo:
            // menu = new Fl_Menu_Button( 0, 0, 0, 0, _("Attributes Menu") );
            // menu->type( Fl_Menu_Button::POPUP3 );
            g->begin();

            int W = g->w();

            SettingsObject* settings = App::app->settings();

            // CollapsibleGrop recalcs, we don't care its xyh sizes
            m_image = new CollapsibleGroup(g->x(), Y, W, 0, _("Main"));
            m_image->end();
            Fl_Button* b = m_image->button();
            b->callback(
                [](Fl_Widget* w, void* d)
                {
                    CollapsibleGroup* cg = static_cast<CollapsibleGroup*>(d);
                    if (cg->is_open())
                        cg->close();
                    else
                        cg->open();
                    imageInfoPanel->refresh();
                },
                m_image);

            std::string prefix = tab_prefix();
            std::string key = prefix + "Main";
            int open = settings->getValue<int>(key);
            if (!open)
                m_image->close();

            Y += m_image->h();
            m_video = new CollapsibleGroup(g->x(), Y, W, 0, _("Video"));
            m_video->end();
            b = m_video->button();
            b->callback(
                [](Fl_Widget* w, void* d)
                {
                    CollapsibleGroup* cg = static_cast<CollapsibleGroup*>(d);
                    if (cg->is_open())
                        cg->close();
                    else
                        cg->open();
                    imageInfoPanel->refresh();
                },
                m_video);

            key = prefix + "Video";
            open = settings->getValue<int>(key);
            if (!open)
                m_video->close();

            Y += m_video->h();
            m_audio = new CollapsibleGroup(g->x(), Y, W, 0, _("Audio"));
            m_audio->end();
            b = m_audio->button();
            b->callback(
                [](Fl_Widget* w, void* d)
                {
                    CollapsibleGroup* cg = static_cast<CollapsibleGroup*>(d);
                    if (cg->is_open())
                        cg->close();
                    else
                        cg->open();
                    imageInfoPanel->refresh();
                },
                m_audio);

            key = prefix + "Audio";
            open = settings->getValue<int>(key);
            if (!open)
                m_audio->close();

            Y += m_audio->h();

            m_subtitle = new CollapsibleGroup(g->x(), Y, W, 0, _("Subtitle"));
            m_subtitle->end();
            b = m_subtitle->button();
            b->callback(
                [](Fl_Widget* w, void* d)
                {
                    CollapsibleGroup* cg = static_cast<CollapsibleGroup*>(d);
                    if (cg->is_open())
                        cg->close();
                    else
                        cg->open();
                    imageInfoPanel->refresh();
                },
                m_subtitle);

            key = prefix + "Subtitle";
            open = settings->getValue<int>(key);
            if (!open)
                m_subtitle->close();

            Y += m_subtitle->h();
            m_attributes = new CollapsibleGroup(g->x(), Y, W, 0, _("Metadata"));
            m_attributes->end();
            b = m_attributes->button();
            b->callback(
                [](Fl_Widget* w, void* d)
                {
                    CollapsibleGroup* cg = static_cast<CollapsibleGroup*>(d);
                    if (cg->is_open())
                        cg->close();
                    else
                        cg->open();
                    imageInfoPanel->refresh();
                },
                m_attributes);

            key = prefix + "Attributes";
            open = settings->getValue<int>(key);
            if (!open)
                m_attributes->close();

            scroll->size(pack->w(), pack->h());
            controls->show(); // show controls as group is hidden by default
        }

        struct AspectName
        {
            double ratio;
            const char* name;
        };

        static const AspectName kAspectRatioNames[] = {
            {640.0 / 480.0, _("Video")},
            {680.0 / 550.0, _("PAL Video")},
            {720.0 / 576.0, _("PAL Video")},
            {768.0 / 576.0, _("PAL Video")},
            {720.0 / 486.0, _("NTSC Video")},
            {720.0 / 540.0, _("NTSC Video")},
            {1.5, _("NTSC Video")},
            {1.37, _("35mm Academy")},
            {1.56, _("Widescreen (HDTV + STV)")},
            {1.66, _("35mm European Widescreen")},
            {1.75, _("Early 35mm")},
            {1.777778, _("HDTV / Widescreen 16:9")},
            {1.85, _("35mm Flat")},
            {2.2, _("70mm")},
            {2.35, _("35mm Anamorphic")},
            {2.39, _("35mm Panavision")},
            {2.55, _("Cinemascope")},
            {2.76, _("MGM Camera 65")},
        };

        void ImageInfoPanel::enum_cb(mrv::PopupMenu* m, ImageInfoPanel* v)
        {
            m->label(m->child(m->value())->label());
        }

        // Update int slider from int input
        static void update_int_slider(IntInput* w)
        {
            Fl_Group* g = w->parent();
            Fl_Slider* s = (Fl_Slider*)g->child(1);
            s->value(atoi(w->value()));
        }

        void ImageInfoPanel::int_slider_cb(Fl_Slider* s, void* data)
        {
            IntInput* n = (IntInput*)data;
            char buf[64];
            snprintf(buf, 64, "%g", s->value());
            n->value(buf);
            n->do_callback();
        }

        static bool modify_int(IntInput* w, tl::io::Attribute::iterator& i)
        {
            update_int_slider(w);
            return true;
        }

        static void change_x_and_y_level_cb(HorSlider* w, ImageInfoPanel* info)
        {
            int f = w->value();
            info->setXLevel(f);
            info->setYLevel(f);
            info->m_update = false;
            refresh_media_cb(nullptr, nullptr);
            info->m_update = true;
        }
        
        static void change_xlevel_cb(HorSlider* w, ImageInfoPanel* info)
        {
            int f = w->value();
            info->setXLevel(f);
            info->m_update = false;
            refresh_media_cb(nullptr, nullptr);
            info->m_update = true;
        }
        
        static void change_ylevel_cb(HorSlider* w, ImageInfoPanel* info)
        {
            int f = w->value();
            info->setYLevel(f);
            info->m_update = false;
            refresh_media_cb(nullptr, nullptr);
            info->m_update = true;
        }

        static void change_first_frame_cb(HorSlider* w, ImageInfoPanel* info)
        {
            double f = w->value();
            ViewerUI* ui = App::ui;
            TimelineClass* c = ui->uiTimeWindow;
            c->uiStartFrame->value(f);
            c->uiStartFrame->do_callback();
        }

        static void change_last_frame_cb(HorSlider* w, ImageInfoPanel* info)
        {
            double f = w->value();
            ViewerUI* ui = App::ui;
            TimelineClass* c = ui->uiTimeWindow;
            c->uiEndFrame->value(f);
            c->uiEndFrame->do_callback();
        }

        static void change_fps_cb(HorSlider* w, ImageInfoPanel* info)
        {
            double f = w->value();
            ViewerUI* ui = App::ui;
            TimelineClass* c = ui->uiTimeWindow;
            c->uiFPS->value(f);
            auto player = ui->uiView->getTimelinePlayer();
            player->setSpeed(f);
        }

        static void change_pixel_ratio_cb(HorSlider* w, ImageInfoPanel* info)
        {
            ViewerUI* ui = App::ui;
            auto view = ui->uiView;

            float pixelRatio = w->value();
            view->setPixelAspectRatio(pixelRatio);
        }

        double
        ImageInfoPanel::to_memory(std::uintmax_t value, const char*& extension)
        {
            double out = value;
            if (value >= 1099511627776)
            {
                out /= 1099511627776.F;
                extension = "Tb";
            }
            else if (value >= 1073741824)
            {
                out /= 1073741824.F;
                extension = "Gb";
            }
            else if (value >= 1048576)
            {
                out /= 1048576.F;
                extension = "Mb";
            }
            else if (value >= 1024)
            {
                out /= 1024.F;
                extension = "Kb";
            }
            else
            {
                extension = "bytes";
            }
            return out;
        }

        ImageInfoPanel::~ImageInfoPanel()
        {
            save_tabs();
        }

        TimelinePlayer* ImageInfoPanel::timelinePlayer() const
        {
            return player;
        }

        void ImageInfoPanel::setTimelinePlayer(TimelinePlayer* timelinePlayer)
        {
            if (timelinePlayer == player && timelinePlayer != nullptr)
                return;
            player = timelinePlayer;
            refresh();
        }

        void ImageInfoPanel::hide_tabs()
        {
            m_curr = nullptr;

            m_image->hide();
            m_video->hide();
            m_audio->hide();
            m_subtitle->hide();
            m_attributes->hide();
        }

        void ImageInfoPanel::save_tabs() const
        {
            TLRENDER_P();

            SettingsObject* settings = App::app->settings();
            std::string prefix = tab_prefix();
            std::string key = prefix + "Main";
            int value = m_image->is_open();
            settings->setValue(key, value);

            key = prefix + "Video";
            value = m_video->is_open();
            settings->setValue(key, value);

            key = prefix + "Audio";
            value = m_audio->is_open();
            settings->setValue(key, value);

            key = prefix + "Subtitle";
            value = m_subtitle->is_open();
            settings->setValue(key, value);

            key = prefix + "Attributes";
            value = m_attributes->is_open();
            settings->setValue(key, value);
        }

        void ImageInfoPanel::save()
        {
            TLRENDER_P();
            PanelWidget::save();
            save_tabs();
        }

        void ImageInfoPanel::imageRefresh()
        {
            Fl_Group* orig = Fl_Group::current();
            fill_image_data();
            Fl_Group::current(orig);
        }

        void ImageInfoPanel::videoRefresh()
        {
            Fl_Group* orig = Fl_Group::current();
            fill_video_data();
            Fl_Group::current(orig);
        }

        void ImageInfoPanel::metadataRefresh()
        {
            Fl_Group* orig = Fl_Group::current();
            fill_metadata();
            Fl_Group::current(orig);
        }

        void ImageInfoPanel::refresh()
        {
            if (!m_update)
                return;
            
            Fl_Group* orig = Fl_Group::current();

            hide_tabs();

            m_image->clear();
            m_video->clear();
            m_audio->clear();
            m_subtitle->clear();
            m_attributes->clear();

            getTags();
            fill_data();

            m_attributes->end();
            m_subtitle->end();
            m_audio->end();
            m_video->end();
            m_image->end();

            // Needed to resize the panels
            if (player)
                end_group();

            Fl_Group::current(orig);
        }

        Table*
        ImageInfoPanel::add_browser(CollapsibleGroup* g, const char* label)
        {
            if (!g)
                return nullptr;

            X = 0;
            Y = g->y() + kLineHeight;

            // Box is needed to leave a space for Table's title like
            // "Audio Stream #2".
            Fl_Box* box = new Fl_Box(0, Y, g->w(), 20);
            g->add(box);

            Table* table = new Table(0, Y + box->h(), g->w(), 20);
            table->column_separator(true);
            // table->auto_resize( true );
            //  table->labeltype(FL_NO_LABEL);
            static const char* headers[] = {_("Attribute"), _("Value"), 0};
            table->column_labels(headers);
            table->col_width_all(kMiddle);

            table->align(FL_ALIGN_CENTER | FL_ALIGN_TOP);
            table->end();
            table->copy_label(label);

            g->add(table);

            group = row = 0; // controls line colors

            return table;
        }

        Table* ImageInfoPanel::add_browser(CollapsibleGroup* g)
        {
            if (!g)
                return nullptr;

            X = 0;
            Y = g->y() + kLineHeight;

            Table* table = new Table(0, Y, g->w(), 20);
            table->column_separator(true);
            // table->auto_resize( true );
            //  table->labeltype(FL_NO_LABEL);
            static const char* headers[] = {_("Attribute"), _("Value"), 0};
            table->column_labels(headers);
            table->col_width_all(kMiddle);

            table->align(FL_ALIGN_CENTER | FL_ALIGN_TOP);
            table->end();

            g->add(table);

            group = row = 0; // controls line colors

            return table;
        }

        Fl_Color ImageInfoPanel::get_title_color()
        {
            return kTitleColors[group % kSizeOfTitleColors];
        }

        Fl_Color ImageInfoPanel::get_widget_color()
        {
            Fl_Color col = kRowColors[row % kSizeOfRowColors];
            ++row;
            return col;
        }

        void ImageInfoPanel::add_button(
            const char* name, const char* tooltip, Fl_Callback* callback,
            Fl_Callback* callback2)
        {
            Fl_Color colA = get_title_color();
            Fl_Color colB = get_widget_color();

            int hh = kLineHeight;
            Y += hh;

            Fl_Group* g = new Fl_Group(X, Y, kMiddle, hh);
            {
                Fl_Box* widget = new Fl_Box(X, Y, kMiddle, hh);
                widget->box(FL_FLAT_BOX);
                widget->color(colA);
                widget->labelcolor(FL_BLACK);
                widget->labelsize(kLabelSize);
                widget->copy_label(name);
                g->end();
            }
            m_curr->add(g);

            g = new Fl_Group(kMiddle, Y, g->w() - kMiddle, hh);
            {
                int w2 = g->w() - kMiddle;
                w2 /= 2;
                Fl_Button* widget = new Fl_Button(kMiddle, Y, w2, hh);
                widget->tooltip(tooltip);
                widget->labelsize(kLabelSize);
                widget->copy_label(_("Load"));
                if (callback)
                    widget->callback((Fl_Callback*)callback, (void*)this);
                widget = new Fl_Button(kMiddle + w2, Y, w2, hh);
                widget->tooltip(tooltip);
                widget->copy_label(_("Reset"));
                if (callback)
                    widget->callback((Fl_Callback*)callback2, (void*)this);
                g->end();
            }
            m_curr->add(g);

            m_curr->end();
        }

        void ImageInfoPanel::add_scale(
            const char* name, const char* tooltip, int pressed, int num_scales,
            Fl_Callback* callback)
        {
            Fl_Color colA = get_title_color();
            Fl_Color colB = get_widget_color();

            int hh = kLineHeight;
            Y += hh;

            Fl_Group* g = new Fl_Group(X, Y, kMiddle, hh);
            {
                Fl_Box* widget = new Fl_Box(X, Y, kMiddle, hh);
                widget->box(FL_FLAT_BOX);
                widget->color(colA);
                widget->labelcolor(FL_BLACK);
                widget->labelsize(kLabelSize);
                widget->copy_label(name);
                g->end();
            }
            m_curr->add(g);

            g = new Fl_Group(kMiddle, Y, g->w() - kMiddle, hh);
            {
                int w5 = g->w() - kMiddle;
                w5 /= num_scales;
                Fl_Button* widget = new Fl_Button(kMiddle, Y, w5, hh);
                widget->tooltip(tooltip);
                widget->labelsize(kLabelSize);
                widget->copy_label(_("1:1"));
                if (pressed == 0)
                    widget->value(1);
                else
                    widget->value(0);
                if (callback)
                    widget->callback((Fl_Callback*)callback, (void*)this);
                widget = new Fl_Button(kMiddle + w5, Y, w5, hh);
                widget->tooltip(tooltip);
                widget->labelsize(kLabelSize);
                widget->copy_label(_("1:2"));
                if (pressed == 1)
                    widget->value(1);
                else
                    widget->value(0);
                if (callback)
                    widget->callback((Fl_Callback*)callback, (void*)this);
                widget = new Fl_Button(kMiddle + w5 * 2, Y, w5, hh);
                widget->tooltip(tooltip);
                widget->labelsize(kLabelSize);
                widget->copy_label(_("1:4"));
                if (pressed == 2)
                    widget->value(1);
                else
                    widget->value(0);
                if (callback)
                    widget->callback((Fl_Callback*)callback, (void*)this);
                widget = new Fl_Button(kMiddle + w5 * 3, Y, w5, hh);
                widget->tooltip(tooltip);
                widget->labelsize(kLabelSize);
                widget->copy_label(_("1:8"));
                if (pressed == 3)
                    widget->value(1);
                else
                    widget->value(0);
                if (callback)
                    widget->callback((Fl_Callback*)callback, (void*)this);
                if (num_scales > 4)
                {
                    widget = new Fl_Button(kMiddle + w5 * 4, Y, w5, hh);
                    widget->tooltip(tooltip);
                    widget->labelsize(kLabelSize);
                    widget->copy_label(_("1:16"));
                    if (pressed == 4)
                        widget->value(1);
                    else
                        widget->value(0);
                    if (callback)
                        widget->callback((Fl_Callback*)callback, (void*)this);
                }
                g->end();
            }
            m_curr->add(g);

            m_curr->end();
        }

        void ImageInfoPanel::add_ocio_ics(
            const char* name, const char* tooltip, const char* content,
            const bool editable, Fl_Callback* callback)
        {
            if (!editable)
                return add_text(name, tooltip, content);

            Fl_Color colA = get_title_color();
            Fl_Color colB = get_widget_color();

            Fl_Box* lbl;
            int hh = kLineHeight;
            Y += hh;
            Fl_Group* g = new Fl_Group(X, Y, kMiddle, hh);
            g->end();
            {
                Fl_Box* widget = lbl = new Fl_Box(X, Y, kMiddle, hh);
                widget->box(FL_FLAT_BOX);
                widget->color(colA);
                widget->labelcolor(FL_BLACK);
                widget->labelsize(kLabelSize);
                widget->copy_label(name);
                g->add(widget);
            }
            m_curr->add(g);

            {
                Fl_Group* sg = new Fl_Group(kMiddle, Y, kMiddle, hh);
                sg->end();

                Input* widget = new Input(kMiddle, Y, sg->w() - 50, hh);
                widget->value(content);
                widget->align(FL_ALIGN_LEFT);
                widget->box(FL_FLAT_BOX);
                widget->textcolor(FL_BLACK);
                widget->color(colB);
                widget->tooltip(tooltip ? tooltip : lbl->label());

                sg->add(widget);

                Fl_Button* pick =
                    new Fl_Button(kMiddle + sg->w() - 50, Y, 50, hh, _("Pick"));
                pick->callback((Fl_Callback*)attach_ocio_ics_cb, _p->ui);
                sg->add(pick);

                m_curr->add(sg);
            }
            m_curr->end();
        }

        void ImageInfoPanel::add_text(
            const char* name, const char* tooltip, const char* content,
            const bool editable, const bool active, Fl_Callback* callback)
        {

            Fl_Color colA = get_title_color();
            Fl_Color colB = get_widget_color();

            Fl_Box* lbl;
            int hh = kLineHeight;
            Y += hh;
            Fl_Group* g = new Fl_Group(X, Y, kMiddle, hh);
            {
                Fl_Box* widget = lbl = new Fl_Box(X, Y, kMiddle, hh);
                widget->box(FL_FLAT_BOX);
                widget->color(colA);
                widget->labelcolor(FL_BLACK);
                widget->labelsize(kLabelSize);
                widget->copy_label(name);
                g->end();
            }
            m_curr->add(g);

            {
                Fl_Widget* widget = nullptr;
                if (!editable)
                {
                    Fl_Output* o =
                        new Fl_Output(kMiddle, Y, g->w() - kMiddle, hh);
                    widget = o;
                    o->value(content);
                    o->textsize(12);
                    o->textcolor(FL_BLACK);
                }
                else
                {
                    Input* o = new Input(kMiddle, Y, g->w() - kMiddle, hh);
                    widget = o;
                    o->value(content);
                }
                widget->align(FL_ALIGN_LEFT);
                widget->box(FL_FLAT_BOX);
                widget->color(colB);
                if (tooltip)
                    widget->tooltip(tooltip);
                else
                    widget->tooltip(lbl->label());
                if (!editable)
                {
                    widget->box(FL_FLAT_BOX);
                }
                else
                {
                    if (callback)
                        widget->callback(callback, this);
                }
                if (!active)
                    widget->deactivate();
                m_curr->add(widget);
            }
            m_curr->end();
        }

        void ImageInfoPanel::add_text(
            const char* name, const char* tooltip, const std::string& content,
            const bool editable, const bool active, Fl_Callback* callback)
        {
            add_text(
                name, tooltip, content.c_str(), editable, active, callback);
        }

        void ImageInfoPanel::add_int(
            const char* name, const char* tooltip, const int content,
            const bool editable, const bool active, Fl_Callback* callback,
            const int minV, const int maxV, const int when)
        {
            Fl_Color colA = get_title_color();
            Fl_Color colB = get_widget_color();

            Fl_Box* lbl;
            int hh = kLineHeight;
            Y += hh;
            Fl_Group* g = new Fl_Group(X, Y, kMiddle, hh);
            g->end();
            {
                Fl_Box* widget = lbl = new Fl_Box(X, Y, kMiddle, hh);
                widget->box(FL_FLAT_BOX);
                widget->labelcolor(FL_BLACK);
                widget->labelsize(kLabelSize);
                widget->copy_label(name);
                widget->color(colA);
                g->add(widget);
            }
            m_curr->add(g);

            {
                Fl_Group* p = new Fl_Group(kMiddle, Y, kMiddle, hh);
                p->box(FL_FLAT_BOX);
                p->begin();

                HorSlider* widget = new HorSlider(kMiddle, Y, p->w(), hh);
                if (tooltip)
                    widget->tooltip(tooltip);
                else
                    widget->tooltip(lbl->label());

                widget->step(1.0F);
                double maxS = maxV;
                if (content > 100000 && maxV <= 100000)
                    maxS = 1000000;
                else if (content > 10000 && maxV <= 10000)
                    maxS = 100000;
                else if (content > 1000 && maxV <= 1000)
                    maxS = 10000;
                else if (content > 100 && maxV <= 100)
                    maxS = 1000;
                else if (content > 10 && maxV <= 10)
                    maxS = 100;
                else if (content > maxS)
                    maxS = content + 50;
                widget->range(minV, maxS);

                widget->value(content);
                widget->default_value(content);
                widget->align(FL_ALIGN_LEFT);
                widget->color(colB);
                widget->setEnabled(editable);
                widget->when(when);

                if (callback)
                    widget->callback(callback, this);

                p->end();
                m_curr->add(p);
                if (!active)
                {
                    p->deactivate();
                }
            }
            m_curr->end();
        }

        void ImageInfoPanel::add_enum(
            const char* name, const char* tooltip, const size_t content,
            const char* const* options, const size_t num, const bool editable,
            Fl_Callback* callback)
        {
            Fl_Color colA = get_title_color();
            Fl_Color colB = get_widget_color();

            Fl_Box* lbl;
            int hh = kLineHeight;
            Y += hh;
            Fl_Group* g = new Fl_Group(X, Y, kMiddle, hh);
            g->end();
            {
                Fl_Box* widget = lbl = new Fl_Box(X, Y, kMiddle, hh);
                widget->box(FL_FLAT_BOX);
                widget->labelcolor(FL_BLACK);
                widget->labelsize(kLabelSize);
                widget->copy_label(name);
                widget->color(colA);
                g->add(widget);
            }
            m_curr->add(g);

            {
                mrv::PopupMenu* widget =
                    new mrv::PopupMenu(kMiddle, Y, g->w() - kMiddle, hh);
                widget->type(0);
                widget->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
                widget->color(colB);
                widget->labelcolor(FL_BLACK);
                widget->textsize(12);
                widget->textcolor(FL_BLACK);
                widget->labelsize(kLabelSize);
                widget->copy_label(_(options[content]));
                if (tooltip)
                    widget->tooltip(tooltip);
                else
                    widget->tooltip(lbl->label());
                widget->menu_end();

                if (!editable)
                {
                    // widget->deactivate();
                    widget->box(FL_FLAT_BOX);
                }
                else
                {
                    for (size_t i = 0; i < num; ++i)
                    {
                        widget->add(_(options[i]));
                    }
                    widget->value(unsigned(content));
                    if (callback)
                        widget->callback(callback, this);
                }
                m_curr->add(widget);
            }
            m_curr->end();
        }

        void ImageInfoPanel::add_enum(
            const char* name, const char* tooltip, const std::string& content,
            std::vector<std::string>& options, const bool editable,
            Fl_Callback* callback)
        {
            size_t index;
            std::vector<std::string>::iterator it =
                std::find(options.begin(), options.end(), content);
            if (it != options.end())
            {
                index = std::distance(options.begin(), it);
            }
            else
            {
                index = options.size();
                options.push_back(content);
            }

            size_t num = options.size();
            const char** opts = new const char*[num];
            for (size_t i = 0; i < num; ++i)
                opts[i] = options[i].c_str();

            add_enum(name, tooltip, index, opts, num, editable, callback);

            delete[] opts;
        }

        void ImageInfoPanel::add_unsigned(
            const char* name, const char* tooltip, const unsigned int content,
            const bool editable, const bool active, Fl_Callback* callback,
            const unsigned int minV, const unsigned int maxV,
            const int when)
        {
            Fl_Color colA = get_title_color();
            Fl_Color colB = get_widget_color();

            Fl_Box* lbl;
            int hh = kLineHeight;
            Y += hh;
            Fl_Group* g = new Fl_Group(X, Y, kMiddle, hh);
            g->end();
            {
                Fl_Box* widget = lbl = new Fl_Box(X, Y, kMiddle, hh);
                widget->box(FL_FLAT_BOX);
                widget->labelsize(kLabelSize);
                widget->copy_label(name);
                widget->color(colA);
                widget->labelcolor(FL_BLACK);
                g->add(widget);
            }
            m_curr->add(g);

            {
                char buf[64];
                Fl_Group* p = new Fl_Group(kMiddle, Y, kMiddle, hh);
                p->begin();
                if (!editable)
                {
                    IntInput* widget = new IntInput(kMiddle, Y, p->w(), hh);
                    snprintf(buf, 64, "% 9d", content);
                    widget->value(buf);
                    widget->box(FL_FLAT_BOX);
                    widget->deactivate();
                    if (tooltip)
                        widget->tooltip(tooltip);
                    else
                        widget->tooltip(lbl->label());
                }
                else
                {
                    IntInput* widget = new IntInput(kMiddle, Y, 60, hh);
                    snprintf(buf, 64, "% 9d", content);
                    widget->value(buf);
                    widget->align(FL_ALIGN_CENTER);
                    widget->when(when);
                    if (tooltip)
                        widget->tooltip(tooltip);
                    else
                        widget->tooltip(lbl->label());

                    if (callback)
                        widget->callback(callback, this);

                    mrv::Slider* slider =
                        new mrv::Slider(kMiddle + 60, Y, p->w() - 60, hh);
                    slider->type(mrv::Slider::TICK_ABOVE);
                    // slider->linesize(1);
                    slider->type(FL_HORIZONTAL);
                    slider->minimum(minV);
                    slider->when(when);

                    unsigned maxS = maxV;
                    if (content > 100000 && maxV <= 100000)
                        maxS = 1000000;
                    else if (content > 10000 && maxV <= 10000)
                        maxS = 100000;
                    else if (content > 1000 && maxV <= 1000)
                        maxS = 10000;
                    else if (content > 100 && maxV <= 100)
                        maxS = 1000;
                    else if (content > 10 && maxV <= 10)
                        maxS = 100;
                    else if (content > maxS)
                        maxS = content + 50;

                    slider->maximum(maxS);
                    slider->value(content);
                    slider->step(1.0F);
                    if (tooltip)
                        slider->tooltip(tooltip);
                    else
                        slider->tooltip(lbl->label());
                    slider->slider_size(10);
                    slider->when(FL_WHEN_RELEASE);
                    slider->callback((Fl_Callback*)int_slider_cb, widget);

                    p->resizable(slider);
                }

                p->end();
                m_curr->add(p);
                if (!active)
                {
                    p->deactivate();
                }
            }

            m_curr->end();
        }

        void ImageInfoPanel::add_time(
            const char* name, const char* tooltip,
            const otime::RationalTime& content, const bool editable)
        {
            char buf[128];

            std::string text;

            double seconds = content.to_seconds();
            snprintf(buf, 128, _("%.3f seconds "), seconds);
            text += buf;

            text += content.to_timecode();

            add_text(name, tooltip, text, false);
        }

        void ImageInfoPanel::add_int64(
            const char* name, const char* tooltip, const int64_t content)
        {

            char buf[128];
            snprintf(buf, 128, "% 9" PRId64, content);
            add_text(name, tooltip, buf, false);
        }

        void ImageInfoPanel::add_rect(
            const char* name, const char* tooltip,
            const tl::math::Box2i& content, const bool editable,
            Fl_Callback* callback)
        {

            Fl_Color colA = get_title_color();
            Fl_Color colB = get_widget_color();

            Fl_Box* lbl;
            int hh = kLineHeight;
            Y += hh;
            Fl_Group* g = new Fl_Group(X, Y, kMiddle, hh);
            g->end();
            {
                Fl_Box* widget = lbl = new Fl_Box(X, Y, kMiddle, hh);
                widget->box(FL_FLAT_BOX);
                widget->labelsize(kLabelSize);
                widget->copy_label(name);
                widget->color(colA);
                widget->labelcolor(FL_BLACK);
                g->add(widget);
            }
            m_curr->add(g);

            char buf[64];
            unsigned dw = (g->w() - kMiddle) / 6;
            Fl_Group* g2 = new Fl_Group(kMiddle, Y, g->w() - kMiddle, hh);
            g2->end();
            if (tooltip)
                g2->tooltip(tooltip);
            else
                g2->tooltip(lbl->label());
            {
                IntInput* widget = new IntInput(kMiddle, Y, dw, hh);
                snprintf(buf, 64, "%d", content.min.x);
                widget->value(buf);
                widget->align(FL_ALIGN_LEFT);
                widget->color(colB);
                widget->box(FL_FLAT_BOX);
                if (!editable)
                {
                    widget->deactivate();
                    widget->box(FL_FLAT_BOX);
                }
                else
                {
                    if (callback)
                        widget->callback(callback, this);
                }
                g2->add(widget);
            }
            {
                IntInput* widget = new IntInput(kMiddle + dw, Y, dw, hh);
                snprintf(buf, 64, "%d", content.min.y);
                widget->value(buf);
                widget->align(FL_ALIGN_LEFT);
                widget->box(FL_FLAT_BOX);
                widget->color(colB);
                if (!editable)
                {
                    widget->deactivate();
                    widget->box(FL_FLAT_BOX);
                }
                else
                {
                    if (callback)
                        widget->callback(callback, this);
                }
                g2->add(widget);
            }
            {
                IntInput* widget = new IntInput(kMiddle + dw * 2, Y, dw, hh);
                snprintf(buf, 64, "%d", content.max.x);
                widget->value(buf);
                widget->align(FL_ALIGN_LEFT);
                widget->box(FL_FLAT_BOX);
                widget->color(colB);
                if (!editable)
                {
                    widget->deactivate();
                    widget->box(FL_FLAT_BOX);
                }
                else
                {
                    if (callback)
                        widget->callback(callback, this);
                }
                g2->add(widget);
            }
            {
                IntInput* widget = new IntInput(kMiddle + dw * 3, Y, dw, hh);
                snprintf(buf, 64, "%d", content.max.y);
                widget->value(buf);
                widget->align(FL_ALIGN_LEFT);
                widget->box(FL_FLAT_BOX);
                widget->color(colB);
                if (!editable)
                {
                    widget->deactivate();
                    widget->box(FL_FLAT_BOX);
                }
                else
                {
                    if (callback)
                        widget->callback(callback, this);
                }
                g2->add(widget);
            }
            {
                IntInput* widget =
                    new IntInput(kMiddle + dw * 4, Y, dw, hh, "W:");
                snprintf(buf, 64, "%d", content.w());
                widget->value(buf);
                widget->align(FL_ALIGN_LEFT);
                widget->box(FL_FLAT_BOX);
                widget->color(colB);
                widget->labelcolor(FL_LIGHT3);
                widget->textcolor(FL_BLACK);
                widget->textsize(12);
                widget->deactivate();
                g2->add(widget);
            }
            {
                IntInput* widget =
                    new IntInput(kMiddle + dw * 5, Y, dw, hh, "H:");
                snprintf(buf, 64, "%d", content.h());
                widget->value(buf);
                widget->align(FL_ALIGN_LEFT);
                widget->box(FL_FLAT_BOX);
                widget->labelcolor(FL_LIGHT3);
                widget->textcolor(FL_BLACK);
                widget->color(colB);
                widget->deactivate();
                g2->add(widget);
            }
            m_curr->add(g2);
            m_curr->end();
        }

        void ImageInfoPanel::add_float(
            const char* name, const char* tooltip, const float content,
            const bool editable, const bool active, Fl_Callback* callback,
            const float minV, float maxV, const int when)
        {
            Fl_Color colA = get_title_color();
            Fl_Color colB = get_widget_color();

            Fl_Box* lbl;
            int hh = kLineHeight;

            Y += hh;
            Fl_Group* g = new Fl_Group(X, Y, kMiddle, hh);
            g->end();

            {
                Fl_Box* widget = lbl = new Fl_Box(X, Y, kMiddle, hh);

                widget->box(FL_FLAT_BOX);
                widget->labelcolor(FL_BLACK);
                widget->labelsize(kLabelSize);
                widget->copy_label(name);
                widget->color(colA);
                g->add(widget);
            }
            m_curr->add(g);

            {
                Fl_Group* p = new Fl_Group(kMiddle, Y, kMiddle, hh);
                p->box(FL_FLAT_BOX);
                p->begin();

                HorSlider* widget = new HorSlider(kMiddle, Y, p->w(), hh);
                widget->value(content);
                widget->default_value(content);
                widget->align(FL_ALIGN_LEFT);
                widget->color(colB);
                double maxS = maxV;
                if (content > 1000000 && maxV <= 1000000)
                    maxS = 1000000;
                if (content > 100000 && maxV <= 100000)
                    maxS = 1000000;
                else if (content > 10000 && maxV <= 10000)
                    maxS = 100000;
                else if (content > 1000 && maxV <= 1000)
                    maxS = 10000;
                else if (content > 100 && maxV <= 100)
                    maxS = 1000;
                else if (content > 10 && maxV <= 10)
                    maxS = 100;
                else if (content > maxS)
                    maxS = content + 50;

                widget->range(minV, maxS);
                widget->setEnabled(editable);
                widget->when(when);

                if (tooltip)
                    widget->tooltip(tooltip);
                else
                    widget->tooltip(lbl->label());

                if (callback)
                    widget->callback(callback, this);

                p->end();
                m_curr->add(p);
                if (!active)
                {
                    p->deactivate();
                }
            }
            m_curr->end();
        }

        void ImageInfoPanel::add_memory(
            const char* name, const char* tooltip, const std::uintmax_t content)
        {
            char buf[256];
            const char* space_type = nullptr;
            const double memory_space = to_memory(content, space_type);
            snprintf(buf, 256, "%.3f %s", memory_space, space_type);
            add_text(name, tooltip, buf);
        }

        void ImageInfoPanel::add_bool(
            const char* name, const char* tooltip, const bool content,
            const bool editable, Fl_Callback* callback)
        {
            Fl_Color colA = get_title_color();
            Fl_Color colB = get_widget_color();

            Fl_Box* lbl;

            int hh = kLineHeight;
            Y += hh;
            Fl_Group* g = new Fl_Group(X, Y, kMiddle, hh);
            g->end();

            {
                Fl_Box* widget = lbl = new Fl_Box(X, Y, kMiddle, hh);
                widget->box(FL_FLAT_BOX);
                widget->labelcolor(FL_BLACK);
                widget->labelsize(kLabelSize);
                widget->copy_label(name);
                widget->color(colA);
                g->add(widget);
            }
            m_curr->add(g);

            {
                Input* widget = new Input(kMiddle, Y, g->w() - kMiddle, 20);
                widget->value(content ? _("Yes") : _("No"));
                widget->box(FL_FLAT_BOX);
                widget->align(FL_ALIGN_LEFT);
                widget->color(colB);
                if (tooltip)
                    widget->tooltip(tooltip);
                else
                    widget->tooltip(lbl->label());
                if (!editable)
                {
                    widget->deactivate();
                    widget->box(FL_FLAT_BOX);
                }
                else
                {
                    if (callback)
                        widget->callback(callback, this);
                }
                m_curr->add(widget);
            }
            m_curr->end();
        }

        void ImageInfoPanel::fill_video_data()
        {
            m_video->hide();
            if (!m_video->is_open())
            {
                m_video->show();
                return;
            }

            m_video->clear();

            const auto& info = player->ioInfo();
            unsigned num_video_streams = info.video.size();

            if (num_video_streams > 0)
            {

                for (int i = 0; i < num_video_streams; ++i)
                {
                    char buf[256];

                    if (num_video_streams > 1)
                    {
                        snprintf(buf, 256, _("Video Stream #%d"), i + 1);
                        m_curr = add_browser(m_video, buf);
                    }
                    else
                    {
                        m_curr = add_browser(m_video);
                    }

                    const auto& video = info.video[i];
                    const auto& size = video.size;

                    add_text(
                        _("Name"), _("Name"), mrv::color::layer(video.name));

                    float rotation = 0.F;
                    std::string colorPrimaries;
                    std::string colorTRC;
                    std::string colorSpace;
                    std::string compression;
                    int compressionNumScanlines = video.compressionNumScanlines;
                    bool isLossyCompression = video.isLossyCompression;
                    bool isValidDeepCompression = video.isValidDeepCompression;
                    std::string HDRdata;
                    int xLevels = 0, yLevels = 0;

#ifdef TLRENDER_EXR
                    int mipmapMode = Imf::LevelMode::ONE_LEVEL;
                    int roundingMode = Imf::LevelRoundingMode::ROUND_DOWN;
#endif
                    
                    if (!tagData.empty())
                    {
                        auto it = tagData.find("Video Codec");
                        if (it != tagData.end())
                        {
                            add_text(_("Codec"), _("Codec"), it->second);
                        }
                        else
                        {
                            compression = video.compression;
                        }

#ifdef TLRENDER_EXR
                        it = tagData.find("Tile");
                        if (it != tagData.end())
                        {
                            std::stringstream s(it->second);
                            s >> mipmapMode >> mipmapMode >> mipmapMode >> roundingMode;
                        }
                        it = tagData.find("numXLevels");
                        if (it != tagData.end())
                        {
                            std::stringstream s(it->second);
                            s >> xLevels;
                        }
                        it = tagData.find("numYLevels");
                        if (it != tagData.end())
                        {
                            std::stringstream s(it->second);
                            s >> yLevels;
                        }
#endif
                        
                        it = tagData.find("Video Rotation");
                        if (it != tagData.end())
                        {
                            std::stringstream s(it->second);
                            s >> rotation;
                        }
                        it = tagData.find("Video Color Primaries");
                        if (it != tagData.end())
                        {
                            colorPrimaries = it->second;
                        }
                        it = tagData.find("Video Color TRC");
                        if (it != tagData.end())
                        {
                            colorTRC = it->second;
                        }
                        it = tagData.find("Video Color Space");
                        if (it != tagData.end())
                        {
                            colorSpace = it->second;
                        }
                        it = tagData.find("hdr");
                        if (it != tagData.end())
                        {
                            HDRdata = it->second;
                        }
                    }
                    else
                    {
                        add_text(_("Codec"), _("Codec"), _("Unknown"));
                    }

                    add_unsigned(
                        _("Width"), _("Width of clip"), (unsigned)size.w,
                        false);
                    add_unsigned(
                        _("Height"), _("Height of clip"), (unsigned)size.h,
                        false);

                    double aspect_ratio = (double)size.w / (double)size.h;

                    const char* name = _("Unknown");
                    int num = sizeof(kAspectRatioNames) / sizeof(AspectName);
                    constexpr double fuzz = 0.001;
                    for (int j = 0; j < num; ++j)
                    {
                        if (mrv::is_equal(
                                aspect_ratio, kAspectRatioNames[j].ratio, fuzz))
                        {
                            name = _(kAspectRatioNames[j].name);
                            break;
                        }
                    }

                    snprintf(buf, 256, "%g (%s)", aspect_ratio, name);
                    add_text(_("Aspect Ratio"), _("Aspect ratio of clip"), buf);

                    const auto view = _p->ui->uiView;
                    float pixelAspectRatio = view->getPixelAspectRatio();
                    if (pixelAspectRatio < 0.001F)
                        pixelAspectRatio = size.pixelAspectRatio;

                    add_float(
                        _("Pixel Ratio"), _("Pixel ratio of clip"),
                        pixelAspectRatio, true, true,
                        (Fl_Callback*)change_pixel_ratio_cb, 0.0f, 8.0f,
                        FL_WHEN_ENTER_KEY | FL_WHEN_CHANGED);

#ifdef TLRENDER_EXR
                    if (mipmapMode == Imf::MIPMAP_LEVELS)
                    {
                        ++group;
                        add_int(_("Mipmap Level"), _("Mipmap Level"),
                                xLevel,
                                true, true,
                                (Fl_Callback*)change_x_and_y_level_cb,
                                0, xLevels,
                                FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE);
                    }
                    else if (mipmapMode == Imf::RIPMAP_LEVELS)
                    {
                        ++group;
                        add_int(_("X Ripmap Level"), _("X Ripmap Level"),
                                xLevel, true, true,
                                (Fl_Callback*)change_xlevel_cb,
                                0, xLevels,
                                FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE);
                        add_int(_("Y Ripmap Level"), _("Y Ripmap Level"),
                                yLevel, true, true,
                                (Fl_Callback*)change_ylevel_cb,
                                0, yLevels,
                                FL_WHEN_ENTER_KEY | FL_WHEN_RELEASE);
                    }
                    if (mipmapMode != Imf::ONE_LEVEL)
                    {
                        std::string roundingModeText = _("DOWN");
                        if (roundingMode == Imf::LevelRoundingMode::ROUND_UP)
                            roundingModeText = _("UP");
                        add_text(_("Rounding Mode"), _("Rounding Mode"),
                                 roundingModeText);
                        ++group;
                    }
#endif
                            
                    if (rotation != 0.F)
                        add_float(_("Rotation"), _("Video Rotation"), rotation);

                    if (!compression.empty() && compression != "Unknown")
                    {
                        add_text(
                            _("Compression"), _("Compression"), compression);

                        if (compressionNumScanlines > 0)
                            add_int(
                                _("Compression Num. Scanlines"),
                                _("Number of Compression Scanlines"),
                                compressionNumScanlines);
                        add_bool(
                            _("Lossy Compression"), _("Lossy Compression"),
                            isLossyCompression);
                        add_bool(
                            _("Deep Compression"), _("Deep Compression"),
                            isValidDeepCompression);
                    }

                    ++group;

                    tl::image::PixelType pixelType = video.pixelType;
                    uint8_t pixelDepth = tl::image::getBitDepth(pixelType);
                    uint8_t channelCount =
                        tl::image::getChannelCount(pixelType);

                    const char* depth;
                    switch (pixelDepth)
                    {
                    case 8:
                        depth = _("unsigned byte (8-bits per channel)");
                        break;
                    case 10:
                        depth = _("(10-bits per channel)");
                        break;
                    case 12:
                        depth = _("(12-bits per channel)");
                        break;
                    case 16:
                        depth = _("unsigned short (16-bits per channel)");
                        if (pixelType == tl::image::PixelType::L_F16 ||
                            pixelType == tl::image::PixelType::LA_F16 ||
                            pixelType == tl::image::PixelType::RGB_F16 ||
                            pixelType == tl::image::PixelType::RGBA_F16)
                            depth = _("half float (16-bits per channel)");
                        break;
                    case 32:
                        depth = _("unsigned int (32-bits per channel)");
                        if (pixelType == tl::image::PixelType::L_F32 ||
                            pixelType == tl::image::PixelType::LA_F32 ||
                            pixelType == tl::image::PixelType::RGB_F32 ||
                            pixelType == tl::image::PixelType::RGBA_F32)
                            depth = _("float (32-bits per channel)");
                        break;
                    default:
                        depth = _("Unknown bit depth");
                        break;
                    }

                    add_text(_("Depth"), _("Bit depth of clip"), depth);

                    add_int(
                        _("Image Channels"), _("Number of channels in clip"),
                        channelCount, false);

                    std::vector< std::string > yuvCoeffs =
                        tl::image::getYUVCoefficientsLabels();
                    add_enum(
                        _("YUV Coefficients"),
                        _("YUV Coefficients used for video conversion"),
                        getLabel(video.yuvCoefficients), yuvCoeffs, false);

                    std::vector< std::string > videoLevels =
                        tl::image::getVideoLevelsLabels();
                    add_enum(
                        _("Video Levels"), _("Video Levels"),
                        getLabel(video.videoLevels), videoLevels, false);

                    if (!colorPrimaries.empty())
                    {
                        add_text(
                            _("Color Primaries"), _("Color Primaries"),
                            colorPrimaries);
                    }
                    if (!colorTRC.empty())
                    {
                        add_text(
                            _("Color TRC"), _("Color Transfer Characteristics"),
                            colorTRC);
                    }
                    if (!colorSpace.empty())
                    {
                        add_text(
                            _("Color Space"), _("Color Transfer Space"),
                            colorSpace);
                    }
                    if (!HDRdata.empty())
                    {
                        nlohmann::json json = nlohmann::json::parse(HDRdata);
                        image::HDRData hdr = json.get<image::HDRData>();

                        math::Vector2f& v =
                            hdr.primaries[image::HDRPrimaries::Red];
                        snprintf(buf, 256, "(%g) (%g)", v.x, v.y);
                        add_text(
                            _("HDR Red Primaries"), _("HDR Red Primaries"),
                            buf);

                        v = hdr.primaries[image::HDRPrimaries::Green];
                        snprintf(buf, 256, "(%g) (%g)", v.x, v.y);
                        add_text(
                            _("HDR Green Primaries"), _("HDR Green Primaries"),
                            buf);

                        v = hdr.primaries[image::HDRPrimaries::Blue];
                        snprintf(buf, 256, "(%g) (%g)", v.x, v.y);
                        add_text(
                            _("HDR Blue Primaries"), _("HDR Blue Primaries"),
                            buf);

                        v = hdr.primaries[image::HDRPrimaries::White];
                        snprintf(buf, 256, "(%g) (%g)", v.x, v.y);
                        add_text(
                            _("HDR White Primaries"), _("HDR White Primaries"),
                            buf);

                        const math::FloatRange& luminance =
                            hdr.displayMasteringLuminance;
                        snprintf(
                            buf, 256, "min: %g max: %g", luminance.getMin(),
                            luminance.getMax());
                        add_text(
                            _("HDR Display Mastering Luminance"),
                            _("HDR Display Mastering Luminance"), buf);

                        snprintf(buf, 256, "%g", hdr.maxCLL);
                        add_text(_("HDR maxCLL"), _("HDR maxCLL"), buf);

                        snprintf(buf, 256, "%g", hdr.maxFALL);
                        add_text(_("HDR maxFALL"), _("HDR maxFALL"), buf);
                    }
                    ++group;

                    std::string format;
                    auto it = info.tags.find("FFmpeg Pixel Format");
                    if (it != info.tags.end())
                        add_text(
                            _("FFmpeg Pixel Format"), _("FFmpeg Pixel Format"),
                            it->second);

                    format = tl::image::getLabel(pixelType);

                    add_text(
                        _("Render Pixel Format"), _("Render Pixel Format"),
                        format);

                    ++group;
                    if (!tagData.empty())
                    {
                        snprintf(buf, 256, "Video Stream #%d:", i + 1);
                        const std::string& match = buf;

                        for (const auto& tag : tagData)
                        {
                            std::string key = tag.first;
                            
                            auto i = key.find(match);
                            if (i != std::string::npos)
                            {
                                key = key.replace(i, match.size(), "");
                                add_text(key.c_str(), key.c_str(), tag.second);
                            }
                        }
                    }
                }

                m_video->show();
            }
        }

        void ImageInfoPanel::fill_image_data()
        {
            m_image->hide();

            if (!m_image->is_open())
            {
                m_image->show();
                return;
            }

            m_image->clear();

            char buf[1024];
            m_curr = add_browser(m_image);

            const auto& info = player->ioInfo();
            unsigned num_video_streams = info.video.size();

            unsigned num_audio_streams = 0;
            if (info.audio.isValid())
                num_audio_streams = info.audio.trackCount;

            const auto& path = player->path();
            const auto& directory = path.getDirectory();

            const auto& audioPath = player->audioPath();
            const otime::RationalTime& time = player->currentTime();

            const auto& fullname = createStringFromPathAndTime(path, time);

            add_text(
                _("Directory"), _("Directory where clip resides"), directory);

            add_text(_("Filename"), _("Filename of the clip"), fullname);

            if (!audioPath.isEmpty() && path != audioPath)
            {
                add_text(
                    _("Audio Directory"),
                    _("Directory where audio clip resides"),
                    audioPath.getDirectory());

                add_text(
                    _("Audio Filename"), _("Filename of the audio clip"),
                    audioPath.get(-1, tl::file::PathType::FileName));
            }

            ++group;

            add_int(
                _("Video Streams"), _("Number of video streams in file"),
                num_video_streams);
            add_int(
                _("Audio Streams"), _("Number of audio streams in file"),
                num_audio_streams);
            // add_int( _("Subtitle Streams"),
            //          _("Number of subtitle streams in file"),
            //          num_subtitle_streams );

            const auto& range = player->timeRange();
            const auto& startTime = range.start_time();
            const auto& endTime = range.end_time_inclusive();
            add_time(
                _("Start Time"), _("Beginning frame of clip"), startTime,
                false);
            add_time(_("End Time"), _("Ending frame of clip"), endTime, false);

            const otime::TimeRange& iorange = player->inOutRange();
            int64_t first = iorange.start_time().to_frames();
            int64_t last = iorange.end_time_inclusive().to_frames();

            add_int(
                _("First Frame"), _("First frame of clip - User selected"),
                (int)first, true, true, (Fl_Callback*)change_first_frame_cb,
                first, last);
            add_int(
                _("Last Frame"), _("Last frame of clip - User selected"),
                (int)last, true, true, (Fl_Callback*)change_last_frame_cb,
                first, last);

            const char* name = "";
            double fps = player->defaultSpeed();

            if (is_equal(fps, 29.97))
                name = "(NTSC)";
            else if (is_equal(fps, 30.0))
                name = "(60hz HDTV)";
            else if (is_equal(fps, 25.0))
                name = "(PAL)";
            else if (is_equal(fps, 24.0))
                name = "(Film)";
            else if (is_equal(fps, 50.0))
                name = _("(PAL Fields)");
            else if (is_equal(fps, 59.940059))
                name = _("(NTSC Fields)");

            snprintf(buf, 256, "%g %s", fps, name);

            add_text(
                _("Default Speed"), _("Default Speed in Frames per Second"),
                buf);

            fps = player->speed();
            add_float(
                _("Current Speed"), _("Current Speed (Frames Per Second)"), fps,
                true, true, (Fl_Callback*)change_fps_cb, 1.0f, 60.0f,
                FL_WHEN_RELEASE);

            ++group;

            struct stat file_stat;
            const std::string& filename = directory + fullname;

            if (stat(filename.c_str(), &file_stat) == 0)
            {
                // Get file size
                std::uintmax_t filesize = fs::file_size(filename);
                add_memory(_("Disk space"), _("Disk space"), filesize);

                // Retrieve last modification time
                time_t mod_time = file_stat.st_mtime;
                time_t creation_time = file_stat.st_ctime;

                // Format time according to current locale
                strftime(buf, sizeof(buf), "%c", localtime(&creation_time));
                add_text(_("Creation Date"), _("Creation date of file"), buf);

                // Format time according to current locale
                strftime(buf, sizeof(buf), "%c", localtime(&mod_time));
                add_text(
                    _("Modified Date"), _("Last modified date of file"), buf);
            }

            m_image->end();
            m_image->show();
        }

        void ImageInfoPanel::fill_data()
        {
            if (!player)
            {
                g->tooltip(_("Load an image or movie file"));
                return;
            }
            else
            {
                g->tooltip("");
            }

            // Refresh the dock size

            kMiddle = g->w() / 2;

            const auto& info = player->ioInfo();
            unsigned num_video_streams = info.video.size();

            unsigned num_audio_streams = 0;
            if (info.audio.isValid())
                num_audio_streams = info.audio.trackCount;

            // @todo: tlRender does not handle subtitle tracks
            unsigned num_subtitle_streams = 0;

            fill_image_data();

            // First, check the metadata
            fill_video_data();

            if (num_audio_streams > 0)
            {
                for (int i = 0; i < num_audio_streams; ++i)
                {
                    char buf_english[256];
                    char buf[256];

                    snprintf(buf_english, 256, "Audio Stream #%d:", i + 1);
                    snprintf(buf, 256, _("Audio Stream #%d"), i + 1);
                    if (num_audio_streams > 1)
                    {
                        m_curr = add_browser(m_audio, buf);
                    }
                    else
                    {
                        m_curr = add_browser(m_audio);
                    }

                    // tlRender handles only one audio track per clip.
                    // But we added support in FFmpegReadAudio to get the info
                    // of all audio tracks which we store in info.audioInfo.
                    //
                    // Switching from one track to another currently involves
                    // cloning and reopening the movie unfortunately.
                    //
                    auto audio = info.audio;
                    if (i > 0)
                        audio = *info.audio.audioInfo[i];

                    auto it = info.tags.find("Audio Codec");
                    if (it != info.tags.end())
                        add_text(_("Codec"), _("Codec"), it->second);

#if 0
                    add_text( _("FourCC"), _("Four letter ID"), s.fourcc );
#endif
                    ++group;

                    const char* channels = "Stereo";
                    switch (audio.channelCount)
                    {
                    case 1:
                        channels = "Mono";
                        break;
                    case 6:
                        channels = "5:1";
                        break;
                    case 8:
                        channels = "7:1";
                        break;
                    default:
                        snprintf(buf, 256, "%zu", audio.channelCount);
                        channels = buf;
                        break;
                    }

                    add_text(
                        _("Channels"), _("Number of audio channels"), channels);

                    add_text(
                        _("Format"), _("Format"), getLabel(audio.dataType));
                    snprintf(buf, 256, _("%d Hz."), audio.sampleRate);
                    add_text(_("Frequency"), _("Frequency of audio"), buf);

#if 0
                    snprintf( buf, 256, _("%d kb/s"), s.bitrate/1000 );
                    add_text( _("Max. Bitrate"), _("Max. Bitrate"), buf );
#endif

                    ++group;

#if 0
                    add_text( _("Disposition"), _("Disposition of Track"),
                              s.disposition);
                    ++group;

                    add_time( _("Start"), _("Start of Audio"), s.start, fps );
                    add_time( _("Duration"), _("Duration of Audio"),
                              s.duration, fps );
#endif

                    if (!tagData.empty())
                    {
                        ++group;
                        const std::string& match = buf_english;
                        for (const auto& tag : tagData)
                        {
                            std::string key = tag.first;
                            auto i = key.find(match);
                            if (i != std::string::npos)
                            {
                                key = key.replace(i, match.size(), "");
                                add_text(key.c_str(), key.c_str(), tag.second);
                            }
                        }
                    }
                }

                m_audio->show();
            }

#if 0
            if ( num_subtitle_streams > 0 )
            {
                for ( int i = 0; i < num_subtitle_streams; ++i )
                {
                    char buf[256];

                    if ( num_subtitle_streams > 1 )
                    {
                        snprintf( buf, 256, _("Subtitle Stream #%d"), i+1 );
                        m_curr = add_browser( m_subtitle, buf );
                    }
                    else
                    {
                        m_curr = add_browser( m_subtitle, );
                    }

                    const Media::subtitle_info_t& s = img->subtitle_info(i);

                    add_text( _("Codec"), _("Codec Name"), s.codec_name );
                    add_text( _("FourCC"), _("Four letter ID"), s.fourcc );
                    add_bool( _("Closed Captions"), _("Video has Closed Captions"),
                              s.closed_captions );
                    ++group;

                    snprintf( buf, 256, _("%d kb/s"), s.bitrate/1000 );
                    add_text( _("Avg. Bitrate"), _("Avg. Bitrate"), buf );

                    ++group;
                    add_text( _("Language"), _("Language if known"), s.language );
                    ++group;
                    add_text( _("Disposition"), _("Disposition of Track"),
                              s.disposition );

                    ++group;
                    add_time( _("Start"), _("Start of Subtitle"), s.start, img->fps() );
                    add_time( _("Duration"), _("Duration of Subtitle"),
                              s.duration, img->fps() );
                }

                m_subtitle->show();
            }
#endif

            fill_metadata();
        }

        void ImageInfoPanel::fill_metadata()
        {
            m_attributes->hide();

            if (tagData.empty())
                return;

            if (!m_attributes->is_open())
            {
                bool found_data = false;

                for (const auto& item : tagData)
                {
                    bool skip = false;
                    if (item.first.substr(0, 5) == "Video" ||
                        item.first.substr(0, 5) == "Audio" ||
                        item.first.substr(0, 19) == "FFmpeg Pixel Format")
                    {
                        skip = true;
                    }
                    if (skip)
                    {
                        continue;
                    }
                    found_data = true;
                }
                if (found_data)
                    m_attributes->show();
                return;
            }

            m_attributes->clear();

            m_curr = add_browser(m_attributes);

            bool found_data = false;

            for (const auto& item : tagData)
            {
                bool skip = false;
                if (item.first == "hdr" ||
                    item.first.substr(0, 5) == "Video" ||
                    item.first.substr(0, 5) == "Audio" ||
                    item.first.substr(0, 19) == "FFmpeg Pixel Format" ||
                    item.first.substr(0, 12) == "otioClipName" ||
                    item.first.substr(0, 12) == "otioClipTime")
                {
                    skip = true;
                }
                if (skip)
                {
                    continue;
                }
                found_data = true;
                add_text(_(item.first.c_str()), "", _(item.second.c_str()));
            }

            m_attributes->end();

            if (found_data)
                m_attributes->show();
        }

        void ImageInfoPanel::getTags()
        {
            tagData.clear();

            if (!player)
                return;

            const auto& info = player->ioInfo();

            // First, add global tags
            for (const auto& tag : info.tags)
            {
                tagData[tag.first] = tag.second;
            }

            const auto view = _p->ui->uiView;
            const auto videoData = view->getVideoData();

            // Then add image tags
            if (!videoData.empty() && !videoData[0].layers.empty() &&
                videoData[0].layers[0].image)
            {
                const image::Tags& tags =
                    videoData[0].layers[0].image->getTags();
                for (const auto& tag : tags)
                {
                    tagData[tag.first] = tag.second;
                }
            }
        }
    } // namespace panel

} // namespace mrv
