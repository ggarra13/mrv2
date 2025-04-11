// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/IO.h>

namespace tl
{
    namespace io
    {
        Options merge(const Options& a, const Options& b)
        {
            Options out = b;
            for (const auto& i : a)
            {
                out[i.first] = i.second;
            }
            return out;
        }
    } // namespace io
} // namespace tl
