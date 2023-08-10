
#ifdef TLRENDER_USD

#    include "mrvApp/mrvUSD.h"

#    include "mrvPanels/mrvPanelsCallbacks.h"

#    include "mrvApp/mrvSettingsObject.h"
#    include "mrvApp/App.h"

namespace mrv
{
    namespace usd
    {
        tl::usd::RenderOptions renderOptions()
        {
            App* app = App::application();
            auto settingsObject = app->settingsObject();
            tl::usd::RenderOptions o;
            o.renderWidth =
                std_any_cast<int>(settingsObject->value("usd/renderWidth"));
            o.complexity =
                std_any_cast<float>(settingsObject->value("usd/complexity"));
            o.drawMode = static_cast<tl::usd::DrawMode>(
                std_any_cast<int>(settingsObject->value("usd/drawMode")));
            o.enableLighting = static_cast<bool>(
                std_any_cast<int>(settingsObject->value("usd/enableLighting")));
            o.stageCacheCount =
                std_any_cast<int>(settingsObject->value("usd/stageCacheCount"));
            o.diskCacheByteCount = std_any_cast<int>(
                settingsObject->value("usd/diskCacheByteCount"));
            return o;
        }

        bool setRenderOptions(const tl::usd::RenderOptions& o)
        {
            App* app = App::application();
            auto settingsObject = app->settingsObject();
            settingsObject->setValue("usd/renderWidth", o.renderWidth);
            settingsObject->setValue("usd/complexity", o.complexity);
            settingsObject->setValue("usd/drawMode", o.drawMode);
            settingsObject->setValue("usd/enableLighting", o.enableLighting);
            settingsObject->setValue("usd/stageCacheCount", o.stageCacheCount);
            settingsObject->setValue(
                "usd/diskCacheByteCount", o.diskCacheByteCount);

            if (usdPanel)
                usdPanel->refresh();
            return true;
        }
    } // namespace usd
} // namespace mrv

#endif
