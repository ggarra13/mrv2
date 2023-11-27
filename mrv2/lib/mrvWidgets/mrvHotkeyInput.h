#pragma once

#include <string>

#include <FL/Fl_Input.H>
#include <FL/Fl_Window.H>

class ChooseHotkey;

namespace mrv
{
    class HotkeyInput : public Fl_Input
    {
    public:
        HotkeyInput(int X, int Y, int W, int H, const char* L = 0);

        int handle(int) FL_OVERRIDE;

        void choose(ChooseHotkey* h) { hotkey = h; }

    protected:
        unsigned m_key = 0;
        std::string m_text;
        ChooseHotkey* hotkey = nullptr;
    };

} // namespace mrv
