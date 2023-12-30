// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#ifdef TLRENDER_USD

#    include <mrvCore/mrvUSD.h>

namespace mrv
{
    namespace usd
    {
        bool RenderOptions::operator==(const RenderOptions& b) const
        {
            return (
                rendererName == b.rendererName &&
                renderWidth == b.renderWidth && complexity == b.complexity &&
                drawMode == b.drawMode && enableLighting == b.enableLighting &&
                sRGB == b.sRGB && stageCache == b.stageCache &&
                diskCache == b.diskCache);
        }

        bool RenderOptions::operator!=(const RenderOptions& b) const
        {
            return !(*this == b);
        }
    } // namespace usd
} // namespace mrv

#endif
