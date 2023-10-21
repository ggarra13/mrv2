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
            o.renderWidth = settingsObject->getValue<int>("USD/renderWidth");
            o.complexity = settingsObject->getValue<float>("USD/complexity");
            o.drawMode = static_cast<tl::usd::DrawMode>(
                settingsObject->getValue<int>("USD/drawMode"));
            o.enableLighting =
                settingsObject->getValue<bool>("USD/enableLighting");
            o.sRGB = settingsObject->getValue<bool>("USD/sRGB");
            o.stageCache = settingsObject->getValue<int>("USD/stageCache");
            o.diskCache = settingsObject->getValue<int>("USD/diskCache");
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
