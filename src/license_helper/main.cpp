
#include "mrvOS.h"


#include <FL/fl_utf8.h>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Preferences.H>

#if defined(_WIN32)
#  include <windows.h>
#  include <winreg.h>
#else // Linux/Unix
#  include <unistd.h>
#  include <sys/types.h>
#  include <pwd.h>
#endif

#include <algorithm>
#include <array>
#include <cstdlib>
#include <memory>
#include <filesystem>
namespace fs = std::filesystem;
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

std::string get_machine_id() {
    std::string out;
        
#if defined(_WIN32)
#  if defined(_M_X64) || defined(_M_AMD64)
    std::string errors;
    mrv::os::exec_command("wmic csproduct get uuid", out, errors);
    size_t pos = out.find("\r\n");
    if (pos != std::string::npos)
    {
        out = out.substr(pos + 2);
    }
#  else
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                      "SOFTWARE\\Microsoft\\Cryptography",
                      0, KEY_READ, &hKey) == ERROR_SUCCESS)
    {
        char value[256];
        DWORD value_length = sizeof(value);
        if (RegGetValueA(hKey, nullptr, "MachineGuid",
                         RRF_RT_REG_SZ, nullptr,
                         &value, &value_length) == ERROR_SUCCESS)
        {
            out = value;
        }
        RegCloseKey(hKey);
    }
#  endif
#elif defined(__APPLE__)
    std::array<char, 128> buffer;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(
                                                      "ioreg -rd1 -c IOPlatformExpertDevice | grep IOPlatformUUID | cut -d '\"' -f4", "r"), pclose);
    if (pipe) {
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            out += buffer.data();
        }
    }
#else
    std::ifstream f("/etc/machine-id");
    std::getline(f, out);
#endif
    out.erase(remove(out.begin(), out.end(), '\n'), out.end());
    out.erase(remove(out.begin(), out.end(), '\r'), out.end());
    out.erase(remove(out.begin(), out.end(), ' '), out.end());
    return out;
}

static void exit_cb(Fl_Widget* b, void* data)
{
    exit(1);
}

Fl_Input* license;


int main(int argc, char** argv)
{
    Fl_Double_Window win(640, 380, "License helper");
    std::string machine_id = get_machine_id();

    win.begin();

    Fl_Box    box(20, 30, 600, 80);
    box.label("Please submit this machine id "
              "with your Paypal donation of\nUSD $50 (yearly license) or\n"
              "USD $150 (license to own) to ggarra13@@gmail.com");

    Fl_Output machine(20, 160, 600, 40, "Machine ID");
    machine.align(FL_ALIGN_CENTER | FL_ALIGN_TOP);
    machine.value(machine_id.c_str());

    Fl_Button exit(180, 300, 250, 40, "Demo");
    exit.callback((Fl_Callback*)exit_cb, nullptr);


    
    win.end();               
    win.show();
    return Fl::run();
}

#if defined(_WIN32) && defined(_MSC_VER)

#include <stdio.h>

#include <FL/fl_utf8.h>
#include <FL/fl_string_functions.h>

int WINAPI WinMain(
    HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    int rc;
    int i;
    int argc;
    char** argv;

    /*
     * If we are compiling in debug mode, open a console window so
     * we can see any printf's, etc...
     *
     * While we can detect if the program was run from the command-line -
     * look at the CMDLINE environment variable, it will be "WIN" for
     * programs started from the GUI - the shell seems to run all Windows
     * applications in the background anyways...
     */

    /* Convert the command line arguments to UTF-8 */
    LPWSTR* wideArgv = CommandLineToArgvW(GetCommandLineW(), &argc);

    /* Allocate an array of 'argc + 1' string pointers */
    argv = (char **)malloc((argc + 1) * sizeof(char *));
  
    /* Convert the command line arguments to UTF-8 */
    for (i = 0; i < argc; i++) {
        /* find the required size of the buffer */
        int u8size = WideCharToMultiByte(CP_UTF8,     /* CodePage */
                                         0,           /* dwFlags */
                                         wideArgv[i], /* lpWideCharStr */
                                         -1,          /* cchWideChar */
                                         NULL,        /* lpMultiByteStr */
                                         0,           /* cbMultiByte */
                                         NULL,        /* lpDefaultChar */
                                         NULL);       /* lpUsedDefaultChar */
        if (u8size > 0) {
            char *strbuf = (char *)malloc(u8size);
            int ret = WideCharToMultiByte(CP_UTF8,     /* CodePage */
                                          0,           /* dwFlags */
                                          wideArgv[i], /* lpWideCharStr */
                                          -1,          /* cchWideChar */
                                          strbuf,      /* lpMultiByteStr */
                                          u8size,      /* cbMultiByte */
                                          NULL,        /* lpDefaultChar */
                                          NULL);       /* lpUsedDefaultChar */
            if (ret) {
                argv[i] = strbuf;
            } else {
                argv[i] = _strdup("");
                free(strbuf);
                fprintf(stderr, "Failed to convert arg %d\n", i);
            }
        } else {
            argv[i] = _strdup("");
        }
    }
    argv[argc] = NULL; /* required by C standard at end of list */

    /* Free the wide character string array */
    LocalFree(wideArgv);

    /* Call the program's entry point main() */
    rc = main(argc, argv);

    /* Cleanup allocated memory for argv */
    for (int i = 0; i < argc; ++i)
    {
        free((void*)argv[i]);
    }
    free((void*)argv);

    return rc;
}

#endif
