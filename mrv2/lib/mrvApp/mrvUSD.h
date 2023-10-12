// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#ifdef TLRENDER_USD

#    include <tlIO/USD.h>

namespace mrv
{
    namespace usd
    {
        struct RenderOptions
        {
            int renderWidth = 1920;
            float complexity = 1.F;
            tl::usd::DrawMode drawMode = tl::usd::DrawMode::ShadedSmooth;
            bool enableLighting = true;
            size_t stageCache = 10;
            size_t diskCache = 0;
        };

        //! Return the current USD Render Options
        RenderOptions renderOptions();

        //! Set the current USD Render Options
        bool setRenderOptions(const RenderOptions& o);
    } // namespace usd
} // namespace mrv

#endif
