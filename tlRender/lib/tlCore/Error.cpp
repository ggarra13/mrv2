// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/Error.h>

namespace tl
{
    namespace error
    {
        ParseError::ParseError() :
            invalid_argument("Cannot parse value")
        {
        }
    } // namespace error
} // namespace tl
