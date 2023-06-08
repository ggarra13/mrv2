// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

#include "mrvCore/mrvI8N.h"

class ViewerUI;

namespace mrv
{
    inline bool isPanelWithHeight(const std::string& label)
    {
        if (label != _("Files") && label != _("Compare") &&
            label != _("Playlist") && label != _("Network") &&
            label != _("Stereo 3D"))
            return true;
        return false;
    }

    std::string getLayerName(uint16_t layerId, ViewerUI* ui);

} // namespace mrv
