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


#include <FL/Enumerations.H>

#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvHotkey.h"
#include "mrvFl/mrvAsk.h"
#include "mrvFl/mrvHotkey.h"
#include "mrvFl/mrvBrowser.h"

#include "keyboard_ui.h"
#include "mrvHotkeyUI.h"

namespace mrv
{

void fill_ui_hotkeys( mrv::Browser* b )
{
    int r = b->position();
    b->type( FL_SELECT_BROWSER );
    b->clear();

    int w2 = b->w() / 4;
    int w1 = w2 * 3;
    static int widths[] = {w1-20, w2, 0 };
    b->column_widths( widths );
    //b->showcolsep(1);
    //b->colsepcolor(FL_RED);
    b->column_char('\t');  // tabs as column delimiters


    // Labels
    b->add( _("@B12@C7@b@.Function\t@B12@C7@b@.Hotkey") );

    for ( int i = 0; hotkeys[i].name != "END"; ++i )
    {
        HotkeyEntry& h = hotkeys[i];
        std::string row( _(h.name.c_str()) );
        row += "\t" + h.hotkey.to_s();

        b->add( row.c_str() );
    }

    b->position( r );
}

void select_hotkey( HotkeyUI* b )
{
    int idx = b->uiFunction->value() - 2;  // 1 for browser offset, 1 for title
    if ( idx < 0 ) return;

    Hotkey& hk = hotkeys[idx].hotkey;

    ChooseHotkey* h = new ChooseHotkey(hk);
    h->make_window();
    h->fill();

    Fl_Window* window = h->uiMain;
    Fl_Group::current( 0 );
    window->show();

    while ( window->visible() )
        Fl::check();


    for ( int i = 0; hotkeys[i].name != "END"; ++i )
      {
          bool view3d = false;
          if ( i < 4 ) view3d = true;


          if ( h->hk == hotkeys[i].hotkey && idx != i && (idx > 4 && !view3d) &&
               hotkeys[i].hotkey.to_s() != "[" &&
               hotkeys[i].hotkey.to_s() != "]"  )
            {
                int ok = fl_choice( _("Hotkey \"%s\" already used in \"%s\".\n"
                                      "Do you want to override it?"),
                                    _("Yes"), _("No"), 0L,
                                    h->hk.to_s().c_str(),
                                    _(hotkeys[i].name.c_str())
                                    );
                if ( ok )
                  {
                      delete h;
                      return select_hotkey( b );
                  }
                else
                  {
                      hotkeys[i].hotkey.clear();
                  }
            }
      }

    hk = h->hk;

    delete h;

    b->uiHotkeyFile->value( mrv::Preferences::hotkeys_file.c_str() );
    fill_ui_hotkeys( b->uiFunction );
}


} // namespace mrv
