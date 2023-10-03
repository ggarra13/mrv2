// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

namespace mrv
{

    //! Fuzzy equal comparison between floats or doubles.
    template < typename T >
    inline bool is_equal(const T x1, const T x2, const T e = 1e-5)
    {
        return (((x1 > x2) ? x1 - x2 : x2 - x1) <= e);
    }

} // namespace mrv
