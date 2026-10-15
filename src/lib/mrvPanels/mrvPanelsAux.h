// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

#include "mrvOS/mrvI8N.h"

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

        /**
         * 0 for normal thumbnails, 1 for small thumbnails, 2 for none.
         *
         * @param thumbnailType See above.
         *
         * @return size of thumbnail if any.
         */
        image::Size calculateImageSize(int thumbnailType);

        std::string getLayerName(
            const std::shared_ptr<FilesModelItem>& item,
            const uint16_t layerId);

    } // namespace panel

} // namespace mrv
