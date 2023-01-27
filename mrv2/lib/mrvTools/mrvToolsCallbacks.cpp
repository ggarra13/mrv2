// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>

#include "mrvToolsCallbacks.h"
#include "mrViewer.h"

namespace mrv {

    ColorTool*             colorTool = nullptr;
    FilesTool*             filesTool = nullptr;
    CompareTool*         compareTool = nullptr;
    PlaylistTool*       playlistTool = nullptr;
    SettingsTool*       settingsTool = nullptr;
    LatLongTool*         latLongTool = nullptr;
    LogsTool*               logsTool = nullptr;
    DevicesTool*         devicesTool = nullptr;
    ColorAreaTool*     colorAreaTool = nullptr;
    AnnotationsTool* annotationsTool = nullptr;
    ImageInfoTool*     imageInfoTool = nullptr;
    HistogramTool*     histogramTool = nullptr;
    VectorscopeTool* vectorscopeTool = nullptr;

    bool one_panel_only = false;
    
    void onePanelOnly(bool t)
    {
        one_panel_only = t;
    }
    
    bool onePanelOnly()
    {
        return one_panel_only;
    }


    void removePanels()
    {
        if ( colorTool && colorTool->is_panel() )
            color_tool_grp( nullptr, nullptr );
        if ( filesTool && filesTool->is_panel() )
            files_tool_grp( nullptr, nullptr );
        if ( compareTool && compareTool->is_panel() )
            compare_tool_grp( nullptr, nullptr );
        if ( playlistTool && playlistTool->is_panel() )
            playlist_tool_grp( nullptr, nullptr );
        if ( settingsTool && settingsTool->is_panel() )
            settings_tool_grp( nullptr, nullptr );
        if ( logsTool && logsTool->is_panel() )
            logs_tool_grp( nullptr, nullptr );
        if ( devicesTool && devicesTool->is_panel() )
            devices_tool_grp( nullptr, nullptr );
        if ( colorAreaTool && colorAreaTool->is_panel() )
            color_area_tool_grp( nullptr, nullptr );
        if ( annotationsTool && annotationsTool->is_panel() )
            annotations_tool_grp( nullptr, nullptr );
        if ( imageInfoTool && imageInfoTool->is_panel() )
            image_info_tool_grp( nullptr, nullptr );
        if ( histogramTool && histogramTool->is_panel() )
            histogram_tool_grp( nullptr, nullptr );
        if ( vectorscopeTool && vectorscopeTool->is_panel() )
            vectorscope_tool_grp( nullptr, nullptr );
    }
    
    void color_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( colorTool )
        {
            delete colorTool; colorTool = nullptr;
            return;
        }
        colorTool = new ColorTool( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }

    void files_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( filesTool )
        {
            delete filesTool; filesTool = nullptr;
            return;
        }
        filesTool = new FilesTool( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }
    
    void compare_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( compareTool )
        {
            delete compareTool; compareTool = nullptr;
            return;
        }
        compareTool = new CompareTool( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }
    
    void playlist_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( playlistTool )
        {
            delete playlistTool; playlistTool = nullptr;
            return;
        }
        playlistTool = new PlaylistTool( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }
    
    void settings_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( settingsTool )
        {
            delete settingsTool; settingsTool = nullptr;
            return;
        }
        settingsTool = new SettingsTool( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }
    
    void logs_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( logsTool )
        {
            delete logsTool; logsTool = nullptr;
            return;
        }
        logsTool = new LogsTool( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }
    
    void devices_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( devicesTool )
        {
            delete devicesTool; devicesTool = nullptr;
            return;
        }
        devicesTool = new DevicesTool( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }
    
    void color_area_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( colorAreaTool )
        {
            delete colorAreaTool; colorAreaTool = nullptr;
            return;
        }
        colorAreaTool = new ColorAreaTool( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }
    
    void annotations_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( annotationsTool )
        {
            delete annotationsTool; annotationsTool = nullptr;
            return;
        }
        annotationsTool = new AnnotationsTool( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }
    
    void image_info_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( imageInfoTool )
        {
            delete imageInfoTool; imageInfoTool = nullptr;
            return;
        }
        imageInfoTool = new ImageInfoTool( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
        const auto player = ui->uiView->getTimelinePlayer();
        imageInfoTool->setTimelinePlayer( player );
        imageInfoTool->refresh();
    }
    
    void histogram_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( histogramTool )
        {
            delete histogramTool; histogramTool = nullptr;
            return;
        }
        histogramTool = new HistogramTool( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }
    
    void vectorscope_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( vectorscopeTool )
        {
            delete vectorscopeTool; vectorscopeTool = nullptr;
            return;
        }
        vectorscopeTool = new VectorscopeTool( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }
    
    void lat_long_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( latLongTool )
        {
            delete latLongTool; latLongTool = nullptr;
            return;
        }
        latLongTool = new LatLongTool( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }
}
