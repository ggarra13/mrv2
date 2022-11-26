

#include <cstring>
#include <cstdlib>
#include <cstdio>

#include <iostream>
using namespace std;

#include <FL/Fl_Button.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>

#include "mrvCollapsibleGroup.h"

#define BUTTON_H        20
#define GROUP_MARGIN    8               // compensates for FL_ROUND_BUTTON (?)

namespace mrv {

  // Change button label based on open/closed state of pack
  void CollapsibleGroup::relabel_button() {
    int open = _contents->visible() ? 1 : 0;
    char buf[256];
    // Draw arrow in label
    //    Text to the right of arrow based on parent's label()
    //
    if ( open ) sprintf( buf, "@2>  %s", label() ? label() : "(no label)");
    else        sprintf( buf, "@>  %s",  label() ? label() : "(no label)");
    _button->copy_label( buf );
  }

  // Enforce layout
  void CollapsibleGroup::layout() {

    // Size self based on if open() or close()ed
      int gh = _button->h() + (GROUP_MARGIN*2);
      if ( is_open() ) gh += _contents->h();      // include content's height if we're 'open'

    // Note: resizable() set to zero, so this just resizes us, not children.
    Fl_Group::resize(x(), y(), w(), gh);

    // Manage size/position of children
    //    No need to call init_sizes(): resizable(0) is set in ctor,
    //    and we adjust children ourself.
    //
    _button->resize(x()+GROUP_MARGIN,                   // x always inset (leaves room for ROUND box)
                    y()+GROUP_MARGIN,                   // y always inset ("")
                    w()-(GROUP_MARGIN*2),               // width tracks group's w()
                    _button->h() );                          // height fixed

    // Leave _contents->h() alone, we shouldn't change it; Fl_Pack calculates
    // its own height, we don't want to mess that up by changing it.
    // visible() will control whether it's drawn or not. it's seen or not.
    //
    _contents->resize(x()+GROUP_MARGIN,                 // x always same as button
                      y()+_button->h()+GROUP_MARGIN,        // y always "below button"
                      w()-(GROUP_MARGIN*2),             // width tracks group's w()
                      _contents->h());                  // leave height of _contents alone

    // DEBUG

    // printf("--- LAYOUT CHANGED (%s) ------\n", label() ? label() : "(no label)");
    // printf(" grp: %d,%d,%d,%d\n", x(),y(),w(),h());
    // printf(" but: %d,%d,%d,%d\n", _button->x(), _button->y(), _button->w(), _button->h());
    // printf("pack: %d,%d,%d,%d\n", _contents->x(), _contents->y(), _contents->w(), _contents->h());
    // printf("\n");

  }

  void CollapsibleGroup::toggle_tab_cb(Fl_Button* w, void *data) {
    mrv::CollapsibleGroup* g = (mrv::CollapsibleGroup*) data;
    if ( g->is_open() ) g->close(); else g->open();  // toggle open/close state
  }

  // CTOR
  CollapsibleGroup::CollapsibleGroup( const int x, const int y,
                                      const int w, const int h,
                                      const char* l ) : Fl_Group( x, y, w, h, l ) {

    box( FL_ROUNDED_BOX );   // get fancy later, after we debug. It's harder to see problems with round box. -erco

    // Use a border box for now, so we can see our bounds in parent mrv::Pack.
    //box(FL_BORDER_BOX);

    // Disable label from being shown; we show the label only in the button.
    labeltype(FL_NO_LABEL);

    //NOTNEEDED Fl_Group::begin();
    // Button
    _button = new Fl_Button(x+GROUP_MARGIN,           // margin leaves room for FL_ROUND_BOX
                            y+GROUP_MARGIN,           // margin leaves room for FL_ROUND_BOX
                            w-(GROUP_MARGIN*2),       // width same as group within margin
                            BUTTON_H );                // button height fixed size
    _button->align( FL_ALIGN_LEFT| FL_ALIGN_INSIDE );
    _button->labelsize( 16 );
    _button->box( FL_FLAT_BOX );    // commented out for easier debugging. Revert when needed
    _button->callback( (Fl_Callback*)toggle_tab_cb, this );

    _contents = new Pack(_button->x(),                     // lines up with button on x
                         y+_button->y()+_button->h(),      // just below button
                         w-(GROUP_MARGIN*2),               // width same as group within margin
                         10 );                             // changes when child add()ed

    // end() _contents; we dont' want it to begin() sucking up child widgets on return
    _contents->end();
    Fl_Group::end();

    relabel_button();         // relabel button once pack created
    resizable(0);             // prevent FLTK auto-sizing -- we handle children ourself
  }

  CollapsibleGroup::~CollapsibleGroup() {
    _contents->clear();
    Fl_Group::clear();  // delete _button and _contents
  }

  void CollapsibleGroup::spacing( int x ) {
    _contents->spacing( x );
    redraw();
  }

  void CollapsibleGroup::clear() {
    _contents->clear();
    redraw();
  }

// // DEBUG
// // // // We don't really need this other than for debugging..
  // void CollapsibleGroup::draw() {
  //    fl_push_clip(x(), y(), w(), h());  // enforce clipping
  //    Fl_Group::draw();                  // let group draw itself and children
  //    fl_pop_clip();                     // enforce clipping

  //    fl_color(FL_RED);   fl_rect(x(), y(), w(), h());   // red line around group's xywh
  //    fl_color(FL_GREEN); fl_rect(_contents->x(),        // grn line around pack's xywh
  //                                _contents->y(),
  //                                _contents->w(),
  //                                _contents->h());
  // }

  void CollapsibleGroup::add( Fl_Widget* w ) {
    _contents->add( w );
    _contents->redraw();
  }

  void CollapsibleGroup::resize(int X,int Y,int W,int H) {
    Fl_Group::resize(X,Y,W,H);   // let group resize
    layout();                    // let layout() handle child pos/sizes
  }

  // Open the widget
  void CollapsibleGroup::open() {
      if ( is_open() ) return;   // already open? do nothing
      _contents->show();
      relabel_button();
      layout();                  // layout changed
      window()->redraw();        // force redraw (_contents->hide()/show() doesn't)
  }

  // Close the widget
  void CollapsibleGroup::close() {
      if ( !is_open() ) return;  // already closed? do nothing
      _contents->hide();
      relabel_button();
      layout();                  // layout changed
      window()->redraw();        // force redraw (_contents->hide()/show() doesn't)
  }

} // namespace mrv
