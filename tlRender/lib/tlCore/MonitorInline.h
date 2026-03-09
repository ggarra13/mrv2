// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace monitor
    {

        inline bool Capabilities::operator==(const Capabilities& other) const
        {
            return (hdr_supported == other.hdr_supported &&
                    hdr_enabled == other.hdr_enabled &&
                    min_nits == other.min_nits &&
                    max_nits == other.max_nits &&
                    red  == other.red &&
                    green == other.green &&
                    blue == other.blue &&
                    white == other.white);
        }

        inline bool Capabilities::operator!=(const Capabilities& other) const
        {
            return !(other == *this);
        }        
        
    } // namespace monitor
} // namespace tl
