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
#include "mrvFl/mrvCallbacks.h"

#include "mrvPreferencesUI.h"

#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "lang";
}

LanguageTable kLanguages[18] = {
    {_("English"), "en.UTF-8"},
    {_("Spanish"), "es.UTF-8"},
};

#ifdef _WIN32
namespace
{
    int win32_execv()
    {
        // Get the full command line string
        LPWSTR lpCmdLine = GetCommandLineW();

        // Parse the command line string into an array of arguments
        int argc;
        LPWSTR* argv = CommandLineToArgvW(lpCmdLine, &argc);

        if (argv == NULL)
        {
            wprintf(L"Failed to parse command line\n");
            return EXIT_FAILURE;
        }

        // Enclose argv[0] in double quotes if it contains spaces
        LPWSTR cmd = argv[0];
        if (wcschr(cmd, L' ') != NULL)
        {
            size_t len = wcslen(cmd) + 3; // 2 for quotes, 1 for null terminator
            LPWSTR quoted_cmd = (LPWSTR)malloc(len * sizeof(wchar_t));
            if (quoted_cmd == NULL)
            {
                wprintf(L"Failed to allocate memory for command line\n");
                return EXIT_FAILURE;
            }
            swprintf_s(quoted_cmd, len, L"\"%s\"", cmd);

            // Free the memory used by the unquoted command
            argv[0] = quoted_cmd;
        }

        // Call _wexecv with the command string and arguments in separate
        // parameters
        intptr_t result = _wexecv(cmd, argv);

        // Free the cmd (used to be argv[0])
        free(cmd);

        // Free the array of arguments
        LocalFree(argv);

        if (result == -1)
        {
            perror("_wexecv");
            return EXIT_FAILURE;
        }

        exit(EXIT_SUCCESS);
    }

} // namespace
#endif

void check_language(PreferencesUI* uiPrefs, int& language_index)
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

            Fl_Preferences base(mrv::prefspath().c_str(), "filmaura", "mrv2");

            // Save ui preferences
            Fl_Preferences ui(base, "ui");
            ui.set("language_code", language);

            base.flush();

#ifdef _WIN32
            win32_execv();
#else
            std::string root = mrv::rootpath();
            root += "/bin/mrv2";

            const char* const parmList[] = {root.c_str(), NULL};
            execv(root.c_str(), (char* const*)parmList);
#endif
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
            win32_execv();
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

        Fl_Preferences base(mrv::prefspath().c_str(), "filmaura", "mrv2");

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
            // THis is for Apple mainly, as it we just set LC_MESSAGES only
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

        try
        {
            // This is broken.
            // fs::path::imbue(std::locale());
        }
        catch (const std::runtime_error& e)
        {
            LOG_ERROR(e.what());
        }

        std::string path = mrv::rootpath();
        path += "/share/locale/";

        char buf[256];
        sprintf(buf, "mrv2-v%s", mrv::version());
        bindtextdomain(buf, path.c_str());
        bind_textdomain_codeset(buf, "UTF-8");
        textdomain(buf);

        const std::string msg =
            tl::string::Format(_("Set Language to {0}, Numbers to {1}"))
                .arg(language)
                .arg(numericLocale);

        return msg;
    }

} // namespace mrv
