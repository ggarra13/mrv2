// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cinttypes>
#include <random>

#include "mrvCore/mrvString.h"

namespace mrv
{
    namespace string
    {
        std::string random()
        {
            static const std::string charset =
                "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ012345678"
                "9";
            std::string out;

            const int length = 16;
            std::random_device rd;
            std::default_random_engine generator(rd());
            std::uniform_int_distribution<int> distribution(
                0, charset.size() - 1);

            for (int i = 0; i < length; ++i)
            {
                out += charset[distribution(generator)];
            }
            return out;
        }

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
