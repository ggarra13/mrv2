// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>

#include <FL/Fl_Menu_Button.H>

#include "mrvCore/mrvI8N.h"

#include "mrvWidgets/mrvTextBrowser.h"

namespace
{
    std::string replaceTabsAndAlign(const std::string& str, mrv::TextBrowser* o)
    {
        std::string out;
        int column = 0;
        int spaces_used = 0;

        const int* column_widths = o->column_widths();
        int font_size = o->textsize();
        
        for (char c : str)
        {
            if (c == '\t')
            {
                int width = column_widths[column] / font_size;
                if (width <= 0) continue;
                
                // Calculate spaces needed to reach the column
                int spaces_to_add = width - spaces_used + 1;
                if (spaces_to_add <= 0) continue;
                
                out.append(spaces_to_add, ' ');
                spaces_used = 0; // Reset counter for next word
                ++column;
            }
            else
            {
                out.push_back(c);
                spaces_used++; // Increment for non-whitespace characters
            }
        }

        return out;
    }
    
    void copy_text_cb(Fl_Menu_* m, mrv::TextBrowser* o)
    {
        std::string buffer;
        for (int i = 0; i <= o->size(); ++i)
        {
            if (o->selected(i))
            {
                const char* linestr = o->text(i);
                if (!linestr) continue;
                const std::string& line = linestr;
                buffer += replaceTabsAndAlign(line, o);
                buffer += "\n";
            }
        }
        Fl::copy(buffer.c_str(), buffer.size(), 2);
    }
}


namespace mrv
{
    

    // CTOR
    TextBrowser::TextBrowser(int X, int Y, int W, int H, const char* L) :
        Fl_Select_Browser(X, Y, W, H, L)
    {
        type(FL_MULTI_BROWSER);
    }

    int TextBrowser::handle(int e)
    {
        if (e == FL_PUSH)
        {
            if (Fl::event_button3())
            {
                Fl_Menu_Button* menu = new Fl_Menu_Button(0, 0, 0, 0);
                
                menu->type(Fl_Menu_Button::POPUP3);
                menu->add(
                    _("&Copy"), FL_COMMAND + 'c', (Fl_Callback*)copy_text_cb,
                    this);
                menu->popup();
                
                delete menu;
                return 1;
            }
        }
            
        return (Fl_Select_Browser::handle(e));
    }
} // namespace mrv
