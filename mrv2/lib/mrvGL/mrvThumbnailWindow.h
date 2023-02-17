// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.



#pragma once

#include <FL/Fl_Gl_Window.H>

namespace mrv
{
	class ThumbnailWindow : public Fl_Gl_Window
	{
	public:
		ThumbnailWindow( int X, int Y, int W, int H ) :
			Fl_Gl_Window( X, Y, W, H )
			{
				mode( FL_RGB | FL_DOUBLE | FL_ALPHA | FL_OPENGL3 | FL_STENCIL );
				border(0);
			}
	};
}
