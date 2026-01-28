// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <filesystem>
namespace fs = std::filesystem;

#ifdef _WIN32
#    define WIN32_LEAN_AND_MEAN
#    include <windows.h>
#    include <shellapi.h>
#else
#    include <unistd.h>
#endif

#include "mrvOS.h"
#include "mrvEnv.h"
#include "mrvHome.h"
#include "mrvI8N.h"

#ifdef __APPLE__
#    include <CoreFoundation/CoreFoundation.h>
#endif

#include <FL/Fl_Preferences.H>
#include <FL/fl_ask.H>
#include <FL/fl_utf8.h>

#include <iostream>
#include <vector>
#include <string>




namespace
{
    const char* kModule = "lang";
}

struct LanguageTable
{
    const char* name; // in English
    const char* code;
};

LanguageTable kLanguages[] = {
    {"English", "en.UTF-8"},
    {"Spanish", "es.UTF-8"},
    {"German", "de.UTF-8"},
    {"Hindi", "hi_IN.UTF-8"},
    {"Chinese Simplified", "zh-CN.UTF-8"},
    {"Portuguese", "pt.UTF-8"},
    {"French", "fr.UTF-8"},
    {"Russian", "ru.UTF-8"},
    {"Japanese", "ja.UTF-8"},
    {nullptr, nullptr}};


char* select_character(const char* p, bool colon)
{
    int size = fl_utf8len1(p[0]);
    const char* end = p + size;

    int len;
    unsigned code;
    if (*p & 0x80)
    { // what should be a multibyte encoding
        code = fl_utf8decode(p, end, &len);
        if (len < 2)
            code = 0xFFFD; // Turn errors into REPLACEMENT CHARACTER
    }
    else
    { // handle the 1-byte UTF-8 encoding:
        code = *p;
        len = 1;
    }

    static char buf[8];
    memset(buf, 0, 7);
    if (len > 1)
    {
        len = fl_utf8encode(code, buf);
    }
    else
    {
        buf[0] = *p;
    }
    if (colon)
    {
        buf[len] = ':';
        ++len;
    }
    buf[len] = 0;

    return buf;
}

namespace mrv
{
    const char* kVersion = MRV2_VERSION;
    const char* version()
    {
        return kVersion;
    }
    
    std::vector<std::string> getLanguages()
    {
        std::vector<std::string> out;

        LanguageTable* t = kLanguages;
        for (; (*t).name; ++t)
        {
            out.push_back((*t).name);
        }

        return out;
    }

    std::vector<std::string> getLanguageCodes()
    {
        std::vector<std::string> out;

        LanguageTable* t = kLanguages;
        for (; (*t).code; ++t)
        {
            out.push_back((*t).code);
        }

        return out;
    }

    std::string getDefaultLocale()
    {
        std::string out = "en_US.UTF-8";
#ifdef _WIN32
        // For Windows, we get the default language from the system one.
        wchar_t wbuffer[LOCALE_NAME_MAX_LENGTH];
        if (GetUserDefaultLocaleName(wbuffer, LOCALE_NAME_MAX_LENGTH))
        {
            static char buffer[LOCALE_NAME_MAX_LENGTH * 4];
            int len = WideCharToMultiByte(
                CP_UTF8, 0, wbuffer, -1, buffer, sizeof(buffer), nullptr,
                nullptr);
            if (len > 0)
            {
                out = buffer;
                out += ".UTF-8"; // Ensure POSIX format

                // Assuming lang_REGION format
                std::string localeStr(buffer);
                size_t dashPos = localeStr.find('-');
                if (dashPos != std::string::npos) {
                    std::string lang = localeStr.substr(0, dashPos);
                    std::string region = localeStr.substr(dashPos + 1);
                    // Convert to POSIX format
                    out = lang + "_" + region + ".UTF-8";
                }

                // Compare it to our known languages or default to en_US.UTF-8.
                auto languageCodes = getLanguageCodes();
                for (const auto& code : languageCodes)
                {
                    if (strncmp(code.c_str(), out.c_str(), 2) == 0)
                        return out;
                }
                out = "en_US.UTF-8";
            }
        }
#else
#    ifdef __APPLE__
            CFLocaleRef locale = CFLocaleCopyCurrent();
            if (!locale)
                return "en_US.UTF-8"; // Fallback to English if retrieval fails

            CFStringRef lang =
                (CFStringRef)CFLocaleGetValue(locale, kCFLocaleIdentifier);
            char buffer[64];
            if (CFStringGetCString(
                    lang, buffer, sizeof(buffer), kCFStringEncodingUTF8))
            {
                CFRelease(locale);
                const std::string utf8_locale = std::string(buffer) + ".UTF-8";
                return utf8_locale;
            }

            CFRelease(locale);
            return "en_US.UTF-8";
#    else
            out = "en_US.UTF-8";
#    endif
#endif
        return out;
    }

