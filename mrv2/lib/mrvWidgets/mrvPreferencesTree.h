// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

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
