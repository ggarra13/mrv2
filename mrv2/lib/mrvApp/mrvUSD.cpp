// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#ifdef TLRENDER_USD

#    include <tlCore/StringFormat.h>

#    include "mrvPanels/mrvPanelsCallbacks.h"

#    include "mrvApp/mrvSettingsObject.h"
#    include "mrvApp/mrvUSD.h"
#    include "mrvApp/App.h"

#    include "mrViewer.h"

namespace mrv
{
    namespace usd
    {
        RenderOptions renderOptions()
        {
            auto settingsObject = App::app->settingsObject();
            RenderOptions o;
            o.renderWidth =
                std_any_cast<int>(settingsObject->value("USD/renderWidth"));
            o.complexity =
                std_any_cast<float>(settingsObject->value("USD/complexity"));
            o.drawMode = static_cast<tl::usd::DrawMode>(
                std_any_cast<int>(settingsObject->value("USD/drawMode")));
            o.enableLighting = static_cast<bool>(
                std_any_cast<int>(settingsObject->value("USD/enableLighting")));
            o.stageCache =
                std_any_cast<int>(settingsObject->value("USD/stageCache"));
            o.diskCache =
                std_any_cast<int>(settingsObject->value("USD/diskCache"));
            return o;
        }

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

        void setRenderOptions(const RenderOptions& o)
        {
            auto settingsObject = App::app->settingsObject();
            if (o == renderOptions())
                return;

            settingsObject->setValue("USD/renderWidth", o.renderWidth);
            settingsObject->setValue("USD/complexity", o.complexity);
            settingsObject->setValue("USD/drawMode", o.drawMode);
            settingsObject->setValue("USD/enableLighting", o.enableLighting);
            settingsObject->setValue("USD/stageCache", o.stageCache);
            settingsObject->setValue("USD/diskCache", o.diskCache);

            sendIOOptions();

            if (usdPanel)
                usdPanel->refresh();
        }

    } // namespace usd
} // namespace mrv

#endif
