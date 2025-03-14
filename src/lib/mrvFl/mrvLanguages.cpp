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

#ifdef __APPLE__
#    include <CoreFoundation/CoreFoundation.h>
#endif

#include <vector>
#include <string>

#include <FL/fl_ask.H>

#include <tlCore/StringFormat.h>

#include "mrvCore/mrvEnv.h"
#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvOS.h"

#include "mrvWidgets/mrvPopupMenu.h"

#include "mrvFl/mrvCallbacks.h"
#include "mrvFl/mrvLanguages.h"
#include "mrvFl/mrvSession.h"

#include "mrvApp/mrvApp.h"

#include "mrvPreferencesUI.h"

#include "mrvFl/mrvIO.h"

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
    {_("English"), "en.UTF-8"},
    {_("Spanish"), "es.UTF-8"},
    {_("German"), "de.UTF-8"},
    {_("Hindi"), "hi_IN.UTF-8"},
    {_("Chinese Simplified"), "zh-CN.UTF-8"},
    {_("Portuguese"), "pt.UTF-8"},
    {_("French"), "fr.UTF-8"},
    {_("Russian"), "ru.UTF-8"},
    {nullptr, nullptr}};

void check_language(PreferencesUI* uiPrefs, int& language_index, mrv::App* app)
{
    int uiIndex = uiPrefs->uiLanguage->value();
    if (uiIndex != language_index)
    {
        const char* language = _(kLanguages[uiIndex].name);

        std::string question =
            tl::string::Format(
                _("Need to reboot mrv2 to change language to {0}.  "
                  "Are you sure you want to continue?"))
                .arg(language);

        int ok = fl_choice(question.c_str(), _("No"), _("Yes"), NULL, NULL);
        if (ok)
        {
            language_index = uiIndex;
            const char* language = kLanguages[uiIndex].code;

            // this would create a fontconfig error.
            // setenv( "LC_CTYPE", "UTF-8", 1 );
#ifdef _WIN32
            wchar_t wlanguage[32];
            fl_utf8towc(language, strlen(language), wlanguage, 32);
            setenv(L"LANGUAGE", wlanguage, 1);
#else
            setenv("LANGUAGE", language, 1);
#endif
            // Save ui preferences
            mrv::Preferences::save();

            // Save ui language
            Fl_Preferences base(
                mrv::prefspath().c_str(), "filmaura", "mrv2",
                Fl_Preferences::C_LOCALE);
            Fl_Preferences ui(base, "ui");
            ui.set("language_code", language);

            base.flush();

            // Save a temporary session file
            std::string session = mrv::tmppath();
            session += "/lang.mrv2s";

            mrv::session::save(session);

            app->cleanResources();

            mrv::os::execv("", session);
        }
        else
        {
            uiPrefs->uiLanguage->value(language_index);
        }
    }
}

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

void select_character(mrv::PopupMenu* o, bool colon)
{
    int i = o->value();
    if (i < 0)
        return;
    const char* p = o->text(i);
    o->copy_label(select_character(p, colon));
}

namespace mrv
{
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
        if (os::runningInTerminal())
        {
            out = os::sgetenv("LC_MESSAGES");
            if (out.empty())
            {
                if (out.empty())
                {
                    out = os::sgetenv("LANG");
                    if (out.empty())
                    {
                        out = os::sgetenv("LANGUAGE");
                    }
                }
            }
        }
        else
        {
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
        }
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
            wchar_t wlanguage[32];
            fl_utf8towc(langcode, strlen(langcode), wlanguage, 32);
            setenv(L"LANGUAGE", wlanguage, 1);
            std::wcerr << "Setting language to " << wlanguage
                       << std::endl;
            mrv::os::execv();
            exit(0);
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

        std::cerr << "defaultLocale=" << language << std::endl;
        
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
            if (strcmp(language, "C") == 0 || strncmp(language, "ar", 2) == 0 ||
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
        snprintf(buf, 256, "mrv2-v%s", mrv::version());
        bindtextdomain(buf, path.c_str());
        bind_textdomain_codeset(buf, "UTF-8");
        textdomain(buf);

        const std::string msg =
            tl::string::Format(_("Set Language to {0}, Numbers to {1}"))
                .arg(language)
                .arg(numericLocale);

        return msg;
    }

    LanguageTable kAudioLanguage[] = {
        {_("Arabic"), "ar"},     {_("Armenian"), "hy"},
        {_("Basque"), "eu"},     {_("Belarusian"), "be"},
        {_("Bengali"), "bn"},    {_("Bulgarian"), "bg"},
        {_("Catalan"), "ca"},    {_("Chinese"), "zh"},
        {_("Czech"), "cs"},      {_("Danish"), "da"},
        {_("Dutch"), "nl"},      {_("English"), "en"},
        {_("Finnish"), "fi"},    {_("French"), "fr"},
        {_("Galician"), "gl"},   {_("German"), "de"},
        {_("Greek"), "el"},      {_("Hebrew"), "he"},
        {_("Hindi"), "hi"},      {_("Icelandic"), "is"},
        {_("Indonesian"), "id"}, {_("Italian"), "it"},
        {_("Irish"), "ga"},      {_("Japanese"), "ja"},
        {_("Korean"), "ko"},     {_("Norwegan"), "no"},
        {_("Persian"), "fa"},    {_("Polish"), "pl"},
        {_("Portuguese"), "pt"}, {_("Spanish"), "es"},
        {_("Tibetan"), "to"},    {_("Romanian"), "ro"},
        {_("Russian"), "ru"},    {_("Serbian"), "sr"},
        {_("Slovenian"), "sl"},  {_("Swedish"), "sv"},
        {_("Thai"), "th"},       {_("Tibetan"), "bo"},
        {_("Turkish"), "tr"},    {_("Vietnamese"), "vi"},
        {_("Welsh"), "cy"},      {_("Yiddish"), "yi"},
        {_("Zulu"), "zu"},
    };

    std::string codeToLanguage(const std::string& code)
    {
        std::string out = code;
        std::string id = code.substr(0, 2);
        for (int i = 0; i < sizeof(kAudioLanguage) / sizeof(LanguageTable); ++i)
        {
            if (id == kAudioLanguage[i].code)
            {
                out = _(kAudioLanguage[i].name);
                break;
            }
        }
        return out;
    }

} // namespace mrv
