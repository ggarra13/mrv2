
#include <cstdlib>
#include <cstring>
#include <iostream>

#include "mrvCore/mrvLocale.h"

namespace mrv
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
} // namespace mrv
