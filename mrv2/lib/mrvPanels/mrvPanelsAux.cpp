// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <limits>

#include <tlCore/StringFormat.h>

#include "mrvPanelsAux.h"

#include "mrViewer.h"

namespace mrv
{

    std::string getLayerName(uint16_t layerId, ViewerUI* ui)
    {
        std::string layer;
        unsigned numLayers = ui->uiColorChannel->children();
        if (layerId == std::numeric_limits<uint16_t>::max())
            layerId = 0;
        if (layerId < numLayers)
        {
            layer = "\n";
            layer += ui->uiColorChannel->child(layerId)->label();
        }
        else if (layerId == 0 && numLayers == 0)
        {
            layer = "\nColor";
        }
        else if (layerId >= ui->uiColorChannel->children())
        {
            layer = tl::string::Format("\nLayer Number={0}").arg(layerId);
        }
        return layer;
    }

} // namespace mrv
