// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <memory>
#include <vector>

#include <tlCore/Util.h>

// @todo: these should be listed alphabetically but lead to include issues on
//        windows.
#include "mrvPanels/mrvFilesPanel.h"
#include "mrvPanels/mrvColorPanel.h"
#include "mrvPanels/mrvComparePanel.h"
#include "mrvPanels/mrvPlaylistPanel.h"
#include "mrvPanels/mrvSettingsPanel.h"
#include "mrvPanels/mrvLogsPanel.h"
#include "mrvPanels/mrvDevicesPanel.h"
#include "mrvPanels/mrvColorAreaPanel.h"
#include "mrvPanels/mrvAnnotationsPanel.h"
#include "mrvPanels/mrvImageInfoPanel.h"
#include "mrvPanels/mrvHistogramPanel.h"
#include "mrvPanels/mrvVectorscopePanel.h"
#include "mrvPanels/mrvEnvironmentMapPanel.h"
#include "mrvPanels/mrvStereo3DPanel.h"
#include "mrvPanels/mrvPythonPanel.h"
#include "mrvPanels/mrvNetworkPanel.h"
#include "mrvPanels/mrvUSDPanel.h"

class ViewerUI;
class Fl_Widget;

namespace mrv
{

    extern ColorPanel* colorPanel;
    extern FilesPanel* filesPanel;
    extern ComparePanel* comparePanel;
    extern PlaylistPanel* playlistPanel;
    extern SettingsPanel* settingsPanel;
    extern LogsPanel* logsPanel;
    extern DevicesPanel* devicesPanel;
    extern ColorAreaPanel* colorAreaPanel;
    extern AnnotationsPanel* annotationsPanel;
    extern ImageInfoPanel* imageInfoPanel;
    extern HistogramPanel* histogramPanel;
    extern VectorscopePanel* vectorscopePanel;
    extern EnvironmentMapPanel* environmentMapPanel;
    extern Stereo3DPanel* stereo3DPanel;
    extern PythonPanel* pythonPanel;
#ifdef MRV2_NETWORK
    extern NetworkPanel* networkPanel;
#endif
#ifdef TLRENDER_USD
    extern USDPanel* usdPanel;
#endif

    void onePanelOnly(bool t);
    bool onePanelOnly();

    void removePanels(ViewerUI* ui);
    void removeWindows(ViewerUI* ui);

    void redrawPanelThumbnails();
    void refreshPanelThumbnails();

    void syncPanels();

    void annotations_panel_cb(Fl_Widget* w, ViewerUI* ui);
    void color_area_panel_cb(Fl_Widget* w, ViewerUI* ui);
    void color_panel_cb(Fl_Widget* w, ViewerUI* ui);
    void compare_panel_cb(Fl_Widget* w, ViewerUI* ui);
    void devices_panel_cb(Fl_Widget* w, ViewerUI* ui);
    void environment_map_panel_cb(Fl_Widget* w, ViewerUI* ui);
    void files_panel_cb(Fl_Widget* w, ViewerUI* ui);
    void histogram_panel_cb(Fl_Widget* w, ViewerUI* ui);
    void image_info_panel_cb(Fl_Widget* w, ViewerUI* ui);
    void logs_panel_cb(Fl_Widget* w, ViewerUI* ui);
    void network_panel_cb(Fl_Widget* w, ViewerUI* ui);
    void python_panel_cb(Fl_Widget* w, ViewerUI* ui);
    void playlist_panel_cb(Fl_Widget* w, ViewerUI* ui);
    void settings_panel_cb(Fl_Widget* w, ViewerUI* ui);
    void usd_panel_cb(Fl_Widget* w, ViewerUI* ui);
    void vectorscope_panel_cb(Fl_Widget* w, ViewerUI* ui);
    void stereo3D_panel_cb(Fl_Widget* w, ViewerUI* ui);

} // namespace mrv
