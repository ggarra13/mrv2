// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <clocale>
#include <mutex>

namespace mrv
{
    namespace locale
    {
        //! Thread safe setlocale.
        struct SetAndRestore
        {
            SetAndRestore(
                const int category = LC_NUMERIC, const char* locale = "C");
            ~SetAndRestore();

            const char* stored() { return m_oldLocale; };

        private:
            static std::mutex m_mutex;
            int m_category;
            char* m_oldLocale;
        };

    } // namespace locale
} // namespace mrv
