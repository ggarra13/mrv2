
#pragma once

#include <clocale>
#include <mutex>

#define StoreLocale mrv::locale::SetAndRestore saved;

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
