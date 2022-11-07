#ifndef mrvTree_h
#define mrvTree_h

#include <FL/Fl_Tree.H>

namespace mrv
{

class PreferencesTree : public Fl_Tree
{
  public:
    PreferencesTree( int X, int Y, int W, int H, const char* l = 0 );
    ~PreferencesTree();

    virtual void draw();
};


} // namespace mrv


#endif
