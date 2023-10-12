// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#ifdef TLRENDER_USD

#    include <mrvApp/mrvUSD.h>

namespace mrv
{
    namespace usd
    {
        bool RenderOptions::operator==(const RenderOptions& b) const
        {
            return (*this == b);
        }

        bool RenderOptions::operator!=(const RenderOptions& b) const
        {
            return !(*this == b);
        }
    } // namespace usd
} // namespace mrv

#endif
