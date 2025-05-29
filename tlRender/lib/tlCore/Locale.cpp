// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <cstdlib>
#include <cstring>
#include <iostream>

#include <tlCore/Locale.h>

namespace tl
{
    namespace locale
    {
        SetAndRestore::SetAndRestore(const int category, const char* locale) :
            m_category(category)
        {
            m_mutex.lock();
            m_oldLocale = strdup(setlocale(category, NULL));
            if (!m_oldLocale)
                throw std::runtime_error("Could not allocate locale");
            setlocale(category, locale);
        }

        SetAndRestore::~SetAndRestore()
        {
            setlocale(m_category, m_oldLocale);
            free(m_oldLocale);
            m_mutex.unlock();
        }

        std::mutex SetAndRestore::m_mutex;

    } // namespace locale
} // namespace tl
