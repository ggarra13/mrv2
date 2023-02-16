// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <vector>

#include "mrvPanelWidget.h"

class ViewerUI;
class Fl_Input;
class Fl_Check_Button;
class Fl_Widget;

namespace mrv {

class ColorPanel : public PanelWidget {
  Fl_Check_Button *colorOn;
  Fl_Check_Button *levelsOn;
  Fl_Check_Button *softClipOn;
  Fl_Input *lutFilename;
  std::vector<Fl_Widget *> lutWidgets;
  std::vector<Fl_Widget *> colorWidgets;
  std::vector<Fl_Widget *> levelsWidgets;
  std::vector<Fl_Widget *> softClipWidgets;

public:
  ColorPanel(ViewerUI *ui);
  void refresh() noexcept;

  virtual void add_controls() override;
};

} // namespace mrv
