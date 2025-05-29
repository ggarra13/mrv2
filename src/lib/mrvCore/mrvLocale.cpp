// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "mrvCore/mrvLocale.h"

#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <mutex>

namespace mrv
{
    namespace locale
    {
        SetAndRestore::SetAndRestore(const int category, const char* locale) :
            m_category(category)
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_oldLocale = strdup(setlocale(category, NULL));
            if (!m_oldLocale)
                throw std::runtime_error("Could not allocate locale");
            setlocale(category, locale);
        }

        SetAndRestore::~SetAndRestore()
        {
            setlocale(m_category, m_oldLocale);
            free(m_oldLocale);
        }

        std::mutex SetAndRestore::m_mutex;

    } // namespace locale
} // namespace mrv
