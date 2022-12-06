// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <memory>
#include <vector>

#include <tlCore/Util.h>

#include "mrvFilesTool.h"
#include "mrvColorTool.h"
#include "mrvCompareTool.h"
#include "mrvSettingsTool.h"
#include "mrvLogsTool.h"
#include "mrvDevicesTool.h"
#include "mrvColorAreaTool.h"
#include "mrvAnnotationsTool.h"
#include "mrvImageInfoTool.h"
#include "mrvHistogramTool.h"
#include "mrvVectorscopeTool.h"

class ViewerUI;
class Fl_Widget;

namespace mrv
{
    
    extern ColorTool*           colorTool;
    extern FilesTool*           filesTool;
    extern CompareTool*       compareTool;
    extern SettingsTool*     settingsTool;
    extern LogsTool*             logsTool;
    extern DevicesTool*       devicesTool;
    extern ColorAreaTool*   colorAreaTool;
    extern AnnotationsTool* annotationsTool;
    extern ImageInfoTool*   imageInfoTool;
    extern HistogramTool*   histogramTool;
    extern VectorscopeTool* vectorscopeTool;
    
    void color_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void files_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void compare_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void settings_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void logs_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void devices_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void color_area_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void annotations_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void image_info_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void histogram_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void vectorscope_tool_grp( Fl_Widget* w, ViewerUI* ui );
    
} // namespace mrv
