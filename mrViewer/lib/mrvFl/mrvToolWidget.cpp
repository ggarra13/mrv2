#include <errno.h>   // errno
#include <string.h>  // strerror
#include <FL/fl_message.H>
#include <FL/Fl.H>

#include "mrvWidgets/mrvDockGroup.h"
#include "mrvWidgets/mrvResizableBar.h"
#include "mrvWidgets/mrvToolGroup.h"

#include "mrvFl/mrvToolWidget.h"

#include "mrvPlayApp/mrvSettingsObject.h"

#include "mrViewer.h"

#include "mrvFl/mrvIO.h"

namespace {
  const char* kModule = "toolwidget";
}

namespace mrv
{
    
    
    ToolWidget::ToolWidget( ViewerUI* ui ) :
        _p( new Private )
    {
        _p->ui = ui;
        svg_root = fl_getenv("MRV_ROOT");
        svg_root += "/icons/";
    }

    ToolWidget::~ToolWidget()
    {
        save();
        delete g->image(); g->image( nullptr );
        ToolGroup::cb_dismiss( NULL, g );
    }
  
    Fl_SVG_Image* ToolWidget::load_svg( const std::string& svg_name )
    {
      std::string file = svg_root + svg_name;
      LOG_INFO( "Load SVG " << file );
      Fl::check();
      Fl_SVG_Image* svg = new Fl_SVG_Image( file.c_str() );
      LOG_INFO( "Loaded SVG " << file );
      if ( !svg ) return nullptr;
      
      switch (svg->fail()) {
      case Fl_Image::ERR_FILE_ACCESS:
	// File couldn't load? show path + os error to user
	LOG_ERROR( file << ": " << strerror(errno) );
	//fl_alert("%s: %s", file.c_str(), strerror(errno));
	return nullptr;
      case Fl_Image::ERR_FORMAT:
	// Parsing error
	//fl_alert("%s: couldn't decode image", file.c_str());
	LOG_ERROR( file << ": couldn't decode image" );
	return nullptr;
      }
      return svg;
    }

    void ToolWidget::add_group( const char* lbl )
    {
        TLRENDER_P();
        
        Fl_Group* dg = p.ui->uiDockGroup;
        ResizableBar* bar = p.ui->uiResizableBar;
        DockGroup* dock = p.ui->uiDock;

        std::string label = lbl;

        SettingsObject* settings = p.ui->app->settingsObject();

        std::string key = "gui/" + label + "/Window";
        std_any value = settings->value( key );
        int window = value.empty() ? 0 : std_any_cast<int>( value );

        int X = dock->x();
        int Y = dock->y();
        int W = dg->w()-bar->w();
        int H = dg->h();

        if ( window )
        {
            key = "gui/" + label + "/WindowX";
            value = settings->value( key );
            X = value.empty() ? X : std_any_cast<int>( value );
            
            key = "gui/" + label + "/WindowY";
            value = settings->value( key );
            Y = value.empty() ? Y : std_any_cast<int>( value );
            
            key = "gui/" + label + "/WindowW";
            value = settings->value( key );
            W = value.empty() ? W : std_any_cast<int>( value );
            
            key = "gui/" + label + "/WindowH";
            value = settings->value( key );
            H = value.empty() ? H : std_any_cast<int>( value );
        }
        
        g = new ToolGroup(dock, window, X, Y, W, H, lbl );
        g->begin();

        add_controls();
        
        g->end();
        g->box( FL_FLAT_BOX );

        end_group();
    }

    void ToolWidget::end_group()
    {
        TLRENDER_P();
        p.ui->uiDock->pack->layout();
        p.ui->uiResizableBar->HandleDrag(0);
    }

    void ToolWidget::save()
    {
        TLRENDER_P();
        
        SettingsObject* settings = p.ui->app->settingsObject();

        std::string label = g->label();
        std::string key = "gui/" + label + "/Window";
        int window = !g->docked();
        settings->setValue( key, window );
        
        if ( window )
        {
            ToolWindow* w = g->get_window();
            
            key = "gui/" + label + "/WindowX";
            settings->setValue( key, w->x() );
            
            key = "gui/" + label + "/WindowY";
            settings->setValue( key, w->y() );
            
            key = "gui/" + label + "/WindowW";
            settings->setValue( key, w->w() );
            
            key = "gui/" + label + "/WindowH";
            settings->setValue( key, w->h() );
        }
    }

}
