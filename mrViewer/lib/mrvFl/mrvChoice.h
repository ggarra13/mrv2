
#ifndef mrvChoice_h
#define mrvChoice_h

#include <FL/Fl_Choice.H>


namespace mrv {

    class Choice : public Fl_Choice
    {
    public:
        Choice( int x, int y, int w, int h, const char* l = 0 );

        virtual ~Choice() {};
      
        unsigned children();
        Fl_Menu_Item* child( int idx );
    };

}


#endif
