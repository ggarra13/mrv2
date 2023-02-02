// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>

#include "mrvToolsCallbacks.h"
#include "mrViewer.h"

namespace mrv {

    ColorTool*             colorPanel = nullptr;
    FilesPanel*             filesPanel = nullptr;
    ComparePanel*         comparePanel = nullptr;
    PlaylistPanel*       playlistPanel = nullptr;
    SettingsPanel*       settingsPanel = nullptr;
    EnvironmentMapPanel* environmentMapPanel = nullptr;
    LogsPanel*               logsPanel = nullptr;
    DevicesPanel*         devicesPanel = nullptr;
    ColorAreaPanel*     colorAreaPanel = nullptr;
    AnnotationsPanel* annotationsPanel = nullptr;
    ImageInfoPanel*     imageInfoPanel = nullptr;
    HistogramPanel*     histogramPanel = nullptr;
    VectorscopePanel* vectorscopePanel = nullptr;

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
        if ( colorPanel && colorPanel->is_panel() )
            color_panel_cb( nullptr, nullptr );
        if ( filesPanel && filesPanel->is_panel() )
            files_panel_cb( nullptr, nullptr );
        if ( comparePanel && comparePanel->is_panel() )
            compare_panel_cb( nullptr, nullptr );
        if ( playlistPanel && playlistPanel->is_panel() )
            playlist_panel_cb( nullptr, nullptr );
        if ( settingsPanel && settingsPanel->is_panel() )
            settings_panel_cb( nullptr, nullptr );
        if ( logsPanel && logsPanel->is_panel() )
            logs_panel_cb( nullptr, nullptr );
        if ( devicesPanel && devicesPanel->is_panel() )
            devices_panel_cb( nullptr, nullptr );
        if ( colorAreaPanel && colorAreaPanel->is_panel() )
            color_area_panel_cb( nullptr, nullptr );
        if ( annotationsPanel && annotationsPanel->is_panel() )
            annotations_panel_cb( nullptr, nullptr );
        if ( imageInfoPanel && imageInfoPanel->is_panel() )
            image_info_panel_cb( nullptr, nullptr );
        if ( histogramPanel && histogramPanel->is_panel() )
            histogram_panel_cb( nullptr, nullptr );
        if ( vectorscopePanel && vectorscopePanel->is_panel() )
            vectorscope_panel_cb( nullptr, nullptr );
    }

    void color_panel_cb( Fl_Widget* w, ViewerUI* ui )
    {
        if ( colorPanel )
        {
            delete colorPanel; colorPanel = nullptr;
            return;
        }
        colorPanel = new ColorPanel( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }

    void files_panel_cb( Fl_Widget* w, ViewerUI* ui )
    {
        if ( filesPanel )
        {
            delete filesPanel; filesPanel = nullptr;
            return;
        }
        filesPanel = new FilesPanel( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }

    void compare_panel_cb( Fl_Widget* w, ViewerUI* ui )
    {
        if ( comparePanel )
        {
            delete comparePanel; comparePanel = nullptr;
            return;
        }
        comparePanel = new ComparePanel( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }

    void playlist_panel_cb( Fl_Widget* w, ViewerUI* ui )
    {
        if ( playlistPanel )
        {
            delete playlistPanel; playlistPanel = nullptr;
            return;
        }
        playlistPanel = new PlaylistPanel( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }

    void settings_panel_cb( Fl_Widget* w, ViewerUI* ui )
    {
        if ( settingsPanel )
        {
            delete settingsPanel; settingsPanel = nullptr;
            return;
        }
        settingsPanel = new SettingsPanel( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }

    void logs_panel_cb( Fl_Widget* w, ViewerUI* ui )
    {
        if ( logsPanel )
        {
            delete logsPanel; logsPanel = nullptr;
            return;
        }
        logsPanel = new LogsPanel( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }

    void devices_panel_cb( Fl_Widget* w, ViewerUI* ui )
    {
        if ( devicesPanel )
        {
            delete devicesPanel; devicesPanel = nullptr;
            return;
        }
        devicesPanel = new DevicesPanel( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }

    void color_area_panel_cb( Fl_Widget* w, ViewerUI* ui )
    {
        if ( colorAreaPanel )
        {
            delete colorAreaPanel; colorAreaPanel = nullptr;
            return;
        }
        colorAreaPanel = new ColorAreaPanel( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }

    void annotations_panel_cb( Fl_Widget* w, ViewerUI* ui )
    {
        if ( annotationsPanel )
        {
            delete annotationsPanel; annotationsPanel = nullptr;
            return;
        }
        annotationsPanel = new AnnotationsPanel( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }

    void image_info_panel_cb( Fl_Widget* w, ViewerUI* ui )
    {
        if ( imageInfoPanel )
        {
            delete imageInfoPanel; imageInfoPanel = nullptr;
            return;
        }
        imageInfoPanel = new ImageInfoPanel( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
        const auto player = ui->uiView->getTimelinePlayer();
        imageInfoPanel->setTimelinePlayer( player );
        imageInfoPanel->refresh();
    }

    void histogram_panel_cb( Fl_Widget* w, ViewerUI* ui )
    {
        if ( histogramPanel )
        {
            delete histogramPanel; histogramPanel = nullptr;
            return;
        }
        histogramPanel = new HistogramPanel( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }

    void vectorscope_panel_cb( Fl_Widget* w, ViewerUI* ui )
    {
        if ( vectorscopePanel )
        {
            delete vectorscopePanel; vectorscopePanel = nullptr;
            return;
        }
        vectorscopePanel = new VectorscopePanel( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }

    void environment_map_panel_cb( Fl_Widget* w, ViewerUI* ui )
    {
        if ( environmentMapPanel )
        {
            delete environmentMapPanel; environmentMapPanel = nullptr;
            return;
        }
        environmentMapPanel = new EnvironmentMapPanel( ui );
        ui->uiMain->fill_menu( ui->uiMenuBar );
    }
}
