#include "mrvWidgets/mrvDockGroup.h"
#include "mrvWidgets/mrvResizableBar.h"
#include "mrvWidgets/mrvToolGroup.h"

#include "mrvFl/mrvToolWidget.h"

#include "mrvApp/mrvSettingsObject.h"

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
    }

    ToolWidget::~ToolWidget()
    {
        TLRENDER_P();

        save();

        SettingsObject* settingsObject = p.ui->app->settingsObject();
        std::string label = g->label();
        std::string key = "gui/" + label + "/Window/Visible";
        settingsObject->setValue( key, 0 );

        delete g->image(); g->image( nullptr );
        ToolGroup::cb_dismiss( NULL, g );
    }

    void ToolWidget::add_group( const char* lbl )
    {
        TLRENDER_P();

        Fl_Group* dg = p.ui->uiDockGroup;
        ResizableBar* bar = p.ui->uiResizableBar;
        DockGroup* dock = p.ui->uiDock;

        std::string label = lbl;

        SettingsObject* settingsObject = p.ui->app->settingsObject();

        std::string prefix = "gui/" + label;
        std::string key =  prefix + "/Window";
        std_any value = settingsObject->value( key );
        int window = std_any_empty( value ) ? 0 : std_any_cast<int>( value );

        key += "/Visible";
        settingsObject->setValue( key, 1 );

        int X = dock->x();
        int Y = dock->y();
        int W = dg->w()-bar->w();
        int H = dg->h();

        if ( window )
        {
            key = prefix + "/WindowX";
            value = settingsObject->value( key );
            X = std_any_empty( value ) ? X : std_any_cast<int>( value );

            key = prefix + "/WindowY";
            value = settingsObject->value( key );
            Y = std_any_empty( value ) ? Y : std_any_cast<int>( value );

            key = prefix + "/WindowW";
            value = settingsObject->value( key );
            W = std_any_empty( value ) ? W : std_any_cast<int>( value );

            key = prefix + "/WindowH";
            value = settingsObject->value( key );
            H = std_any_empty( value ) ? H : std_any_cast<int>( value );
        }

        g = new ToolGroup(dock, window, X, Y, W, H, lbl );
        g->begin();

        add_controls();

        end_group();
    }

    void ToolWidget::end_group()
    {
        TLRENDER_P();
        g->end();
        p.ui->uiDock->pack->layout();
        p.ui->uiResizableBar->HandleDrag(0);
    }

    void ToolWidget::undock()
    {
        g->undock_grp(g);
    }

    void ToolWidget::dock()
    {
        g->dock_grp(g);
    }

    void ToolWidget::save()
    {
        TLRENDER_P();

        SettingsObject* settingsObject = p.ui->app->settingsObject();

        std::string label = g->label();
        std::string prefix = "gui/" + label;
        std::string key = prefix + "/Window";
        int window = !g->docked();
        settingsObject->setValue( key, window );

        if ( window )
        {
            ToolWindow* w = g->get_window();

            key = prefix + "/WindowX";
            settingsObject->setValue( key, w->x() );

            key = prefix + "/WindowY";
            settingsObject->setValue( key, w->y() );

            key = prefix + "/WindowW";
            settingsObject->setValue( key, w->w() );

            key = prefix + "/WindowH";
            settingsObject->setValue( key, w->h() );
        }
    }

}
