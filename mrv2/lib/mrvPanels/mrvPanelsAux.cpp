// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvPanelsAux.h"

namespace mrv
{
    namespace panel
    {

        std::string getLayerName(
            const std::shared_ptr<FilesModelItem>& item, const uint16_t layerId)
        {
            if (!item || layerId >= item->videoLayers.size())
                return "";

            std::string layer = item->videoLayers[layerId];
            if (layer == "Default" || layer == "A,B,G,R" || layer == "B,G,R")
                layer = "Color";
            return "\n" + layer;
        }

    } // namespace panel

} // namespace mrv
