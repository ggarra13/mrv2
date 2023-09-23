
#include <string>

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Button.H>

namespace mrv
{
    class TextErase : public Fl_Double_Window
    {
        Fl_Button deleteButton;

    public:
        TextErase(int W, int H, const char* L = 0);

        void add(const std::string&);

        Fl_Choice textAnnotations;
    };
} // namespace mrv
