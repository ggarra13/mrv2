// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <memory>
#include <vector>

#include <tlCore/Util.h>

#include "mrvAnnotationsTool.h"
#include "mrvFilesTool.h"
#include "mrvColorAreaTool.h"
#include "mrvColorTool.h"
#include "mrvCompareTool.h"
#include "mrvDevicesTool.h"
#include "mrvHistogramTool.h"
#include "mrvImageInfoTool.h"
#include "mrvLatLongTool.h"
#include "mrvLogsTool.h"
#include "mrvPlaylistTool.h"
#include "mrvSettingsTool.h"
#include "mrvVectorscopeTool.h"

class ViewerUI;
class Fl_Widget;

namespace mrv
{
    
    extern AnnotationsTool* annotationsTool;
    extern ColorTool*           colorTool;
    extern ColorAreaTool*   colorAreaTool;
    extern CompareTool*       compareTool;
    extern DevicesTool*       devicesTool;
    extern FilesTool*           filesTool;
    extern HistogramTool*   histogramTool;
    extern ImageInfoTool*   imageInfoTool;
    extern LatLongTool*       latLongTool;
    extern LogsTool*             logsTool;
    extern PlaylistTool*     playlistTool;
    extern SettingsTool*     settingsTool;
    extern VectorscopeTool* vectorscopeTool;
    
    void onePanelOnly(bool t);
    bool onePanelOnly();

    void removePanels();
    
    void annotations_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void color_area_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void color_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void compare_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void devices_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void histogram_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void image_info_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void files_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void lat_long_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void logs_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void playlist_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void settings_tool_grp( Fl_Widget* w, ViewerUI* ui );
    void vectorscope_tool_grp( Fl_Widget* w, ViewerUI* ui );

    
} // namespace mrv
