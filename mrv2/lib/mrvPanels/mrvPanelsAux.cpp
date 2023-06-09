// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/StringFormat.h>

#include "mrvPanelsAux.h"

#include "mrViewer.h"

namespace mrv
{

    std::string getLayerName(uint16_t layerId, ViewerUI* ui)
    {
        std::string layer;
        if (layerId < ui->uiColorChannel->children())
        {
            layer = "\n";
            layer += ui->uiColorChannel->child(layerId)->label();
        }

        if (layerId >= ui->uiColorChannel->children())
        {
            layer = tl::string::Format("\n{0}").arg(layerId);
        }
        return layer;
    }

} // namespace mrv
