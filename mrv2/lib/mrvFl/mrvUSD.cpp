// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#ifdef TLRENDER_USD

#    include <tlCore/StringFormat.h>

#    include "mrvOptions/mrvUSD.h"

#    include "mrViewer.h"

namespace mrv
{
    namespace usd
    {

        void sendIOOptions()
        {
            auto player = App::ui->uiView->getTimelinePlayer();
            if (!player)
                return;

            auto o = renderOptions();

            io::Options ioOptions;
            ioOptions["USD/rendererName"] =
                string::Format("{0}").arg(o.rendererName);
            ioOptions["USD/renderWidth"] =
                string::Format("{0}").arg(o.renderWidth);
            ioOptions["USD/complexity"] =
                string::Format("{0}").arg(o.complexity);
            {
                std::stringstream ss;
                ss << o.drawMode;
                ioOptions["USD/drawMode"] = ss.str();
            }
            ioOptions["USD/enableLighting"] =
                string::Format("{0}").arg(o.enableLighting);
            ioOptions["USD/enableSceneLights"] =
                string::Format("{0}").arg(o.enableSceneLights);
            ioOptions["USD/enableSceneMaterials"] =
                string::Format("{0}").arg(o.enableSceneMaterials);
            ioOptions["USD/sRGB"] = string::Format("{0}").arg(o.sRGB);
            ioOptions["USD/stageCacheCount"] =
                string::Format("{0}").arg(o.stageCache);
            ioOptions["USD/diskCacheByteCount"] =
                string::Format("{0}").arg(o.diskCache);

            player->setIOOptions(ioOptions);
        }

    } // namespace usd
} // namespace mrv

#endif
