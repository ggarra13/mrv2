// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

//! Project namespace.
namespace tl
{
    namespace error
    {
        //! Assert (for convenience use the TLRENDER_ASSERT macro).
        void _assert(const char* file, int line);
    } // namespace error
} // namespace tl

//! Assert macro.
#if defined(TLRENDER_ASSERT)
#    undef TLRENDER_ASSERT
#    define TLRENDER_ASSERT(value)                                             \
        if (!(value))                                                          \
        tl::error::_assert(__FILE__, __LINE__)
#else // TLRENDER_ASSERT
#    define TLRENDER_ASSERT(value)
#endif // TLRENDER_ASSERT
