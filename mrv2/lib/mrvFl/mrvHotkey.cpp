// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <regex>

#include "FL/Enumerations.H"

#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvHotkey.h"

#include "mrvWidgets/mrvBrowser.h"

#include "mrvFl/mrvAsk.h"
#include "mrvFl/mrvHotkey.h"

#include "keyboard_ui.h"
#include "mrvHotkeyUI.h"

namespace mrv
{

    void fill_ui_hotkeys(mrv::Browser* b)
    {
        int r = b->vposition();
        b->type(FL_SELECT_BROWSER);
        b->clear();

        int w2 = b->w() / 4;
        int w1 = w2 * 3;
        static int widths[] = {w1 - 20, w2, 0};
        b->column_widths(widths);
        // b->showcolsep(1);
        // b->colsepcolor(FL_RED);
        b->column_char('\t'); // tabs as column delimiters

        // Labels
        b->add(_("@B12@C7@b@.Function\t@B12@C7@b@.Hotkey"));

        for (int i = 0; hotkeys[i].name != "END"; ++i)
        {
            HotkeyEntry& h = hotkeys[i];
            std::string row(_(h.name.c_str()));
            row += "\t" + h.hotkey.to_s();

            b->add(row.c_str());
        }

        b->vposition(r);
    }

    void select_hotkey(HotkeyUI* b)
    {
        int idx =
            b->uiFunction->value() - 2; // 1 for browser offset, 1 for title
        if (idx < 0)
            return;

        const std::string& name = hotkeys[idx].name;
        Hotkey& hk = hotkeys[idx].hotkey;

        ChooseHotkey* h = new ChooseHotkey(hk);
        h->make_window(name);
        h->fill();

        Fl_Window* window = h->uiMain;
        Fl_Group::current(0);
        window->show();

        while (window->visible())
            Fl::check();

        for (int i = 0; hotkeys[i].name != "END"; ++i)
        {
            bool view3d = false;
            if (i < 4)
                view3d = true;

            if (h->hk == hotkeys[i].hotkey && idx != i &&
                (idx > 4 && !view3d) && hotkeys[i].hotkey.to_s() != "[" &&
                hotkeys[i].hotkey.to_s() != "]")
            {
                int ok = fl_choice(
                    _("Hotkey \"%s\" already used in \"%s\".\n"
                      "Do you want to override it?"),
                    _("Yes"), _("No"), 0L, h->hk.to_s().c_str(),
                    _(hotkeys[i].name.c_str()));
                if (ok)
                {
                    delete h;
                    return select_hotkey(b);
                }
                else
                {
                    hotkeys[i].hotkey.clear();
                }
            }
        }

        hk = h->hk;

        delete h;

        b->uiHotkeyFile->value(mrv::Preferences::hotkeys_file.c_str());
        fill_ui_hotkeys(b->uiFunction);
    }

    void searchFunction(const std::string& text, mrv::Browser* o)
    {
        if (text.empty())
        {
            o->select(o->value(), 0);
            o->topline(0);
            return;
        }
        std::regex regex{text, std::regex_constants::icase};
        int start = o->topline() + 1;
        for (int i = start; i < o->size(); ++i)
        {
            std::string function = o->text(i);
            std::size_t pos = function.find('\t');
            function = function.substr(0, pos - 1);
            if (std::regex_search(function, regex))
            {
                o->topline(i);
                o->select(i);
                return;
            }
        }
    }

    void searchHotkey(const std::string& text, mrv::Browser* o)
    {
        if (text.empty())
        {
            o->select(o->value(), 0);
            o->topline(0);
            return;
        }
        std::regex regex{text, std::regex_constants::icase};
        int start = o->topline() + 1;
        for (int i = start; i < o->size(); ++i)
        {
            std::string hotkey = o->text(i);
            std::size_t pos = hotkey.find('\t');
            hotkey = hotkey.substr(pos + 1, hotkey.size());
            if (std::regex_search(hotkey, regex))
            {
                o->topline(i);
                o->select(i);
                return;
            }
        }
    }

} // namespace mrv
