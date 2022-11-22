#pragma once

#include <iostream>

/* fltk includes */
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Scroll.H>

#include "mrvPack.h"
#include "mrvDockGroup.h"
#include "mrvDragButton.h"
#include "mrvToolWindow.h"

namespace mrv
{

    class ToolGroup : public Fl_Group
    {
    private:
	// control variables
	short _docked;
	DockGroup *dock;

	// constructor helper function
	void create_dockable_group(const char* lbl);
	void create_docked(DockGroup *d, const char* lbl);
	void create_floating(DockGroup *d, int state, int x, int y, int w, int h, const char *l);

    protected:
	// Widgets used by the toolbar
	Fl_Button *dismiss;
	DragButton *dragger;
	Fl_Button *docker;
        ToolWindow* tw;
        Fl_Scroll*  scroll;
        Pack *pack;

	// Sets whether window is docked or not.
	void docked(short r);
	
	// Defines which dock the group can dock into
	void set_dock(DockGroup *w) {dock = w;}
	// get the dock group ID
	DockGroup *get_dock(void) {return dock;}


    public:
	// Constructors for docked/floating window
	ToolGroup(DockGroup *d, int f, int w, int h, const char *l = 0);
	ToolGroup(DockGroup *d, int f, int x, int y, int w, int h, const char *l = 0);

        // Get the toolwindow or null if docked
        ToolWindow* get_window()  {return tw; }
        Pack*       get_pack()    {return pack; }
        Fl_Scroll*  get_scroll()  {return scroll; }

        Fl_Image* image() const     { return dragger->image(); }
        void image( Fl_Image* img ) { dragger->image( img ); }
        
	// methods for hiding/showing *all* the floating windows
	static void show_all(void);
	static void hide_all(void);

	// Tests whether window is docked or not.
	short docked() { return _docked; }

	// generic callback function for the dock/undock checkbox
	void dock_grp(void* v);
	void undock_grp(void* v);

	// generic callback function for the dismiss button
	static void cb_dismiss(Fl_Button*, void* v);
        
        inline void callback( Fl_Callback* c, void* d )
            {
                dismiss->callback( c, d );
            }

        inline const char* label() const { return dragger->label(); }
	// wrap some basic Fl_Group functions to access the enclosed pack
        inline void clear() {pack->clear(); }
	inline void begin() {pack->begin(); }
	void end();
        void resize( int X, int Y, int W, int H );
	inline void resizable(Fl_Widget *box) {pack->resizable(box); }
	inline void resizable(Fl_Widget &box) {pack->resizable(box); }
	inline Fl_Widget *resizable() const { return pack->resizable(); }
	inline void add( Fl_Widget *w ) {
            pack->add( w );
        }
	inline void add( Fl_Widget &w ) { add( &w ); }
	inline void insert( Fl_Widget &w, int n ) { pack->insert( w, n ); }
	inline void insert( Fl_Widget &w, Fl_Widget* beforethis ) { pack->insert( w, beforethis ); }
	inline void remove( Fl_Widget &w ) { pack->remove( w ); }
	inline void remove( Fl_Widget *w ) { pack->remove( w ); }
        inline int children() const { return pack->children(); } 
    };

}
