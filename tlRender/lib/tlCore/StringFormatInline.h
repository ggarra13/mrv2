// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <sstream>

namespace tl
{
    namespace string
    {
        template <typename T> inline Format& Format::arg(T value)
        {
            std::stringstream ss;
            ss << value;
            return arg(ss.str());
        }
    } // namespace string
} // namespace tl
