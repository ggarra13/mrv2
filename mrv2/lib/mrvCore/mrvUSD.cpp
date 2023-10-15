// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#ifdef TLRENDER_USD

#    include <tlCore/StringFormat.h>

#    include "mrvCore/mrvUSD.h"

#    include "mrvPanels/mrvPanelsCallbacks.h"

#    include "mrvApp/mrvSettingsObject.h"
#    include "mrvApp/App.h"

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
            o.sRGB = static_cast<bool>(
                std_any_cast<int>(settingsObject->value("USD/sRGB")));
            o.stageCache =
                std_any_cast<int>(settingsObject->value("USD/stageCache"));
            o.diskCache =
                std_any_cast<int>(settingsObject->value("USD/diskCache"));
            return o;
        }

        void setRenderOptions(const RenderOptions& o)
        {
            if (o == renderOptions())
                return;

            using panel::usdPanel;

            auto settingsObject = App::app->settingsObject();
            settingsObject->setValue("USD/renderWidth", o.renderWidth);
            settingsObject->setValue("USD/complexity", o.complexity);
            settingsObject->setValue("USD/drawMode", o.drawMode);
            settingsObject->setValue("USD/enableLighting", o.enableLighting);
            settingsObject->setValue("USD/sRGB", o.sRGB);
            settingsObject->setValue("USD/stageCache", o.stageCache);
            settingsObject->setValue("USD/diskCache", o.diskCache);

            sendIOOptions();

            if (usdPanel)
                usdPanel->refresh();
        }

    } // namespace usd
} // namespace mrv

#endif
