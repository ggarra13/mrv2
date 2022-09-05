#include <FL/Fl_XPM_Image.H>
#include "mrvCore/mrvPreferences.h"
#include "mrvFl/FLU/flu_pixmaps.h"
#include "mrvFl/mrvTree.h"

namespace mrv
{

PreferencesTree::PreferencesTree( int x, int y, int w, int h, const char* l ) :
Fl_Tree( x, y, w, h, l )
{
    {
        Fl_Pixmap closed_icon( folder_closed_xpm );
        Fl_RGB_Image* img = new Fl_RGB_Image( &closed_icon );
        openicon( img );
    }


    {
        Fl_Pixmap opened_icon( folder_open_xpm );
        Fl_RGB_Image* img = new Fl_RGB_Image( &opened_icon );
        closeicon( img );
    }

    Fl_Tree::item_labelfgcolor(FL_BLACK);

}

void PreferencesTree::draw()
{
    Fl_Tree::draw();
}

PreferencesTree::~PreferencesTree()
{
    delete openicon();
    delete closeicon();
}

}
