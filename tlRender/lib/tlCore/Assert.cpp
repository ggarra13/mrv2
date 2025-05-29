// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/Assert.h>

#include <iostream>

#include <stdlib.h>

namespace tl
{
    namespace error
    {
        void _assert(const char* file, int line)
        {
            std::cout << "ASSERT: " << file << ":" << line << std::endl;
            abort();
        }
    } // namespace error
} // namespace tl