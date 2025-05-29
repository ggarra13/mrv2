// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/Assert.h>

#include <sstream>

namespace tl
{
    namespace tests
    {
        template <typename T>
        inline void ITest::_enum(
            const std::string& name,
            const std::function<std::vector<T>(void)>& value)
        {
            for (auto i : value())
            {
                {
                    std::stringstream ss;
                    ss << name << ": " << i;
                    _print(ss.str());
                }
                {
                    std::stringstream ss;
                    ss << i;
                    T j;
                    ss >> j;
                    TLRENDER_ASSERT(i == j);
                }
            }
        }
    } // namespace tests
} // namespace tl
