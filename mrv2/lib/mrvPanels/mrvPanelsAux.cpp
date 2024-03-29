// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvCore/mrvColorSpaces.h"

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

            const std::string& layer =
                mrv::color::layer(item->videoLayers[layerId]);
            return "\n" + layer;
        }

    } // namespace panel

} // namespace mrv
