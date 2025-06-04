// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace math
    {
        template <typename T>
        inline const T& Random::get(const std::vector<T>& value)
        {
            return value[get(0, static_cast<int>(value.size()) - 1)];
        }
    } // namespace math
} // namespace tl
