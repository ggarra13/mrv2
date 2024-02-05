// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#ifdef TLRENDER_USD

#    include <tlCore/StringFormat.h>

#    include "mrvOptions/mrvUSD.h"

#    include "mrvPanels/mrvPanelsCallbacks.h"

#    include "mrvApp/mrvSettingsObject.h"
#    include "mrvApp/mrvApp.h"

namespace mrv
{
    namespace usd
    {
        RenderOptions renderOptions()
        {
            auto settings = App::app->settings();
            RenderOptions o;
            o.rendererName =
                settings->getValue<std::string>("USD/rendererName");
            o.renderWidth = settings->getValue<int>("USD/renderWidth");
            o.complexity = settings->getValue<float>("USD/complexity");
            o.drawMode = static_cast<tl::usd::DrawMode>(
                settings->getValue<int>("USD/drawMode"));
            o.enableLighting = settings->getValue<bool>("USD/enableLighting");
            o.sRGB = settings->getValue<bool>("USD/sRGB");
            o.stageCache = settings->getValue<int>("USD/stageCache");
            o.diskCache = settings->getValue<int>("USD/diskCache");
            return o;
        }

        void setRenderOptions(const RenderOptions& o)
        {
            if (o == renderOptions())
                return;

            using panel::usdPanel;

            auto settings = App::app->settings();
            settings->setValue("USD/rendererName", o.rendererName);
            settings->setValue("USD/renderWidth", o.renderWidth);
            settings->setValue("USD/complexity", o.complexity);
            settings->setValue("USD/drawMode", o.drawMode);
            settings->setValue("USD/enableLighting", o.enableLighting);
            settings->setValue("USD/sRGB", o.sRGB);
            settings->setValue("USD/stageCache", o.stageCache);
            settings->setValue("USD/diskCache", o.diskCache);

            sendIOOptions();

            if (usdPanel)
                usdPanel->refresh();
        }

    } // namespace usd
} // namespace mrv

#endif
