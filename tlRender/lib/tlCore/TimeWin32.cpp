// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/Time.h>

#include <sstream>

#ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace tl
{
    namespace time
    {
        void sleep(const std::chrono::microseconds& value)
        {
            if (HANDLE h = CreateWaitableTimer(NULL, TRUE, NULL))
            {
                LARGE_INTEGER l;
                l.QuadPart =
                    -std::chrono::duration_cast<std::chrono::nanoseconds>(value)
                         .count() /
                    100;
                if (SetWaitableTimer(h, &l, 0, NULL, NULL, FALSE))
                {
                    WaitForSingleObject(h, INFINITE);
                }
                CloseHandle(h);
            }
        }
    } // namespace time
} // namespace tl
