#include <iostream>

#include "mrvToolsCallbacks.h"
#include "mrViewer.h"

namespace mrv {

    ColorTool*             colorTool = nullptr;
    FilesTool*             filesTool = nullptr;
    CompareTool*         compareTool = nullptr;
    SettingsTool*       settingsTool = nullptr;
    LogsTool*               logsTool = nullptr;
    DevicesTool*         devicesTool = nullptr;
    ColorAreaTool*     colorAreaTool = nullptr;
    AnnotationsTool* annotationsTool = nullptr;
    ImageInfoTool*     imageInfoTool = nullptr;
    HistogramTool*     histogramTool = nullptr;
    VectorscopeTool* vectorscopeTool = nullptr;
    
    void color_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( colorTool )
        {
            delete colorTool; colorTool = nullptr;
            return;
        }
        colorTool = new ColorTool( ui );
    }

    void files_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( filesTool )
        {
            delete filesTool; filesTool = nullptr;
            return;
        }
        filesTool = new FilesTool( ui );
    }
    
    void compare_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( compareTool )
        {
            delete compareTool; compareTool = nullptr;
            return;
        }
        compareTool = new CompareTool( ui );
    }
    
    void settings_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( settingsTool )
        {
            delete settingsTool; settingsTool = nullptr;
            return;
        }
        settingsTool = new SettingsTool( ui );
    }
    
    void logs_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( logsTool )
        {
            delete logsTool; logsTool = nullptr;
            return;
        }
        logsTool = new LogsTool( ui );
    }
    
    void devices_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( devicesTool )
        {
            delete devicesTool; devicesTool = nullptr;
            return;
        }
        devicesTool = new DevicesTool( ui );
    }
    
    void color_area_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( colorAreaTool )
        {
            delete colorAreaTool; colorAreaTool = nullptr;
            return;
        }
        colorAreaTool = new ColorAreaTool( ui );
    }
    
    void annotations_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( annotationsTool )
        {
            delete annotationsTool; annotationsTool = nullptr;
            return;
        }
        annotationsTool = new AnnotationsTool( ui );
    }
    
    void image_info_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( imageInfoTool )
        {
            delete imageInfoTool; imageInfoTool = nullptr;
            return;
        }
        imageInfoTool = new ImageInfoTool( ui );
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
    }
    
    void vectorscope_tool_grp( Fl_Widget* w, ViewerUI* ui )
    {
        if ( vectorscopeTool )
        {
            delete vectorscopeTool; vectorscopeTool = nullptr;
            return;
        }
        vectorscopeTool = new VectorscopeTool( ui );
    }
}
