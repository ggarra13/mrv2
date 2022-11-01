/*
    mrViewer - the professional movie and flipbook playback
    Copyright (C) 2007-2022  Gonzalo Garramuño

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * @file   mainWindow.h
 * @author gga
 * @date   Fri Jul  6 14:31:50 2007
 *
 * @brief
 *
 *
 */

#pragma once

#include <iostream>

//#include <FL/Fl_Double_Window.H>
#include "mrvDropWindow.h"

#ifdef __APPLE__
#  include <IOKit/pwr_mgt/IOPMLib.h>
#endif

class Fl_Menu_;

class ViewerUI;

namespace mrv {


class App;

class MainWindow : public DropWindow
{
public:
    MainWindow( int W, int H, const char* title );
    ~MainWindow();

    void main( ViewerUI* m ) { ui = m; };
    ViewerUI* main() const { return ui; }

    //! Fill menu based on context information
    void fill_menu( Fl_Menu_* menu );

    //! Make window appear always on top of others
    void always_on_top(int above);

    //! Change window's icon to mrViewer's icon
    void set_icon();
    
    //! Iconize all windows
    void iconize_all();


protected:

#ifdef __APPLE__
    IOPMAssertionID assertionID;
    IOReturn success;
#endif

    ViewerUI*   ui = nullptr;
};


} // namespace mrv