    void initLocale(const char*& langcode)
    {
        // We use a fake variable to override language saved in user's prefs.
        // We use this to document the python mrv2 module from within mrv2.
        const char* codeOverride = fl_getenv("LANGUAGE_CODE");
        if (codeOverride && strlen(codeOverride) != 0)
            langcode = codeOverride;

#ifdef _WIN32
        //
        // Windows cannot change the environment variables of the same
        // process.
        // That's why we end up calling execv() which will re-initialize
        // the application with all its arguments intact, *BUT* with
        // LANGUAGE set in the environment now, which we end up reading
        // here.
        //
        const char* language = fl_getenv("LANGUAGE");
        if (!language || strncmp(language, langcode, 2) != 0)
        {
            const std::string languageCheck = langcode;
            wchar_t wlanguage[32];
            fl_utf8towc(langcode, strlen(langcode), wlanguage, 32);
            setenv(L"LANGUAGE", wlanguage, 1);
            //mrv::os::execv();
        }
#else
        // Needed for Linux and OSX.  See above for windows.
        setenv("LANGUAGE", langcode, 1);
#endif


        setlocale(LC_ALL, "");
        setlocale(LC_ALL, langcode);

#ifdef __APPLE__
        setenv("LC_MESSAGES", langcode, 1);
#endif
    }

    std::string setLanguageLocale()
    {
#if defined __APPLE__
        setenv("LC_CTYPE", "UTF-8", 1);
#endif

        char languageCode[32];
        const char* language = "en_US.UTF-8";
        const std::string& defaultLocale = getDefaultLocale();
        if (!defaultLocale.empty())
            language = defaultLocale.c_str();
        
        // Load ui language preferences to see if user chose a different
        // language than the OS one.
        Fl_Preferences base(
            mrv::prefspath().c_str(), "filmaura", "mrv2",
            Fl_Preferences::C_LOCALE);
        Fl_Preferences ui(base, "ui");
        ui.get("language_code", languageCode, "", 32);

        if (strlen(languageCode) != 0)
        {
            language = languageCode;
        }

        //
        // Now, initialize the chosen locale for each platform.
        // Note that gettext() behaves differently on each OS, that's
        // why we need a special function for it.
        //
        initLocale(language);

        const char* numericLocale = language;
        if (language)
        {
            // This is for Apple mainly, as it we just set LC_MESSAGES only
            // and not the numeric locale, which we must set separately for
            // those locales that use periods in their floating point.
            if (strcmp(language, "C") == 0 ||
                strncmp(language, "ar", 2) == 0 ||
                strncmp(language, "en", 2) == 0 ||
                strncmp(language, "hi", 2) == 0 ||
                strncmp(language, "ja", 2) == 0 ||
                strncmp(language, "ko", 2) == 0 ||
                strncmp(language, "zh", 2) == 0)
                numericLocale = "C";
        }

        setlocale(LC_NUMERIC, numericLocale);
        setlocale(LC_TIME, numericLocale);

        // Create and install global locale
        // On Ubuntu and Debian the locales are not fully built. As root:
        //
        // Debian Code:
        //
        // $ apt-get install locales && dpkg-reconfigure locales
        //
        // Ubuntu Code:
        //
        // $ apt-get install locales
        // $ locale-gen en_US.UTF-8
        // $ update-locale LANG=en_US.UTF-8
        // $ reboot
        //

        std::string path = mrv::rootpath();
        path += "/share/locale/";

        char buf[256];
        snprintf(buf, 256, "license_helper-v%s", mrv::version());
        std::cerr << buf << std::endl;
        std::cerr << path << std::endl;
        bindtextdomain(buf, path.c_str());
        bind_textdomain_codeset(buf, "UTF-8");
        textdomain(buf);

        return "";
    }
    
} // namespace mrv
