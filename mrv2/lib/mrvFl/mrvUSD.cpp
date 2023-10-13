// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#ifdef TLRENDER_USD

#    include <tlCore/StringFormat.h>

#    include "mrvCore/mrvUSD.h"

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
            ioOptions["USD/stageCacheCount"] =
                string::Format("{0}").arg(o.stageCache);
            ioOptions["USD/diskCacheByteCount"] =
                string::Format("{0}").arg(o.diskCache);

            player->setIOOptions(ioOptions);
        }

    } // namespace usd
} // namespace mrv

#endif
