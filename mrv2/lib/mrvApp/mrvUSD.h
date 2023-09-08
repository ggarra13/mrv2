// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#ifdef TLRENDER_USD

#    include <tlIO/USD.h>

namespace mrv
{
    namespace usd
    {
        //! Return the current USD Render Options
        tl::usd::RenderOptions renderOptions();

        //! Set the current USD Render Options
        bool setRenderOptions(const tl::usd::RenderOptions& o);
    } // namespace usd
} // namespace mrv

#endif
