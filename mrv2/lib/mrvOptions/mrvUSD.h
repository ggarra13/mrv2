// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#ifdef TLRENDER_USD

#    include <tlIO/USD.h>

namespace mrv
{
    namespace usd
    {
        struct RenderOptions
        {
            std::string rendererName = "GL";
            int renderWidth = 1920;
            float complexity = 1.F;
            tl::usd::DrawMode drawMode = tl::usd::DrawMode::ShadedSmooth;
            bool enableLighting = true;
            bool enableSceneLights = false;
            bool enableSceneMaterials = true;
            bool sRGB = true;
            size_t stageCache = 10;
            size_t diskCache = 0;

            bool operator==(const RenderOptions& b) const;
            bool operator!=(const RenderOptions& b) const;
        };

        //! Return the current USD Render Options
        RenderOptions renderOptions();

        //! Set the current USD Render Options
        void setRenderOptions(const RenderOptions& o);

        //! Set the current USD Render Options
        void sendIOOptions();
    } // namespace usd
} // namespace mrv

#endif
