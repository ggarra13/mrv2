// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvPanelWidget.h"

class ViewerUI;
class Fl_Check_Button;

namespace mrv
{

    class ColorPanel : public PanelWidget
    {
    public:
        ColorPanel(ViewerUI* ui);
        ~ColorPanel();

        void refresh() noexcept;

        void add_controls() override;

    private:
        struct Private;
        std::unique_ptr<Private> _r;
    };

} // namespace mrv
