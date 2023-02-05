// SPDX-License-Identifier: BSD-3-Clause
// mrv2 
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Flex.H>

#include "mrvDockGroup.h"

namespace mrv
{

class DropWindow : public Fl_Double_Window
{
protected:
	void init(void);
	DockGroup *dock;

public:
	// Normal FLTK constructors
	DropWindow(int x, int y, int w, int h, const char *l = 0);
	DropWindow(int w, int h, const char *l = 0);

	// The working area of this window
	Fl_Flex *workspace;

	// override handle method to capture "drop" events
	int handle(int);

	// assign a dock widget to this window
	void set_dock(DockGroup *d) {dock = d;}

};

}
