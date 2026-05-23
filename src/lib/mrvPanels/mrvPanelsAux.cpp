// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "mrvPanelsAux.h"

#include "mrViewer.h"

#include "mrvCore/mrvColorSpaces.h"

#include "mrvOptions/mrvEnums.h"

namespace mrv
{
    namespace panel
    {
        image::Size calculateImageSize()
        {
            image::Size out;
            switch(App::ui->uiPrefs->uiPrefsPanelThumbnails->value())
            {
            case kThumbnailNormal:
                out = image::Size(128, 64);
                break;
            case kThumbnailSmall:
            default:
                out = image::Size(64, 32);
                break;
            }
            return out;
        }
        
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
