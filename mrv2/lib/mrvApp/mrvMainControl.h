// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#pragma once

#include "mrvFl/mrvTimelinePlayer.h"

class ViewerUI;

namespace mrv
{
    //! Main window.
    class MainControl
    {

    public:
        MainControl(ViewerUI*);

        //! Set the timeline players.
        void setTimelinePlayers(const std::vector<mrv::TimelinePlayer*>&);

        void setLUTOptions(const timeline::LUTOptions& value);
        void setDisplayOptions(const timeline::DisplayOptions& value);
        void setImageOptions(const timeline::ImageOptions& value);

    private:
        void _widgetUpdate();

        TLRENDER_PRIVATE();
    };
} // namespace mrv
