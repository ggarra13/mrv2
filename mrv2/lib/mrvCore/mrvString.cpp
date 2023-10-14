// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cinttypes>

#include "mrvCore/mrvString.h"

namespace mrv
{
    namespace string
    {
        int64_t String::toInt() const
        {
            int64_t r = 0;
            sscanf(c_str(), "%" PRId64, &r);
            return r;
        }

        double String::toDouble() const
        {
            double r = 0;
            sscanf(c_str(), "%lg", &r);
            return r;
        }

    } // namespace string
} // namespace mrv
