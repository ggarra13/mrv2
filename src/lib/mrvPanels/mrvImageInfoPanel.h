// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <cinttypes> // for PRId64
#include <map>
#include <memory>
#include <string>

#include <tlCore/Util.h>
#include <tlCore/Box.h>

#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Scroll.H>

#include "mrvCore/mrvMedia.h"
#include "mrvCore/mrvString.h"

#include "mrvWidgets/mrvPopupMenu.h"
#include "mrvWidgets/mrvBrowser.h"
#include "mrvWidgets/mrvSlider.h"

#include "mrvPanels/mrvPanelWidget.h"

class Fl_Box;
class Fl_Input;
class Fl_Flex;
class ViewerUI;

namespace mrv
{
    class Pack;
    class TimelinePlayer;
    class CollapsibleGroup;
    class Table;

    namespace panel
    {

        class ImageInfoPanel : public PanelWidget
        {

        public:
            ImageInfoPanel(ViewerUI* ui);
            ~ImageInfoPanel();

            //! Return X Level
            int getXLevel() const { return xLevel; }
            
            //! Set X Mipmap level.
            void setXLevel(int X) { xLevel = X; }
            
            //! Return X Level
            int getYLevel() const { return yLevel; }
            
            //! Set Y Mipmap level.
            void setYLevel(int X) { yLevel = X; }

            
            //! Get the tags.
            void getTags();

            //! Refresh all tabs.
            void refresh();

            //! Quickly refresh the image tab if open.
            void imageRefresh();

            //! Quickly refresh the video tab if open.
            void videoRefresh();

            //! Quickly refresh the metadata tab if open.
            void metadataRefresh();

            TimelinePlayer* timelinePlayer() const;
            void setTimelinePlayer(TimelinePlayer* p);

            void save() override;

            void scroll_to(int w, int h);

            Pack* get_pack() const { return g->get_pack(); };

        protected:
            Fl_Color get_title_color();
            Fl_Color get_widget_color();

            void clear_callback_data();

            void hide_tabs();
            void save_tabs() const;

            static void enum_cb(mrv::PopupMenu* w, ImageInfoPanel* v);

            static void toggle_tab(Fl_Widget* w, void* data);
            static void int_slider_cb(Fl_Slider* w, void* data);
            static void float_slider_cb(Fl_Slider* w, void* data);

            double to_memory(std::uintmax_t value, const char*& extension);

            Table* add_browser(mrv::CollapsibleGroup* g);
            Table* add_browser(mrv::CollapsibleGroup* g, const char* label);

            void add_button(
                const char* name, const char* tooltip,
                Fl_Callback* callback = NULL, Fl_Callback* callback2 = NULL);

            void add_scale(
                const char* name, const char* tooltip, int pressed,
                int num_scales, Fl_Callback* callback = NULL);

            void add_ocio_ics(
                const char* name, const char* tooltip, const char* content,
                const bool editable = true, Fl_Callback* callback = NULL);

            void add_text(
                const char* name, const char* tooltip, const char* content,
                const bool editable = false, const bool active = false,
                Fl_Callback* callback = NULL);
            void add_text(
                const char* name, const char* tooltip,
                const std::string& content, const bool editable = false,
                const bool active = false, Fl_Callback* callback = NULL);
            void add_float(
                const char* name, const char* tooltip, const float content,
                const bool editable = false, const bool active = false,
                Fl_Callback* callback = NULL, const float minV = 0.0f,
                const float maxV = 1.0f, const int when = FL_WHEN_RELEASE);
            void add_rect(
                const char* name, const char* tooltip,
                const tl::math::Box2i& content, const bool editable = false,
                Fl_Callback* callback = NULL);

            void add_time(
                const char* name, const char* tooltip,
                const otime::RationalTime& content,
                const bool editable = false);

            void add_enum(
                const char* name, const char* tooltip, const size_t content,
                const char* const* options, const size_t num,
                const bool editable = false, Fl_Callback* callback = NULL);

            void add_enum(
                const char* name, const char* tooltip,
                const std::string& content, std::vector<std::string>& options,
                const bool editable = false, Fl_Callback* callback = NULL);

            void add_int64(
                const char* name, const char* tooltip, const int64_t content);

            void add_int(
                const char* name, const char* tooltip, const int content,
                const bool editable = false, const bool active = true,
                Fl_Callback* callback = NULL, const int minV = 0,
                const int maxV = 10, const int when = FL_WHEN_CHANGED);
            void add_unsigned(
                const char* name, const char* tooltip,
                const unsigned int content, const bool editable = false,
                const bool active = true, Fl_Callback* callback = NULL,
                const unsigned int minV = 0, const unsigned int maxV = 9999,
                const int when = FL_WHEN_CHANGED);
            void add_bool(
                const char* name, const char* tooltip, const bool content,
                const bool editable = false, Fl_Callback* callback = NULL);

            void add_memory(
                const char* name, const char* tooltip,
                const std::uintmax_t content);

            void add_controls() override;
            void fill_data();

            void fill_image_data();
            void fill_video_data();
            void fill_metadata();

        public:
            Fl_Flex* flex;
            CollapsibleGroup* m_image;
            CollapsibleGroup* m_video;
            CollapsibleGroup* m_audio;
            CollapsibleGroup* m_subtitle;
            CollapsibleGroup* m_attributes;
            Fl_Input* m_entry;
            Fl_Choice* m_type;

            //! Flag used in callbacks to avoid refreshing the panel on
            //! media refresh.
            bool m_update = true;

        protected:
            int kMiddle;
            Table* m_curr;
            Fl_Color m_color;
            unsigned int group;
            unsigned int row;
            unsigned int X, Y, W, H;
            TimelinePlayer* player = nullptr;
            std::map<std::string, std::string, string::CaseInsensitiveCompare>
                tagData;

            unsigned int xLevel = 0, yLevel = 0;
            
        public:
            Fl_Menu_Button* menu = nullptr;

        private:
            MRV2_PRIVATE();
        };

    } // namespace panel

} // namespace mrv
