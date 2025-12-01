
#include <FL/Fl_Double_Window.H>

class MainWindow : public Fl_Double_Window
{
public:
    MainWindow(int W, int H, const char* L) :
        Fl_Double_Window(W, H, L)
        {
        }

    void always_on_top();
};
