#pragma once

#include <FL/Fl_Double_Window.H>

#include <string>

class Fl_Box;

namespace mrv
{
    class Tooltip : public Fl_Double_Window
    {
    public:
        Tooltip(int X, int Y, int W, int H, const char* L = 0);

        static void hide_cb(Tooltip*);

        void copy_label(const std::string&);
        void show();
    private:
        Fl_Box* box;
    };
}
