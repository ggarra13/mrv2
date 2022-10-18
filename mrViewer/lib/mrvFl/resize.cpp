//
// Please see the following page on how to report bugs and issues:
//
//     https://www.fltk.org/bugs.php
//
#include <iostream>

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Hor_Slider.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Flex.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>
#include <FL/math.h>

#include <FL/gl.h>
#include <FL/Fl_Gl_Window.H>

#include "mrvResizableBar.h"
#include "mrvToolGroup.h"
#include "mrvDropWindow.h"

#include "mrvEventHeader.h"
#define TB_WIDTH  DROP_REGION_WIDTH*2

#include "mrvCollapsibleGroup.h"
#include "mrvHorSlider.h"

using namespace mrv;


static int tb_idx = 0;  // counts how many toolbars we have made
static char labels[32]; // Generic label strings...

static DropWindow *win_main;    // The main window
static DockGroup *dock;      // the toolbar dock

static void cb_Exit(Fl_Button*, void*)
{
        win_main->hide(); // hide the main window
        // do we have toolbars undocked? If so, find them and hide them...
        ToolGroup::hide_all();
}


static void cb_Check(Fl_Button*, void*)
{
}

static void cb_generic(Fl_Button*, void* v)
{
    int* i  = (int*)v;
    int idx = intptr_t(i);
    printf("ToolGroup: %d\n", idx);
    fflush(stdout);
}

static void add_docked(Fl_Button*, void*)
{
        // Create a docked toolgroup
    ToolGroup *tgroup = new ToolGroup(dock, 0, dock->w(), 160);
    tgroup->begin();
    mrv::CollapsibleGroup* g = new mrv::CollapsibleGroup( tgroup->x(), 20, tgroup->w(), 120, "Video" );
    g->button()->labelsize( 14 );
    g->begin();
    mrv::HorSlider* s = new mrv::HorSlider( g->x(), g->y()+40,
                                            g->w(), 20, "Gain" );
    s->setRange( 0.f, 10.0f );
    s->setStep( 0.1f );
    s->setDefaultValue( 1.0f );
    s = new mrv::HorSlider( g->x(), g->y()+40, g->w(), 20, "Brightness" );
    s->setRange( 0.f, 10.0f );
    s->setStep( 0.1f );
    s->setDefaultValue( 1.0f );
    g->end();
    tgroup->end();
    tgroup->box(FL_BORDER_BOX);
    dock->redraw();
}

static void add_floater(Fl_Button*, void*)
{
        // create a floating toolbar window
        ToolGroup *tool_group = new ToolGroup(dock, 1, 140, 155, TB_WIDTH, 80);
          Fl_Button* dm2 = new Fl_Button(3, 21, TB_WIDTH-3, 20);
          tb_idx++;
          sprintf(labels, "test %d", tb_idx);
          dm2->copy_label(labels);
          dm2->box(FL_THIN_UP_BOX);
          dm2->callback((Fl_Callback*)cb_generic, (void *)tb_idx);
          dm2 = new Fl_Button(3, 41, TB_WIDTH-3, 20);
          dm2->copy_label(labels);
          dm2->box(FL_THIN_UP_BOX);
          dm2->callback((Fl_Callback*)cb_generic, (void *)tb_idx);
          dm2 = new Fl_Button(3, 61, TB_WIDTH-3, 20);
          dm2->copy_label(labels);
          dm2->box(FL_THIN_UP_BOX);
          dm2->callback((Fl_Callback*)cb_generic, (void *)tb_idx);
        tool_group->end();
        tool_group->box(FL_THIN_UP_BOX);
}


class shape_window : public Fl_Gl_Window {
  void draw();
public:
  int sides;
  shape_window(int x,int y,int w,int h,const char *l=0);
    void resize( int X,int Y,int W,int H )
        {
            Fl_Gl_Window::resize( X, Y, W, H );
        }
};

shape_window::shape_window(int x,int y,int w,int h,const char *l) :
Fl_Gl_Window(x,y,w,h,l) {
  sides = 3;
}

void shape_window::draw() {
  if (!valid()) {
    valid(1);
    glLoadIdentity();
    glViewport(0,0,pixel_w(),pixel_h());
  }
// draw an amazing but slow graphic:
  glClear(GL_COLOR_BUFFER_BIT);
  glColor3f( 0.5, 0.5, 0.75 );

  glBegin(GL_POLYGON);
  for (int j=0; j<sides; j++) {
      double ang = j*2*M_PI/sides;
      glVertex3f((GLfloat)cos(ang), (GLfloat)sin(ang), 0);
  }
  glEnd();
}

// when you change the data, as in this callback, you must call redraw():
void hide_cb(Fl_Widget *o, void *p) {
  Fl_Group *w = (Fl_Group*)p;
  if ( w->visible() ) {
      w->hide();
  }
  else {
      w->show();
  }
  Fl_Flex* flex = (Fl_Flex*) w->parent();
  flex->layout();
}

int main(int argc, char **argv) {

  Fl::use_high_res_GL(1);
  
  win_main = new DropWindow(640, 480, "Main Window");
  win_main->begin();

  Fl_Flex* flex = nullptr;
  win_main->workspace = flex = new Fl_Flex( 0, 70, 640, 410 );

  flex->type( Fl_Flex::HORIZONTAL );
  flex->color( FL_CYAN );
  flex->begin();

  int H = flex->h();
  int W = flex->w()/2;
  int dockGrpW = flex->w() - W;

  shape_window sw(0, 75, W, H );
  sw.end();

  Fl_Group     dockGrp( W+10, 75, dockGrpW, H );
  flex->set_size( dockGrp, dockGrpW );

  dockGrp.begin();

  ResizableBar bar( dockGrp.x(), dockGrp.y(), 10, dockGrp.h() );


  int controlsX =  bar.x() + bar.w();
  int controlsW =    dockGrpW - bar.w();


  dock = new DockGroup(controlsX, dockGrp.y(),
                       controlsW, dockGrp.h() );
  dock->box(FL_THIN_DOWN_BOX);
  dock->end();
  dock->set_window(win_main);
  win_main->set_dock( dock );
  
  add_docked(0, 0);


  dockGrp.end();


  flex->resizable(&sw);
  flex->end();



  Fl_Button add_button(30, 35, W, 30, "Add");
  add_button.callback((Fl_Callback*)add_docked, NULL);
  
  Fl_Button hide_button(30, 5, W, 30, "Hide");
  hide_button.callback(hide_cb, &dockGrp);

  win_main->resizable(flex);

  win_main->end();
  win_main->callback((Fl_Callback*)cb_Exit);
  
  win_main->show(argc,argv);

  // it still works, but the modality may be awry...
  add_floater(0, 0);

  // Now expose all the non-docked tool windows - not really necessary
  ToolGroup::show_all();


  return Fl::run();
}
