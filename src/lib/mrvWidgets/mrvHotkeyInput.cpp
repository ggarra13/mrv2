// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cstring>

#include <FL/Fl.H>

#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvHotkey.h"

#include "mrvWidgets/mrvHotkeyInput.h"

#include "keyboard_ui.h"

namespace mrv
{
    HotkeyInput::HotkeyInput(int X, int Y, int W, int H, const char* L) :
        Fl_Input(X, Y, W, H, L)
    {
        textcolor(FL_BLACK);
        cursor_color(FL_RED);
    }

    int HotkeyInput::handle(int e)
    {
        if (e == FL_KEYBOARD)
        {
            char buffer[1000];
            buffer[0] = '\0';
            m_text.clear();
            m_key = static_cast<unsigned>(Fl::event_key());
            if (m_key < 128)
            { // ASCII
                snprintf(buffer, sizeof(buffer), "%c", m_key);
            }
            else if (m_key >= 0xa0 && m_key <= 0xff)
            { // ISO-8859-1 (international keyboards)
                char key[8];
                int kl = fl_utf8encode((unsigned)m_key, key);
                key[kl] = '\0';
                snprintf(buffer, sizeof(buffer), "%s", key);
            }
            else if (m_key >= FL_F && m_key <= FL_F + FL_F_Last)
            {
                snprintf(buffer, sizeof(buffer), "F%d", m_key - FL_F);
            }

            for (int i = 0; i < sizeof(table) / sizeof(TableText); i++)
            {
                if (table[i].n == m_key)
                {
                    m_text = _(mrv::table[i].text);
                    break;
                }
            }

            bool m_ctrl = false;
            bool m_shift = false;
            bool m_alt = false;
            bool m_meta = false;

            if (m_key != FL_Control_R && m_key != FL_Control_L)
                m_ctrl = Fl::event_state(FL_CTRL);
            if (m_key != FL_Shift_R && m_key != FL_Shift_L)
                m_shift = Fl::event_state(FL_SHIFT);
            if (m_key != FL_Alt_R && m_key != FL_Alt_L)
                m_alt = Fl::event_state(FL_ALT);
            if (m_key != FL_Meta_R && m_key != FL_Meta_L)
                m_meta = Fl::event_state(FL_META);

            if (m_text.empty())
            {
                m_text = buffer;

                if (m_text.empty())
                {
                    char key = Fl::event_text()[0];
                    if (key != '?' && key != '!' && key != '^' && key != '\0')
                        m_text = Fl::event_text();
                }
            }

            value(m_text.c_str());

            if (hotkey)
            {
                hotkey->uiCtrl->value(m_ctrl);
                hotkey->uiShift->value(m_shift);
                hotkey->uiAlt->value(m_alt);
                hotkey->uiMeta->value(m_meta);
            }
            return 1;
        }
        return Fl_Input::handle(e);
    }
} // namespace mrv
