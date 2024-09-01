// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

#include "mrvCore/mrvI8N.h"

#include "mrvApp/mrvFilesModel.h"

namespace mrv
{
    namespace panel
    {
        inline bool isPanelWithHeight(const std::string& label)
        {
            if (label != _("Files") && label != _("Compare") &&
                label != _("Playlist") && label != _("Network") &&
                label != _("Stereo 3D"))
                return true;
            return false;
        }

        std::string getLayerName(
            const std::shared_ptr<FilesModelItem>& item,
            const uint16_t layerId);

    } // namespace panel

} // namespace mrv
