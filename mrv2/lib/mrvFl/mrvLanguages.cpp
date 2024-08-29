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

#include <vector>
#include <string>

#include <FL/fl_ask.H>

#include <tlCore/StringFormat.h>

#include "mrvCore/mrvEnv.h"
#include "mrvCore/mrvHome.h"

#include "mrvWidgets/mrvPopupMenu.h"

#include "mrvFl/mrvCallbacks.h"
#include "mrvFl/mrvLanguages.h"
#include "mrvFl/mrvSession.h"

#include "mrvApp/mrvApp.h"

#include "mrvPreferencesUI.h"

#include "mrvFl/mrvIO.h"
#include "mrvCore/mrvOS.h"

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
    {_("Italian"), "it.UTF-8"},
    {_("Traditional Chinese"), "zh.UTF-8"},
    {_("Chinese Simplified"), "zh-CN.UTF-8"},
    {nullptr, nullptr}
};


void check_language(PreferencesUI* uiPrefs, int& language_index, mrv::App* app)
{
    int uiIndex = uiPrefs->uiLanguage->value();
    if (uiIndex != language_index)
    {
        int ok = fl_choice(
            _("Need to reboot mrv2 to change language.  "
              "Are you sure you want to continue?"),
            _("No"), _("Yes"), NULL, NULL);
        if (ok)
        {
            language_index = uiIndex;
            const char* language = kLanguages[uiIndex].code;

            // this would create a fontconfig error.
            // setenv( "LC_CTYPE", "UTF-8", 1 );
            setenv("LANGUAGE", language, 1);

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
            uiPrefs->uiLanguage->value(uiIndex);
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
            out.push_back(_((*t).name));
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

    
    void initLocale(const char*& langcode)
    {
        // We use a fake variable to override language saved in user's prefs.
        // We use this to document the python mrv2 module from within mrv2.
        const char* codeOverride = fl_getenv("LANGUAGE_CODE");
        if (codeOverride && strlen(codeOverride) != 0)
            langcode = codeOverride;

#ifdef _WIN32
        const char* language = fl_getenv("LANGUAGE");
        if ((!language || strlen(language) == 0))
        {
            wchar_t wbuffer[LOCALE_NAME_MAX_LENGTH];
            if (GetUserDefaultLocaleName(wbuffer, LOCALE_NAME_MAX_LENGTH))
            {
                static char buffer[256];
                int len = WideCharToMultiByte(
                    CP_UTF8, 0, wbuffer, -1, buffer, sizeof(buffer), nullptr,
                    nullptr);
                if (len > 0)
                {
                    language = buffer;
                }
            }
        }
        if (!language || strncmp(language, langcode, 2) != 0)
        {
            setenv("LANGUAGE", langcode, 1);
            mrv::os::execv();
            exit(0);
        }
#endif

        // Needed for Linux and OSX.  See above for windows.
        setenv("LANGUAGE", langcode, 1);

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

        Fl_Preferences base(
            mrv::prefspath().c_str(), "filmaura", "mrv2",
            Fl_Preferences::C_LOCALE);

        // Load ui language preferences
        Fl_Preferences ui(base, "ui");

        ui.get("language_code", languageCode, "", 32);

        if (strlen(languageCode) != 0)
        {
            language = languageCode;
        }

        initLocale(language);

        const char* numericLocale = setlocale(LC_ALL, NULL);

#if defined __APPLE__ && defined __MACH__
        numericLocale = setlocale(LC_MESSAGES, NULL);
#endif
        if (language)
        {
            // This is for Apple mainly, as it we just set LC_MESSAGES only
            // and not the numeric locale, which we must set separately for
            // those locales that use periods in their floating point.
            if (strcmp(language, "C") == 0 || strncmp(language, "ar", 2) == 0 ||
                strncmp(language, "en", 2) == 0 ||
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
