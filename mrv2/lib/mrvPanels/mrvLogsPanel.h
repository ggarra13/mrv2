// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include "mrvPanelWidget.h"

class ViewerUI;

namespace mrv
{
    class LogsPanel : public PanelWidget
    {
    public:
        LogsPanel(ViewerUI* ui);
        ~LogsPanel();

        void add_controls() override;
        void dock() override;
        void undock() override;

        void info(const std::string& msg) const;
        void warning(const std::string& msg) const;
        void error(const std::string& msg) const;

    private:
        struct Private;
        std::unique_ptr<Private> _r;
    };

} // namespace mrv
