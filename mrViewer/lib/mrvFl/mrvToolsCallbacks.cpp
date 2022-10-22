#include <iostream>

#include "mrvToolsCallbacks.h"

namespace mrv {

  ColorTool* colorTool = nullptr;
    
    void color_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
      if ( colorTool ) return;
      colorTool = new ColorTool( ui );
    }

}
