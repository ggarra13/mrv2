// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvPanelsAux.h"

namespace mrv
{

    std::string getLayerName(
        const std::shared_ptr<FilesModelItem>& item, const uint16_t layerId)
    {
        std::string layer = item->videoLayers[layerId];
        if (layer == "Default" || layer == "A,B,G,R" || layer == "B,G,R")
            layer = "Color";
        return "\n" + layer;
    }

} // namespace mrv
