#pragma once

#include <FL/Fl_Window.H>

class ViewerUI;

namespace mrv {

    class GLViewport;

    class ActionWindow : public Fl_Window
    {
    public:
        ActionWindow( int w, int h, const char* const  lbl = 0 ) :
            Fl_Window( w, h, lbl )
            {
            }

        void main( ViewerUI* m ) {
            uiMain = m;
        }

        ViewerUI* main() const {
            return uiMain;
        }

        GLViewport* view() const;

        virtual int handle( int event ) override;

    protected:
        ViewerUI* uiMain;
    };


}
