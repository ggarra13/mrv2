// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <stdexcept>

namespace tl
{
    //! Errors
    namespace error
    {
#if defined(_WINDOWS)
        //! Get an error string from a Windows system call.
        std::string getLastError();
#endif // _WINDOWS

        //! Parse error.
        class ParseError : public std::invalid_argument
        {
        public:
            ParseError();
        };
    } // namespace error
} // namespace tl
