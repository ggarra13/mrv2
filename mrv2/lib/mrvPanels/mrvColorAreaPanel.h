// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvFl/mrvColorAreaInfo.h"
#include "mrvPanelWidget.h"

class ViewerUI;

namespace mrv {
namespace area {
class Info;
}

class ColorAreaPanel : public PanelWidget {
public:
  ColorAreaPanel(ViewerUI *ui);
  ~ColorAreaPanel();

  void add_controls() override;

  void update(const area::Info &info);

private:
  struct Private;
  std::unique_ptr<Private> _r;
};

} // namespace mrv
