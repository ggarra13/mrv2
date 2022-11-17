#include <FL/Fl_Multiline_Input.H>
#include <FL/Enumerations.H>

namespace mrv
{

    class MultilineInput : public Fl_Multiline_Input
    {
    public:
        MultilineInput( int X, int Y, int W, int H, char* l = 0 ) :
            Fl_Multiline_Input( X, Y, W, H, l )
            {
                box( FL_ROUNDED_BOX );
                color(FL_FREE_COLOR);
                wrap( false );
                tab_nav( false );
            };

        virtual ~MultilineInput() { };

        void font_size( double f ) { _font_size = f; }
        double font_size() const { return _font_size; }

        int accept();

        virtual int handle( int e );
        virtual void draw();

    protected:
        double _font_size;
    };

} // namespace mrv
