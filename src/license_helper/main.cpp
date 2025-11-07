
#include "mrvFile.h"
#include "mrvOS.h"


#include <FL/fl_ask.H>
#include <FL/fl_utf8.h>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Multiline_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Preferences.H>
#include <FL/Fl_Tabs.H>

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
    //
    // Due to legacy issues, on AMD64 we relied on wmic for the license.
    //
    const std::string wmic_exe = "C:/Windows/System32/wbem/WMIC.exe";
    if (mrv::file::isReadable(wmic_exe))
    {
        std::string errors;
        try
        {
            mrv::os::exec_command("wmic csproduct get uuid", out,
                                  errors);
            size_t pos = out.find("\r\n");
            if (pos != std::string::npos)
            {
                out = out.substr(pos + 2);
            }
        }
        catch(const std::exception& e)
        {
        }
    }
#  endif
    if (out.empty())
    {
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
    }
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


std::string homepath()
{
    std::string path;
    char* e = nullptr;

#ifdef _WIN32
    if ((e = fl_getenv("HOME")))
    {
        path = e;
        size_t pos = path.rfind("Documents");
        if (pos != std::string::npos)
        {
            path = path.replace(pos, path.size(), "");
        }
        if (fs::is_directory(path))
            return path;
    }
    if ((e = fl_getenv("USERPROFILE")))
    {
        path = e;
        if (fs::is_directory(path))
            return path;
    }
    if ((e = fl_getenv("HOMEDRIVE")))
    {
        path = e;
        e = fl_getenv("HOMEPATH");
        if (e)
            path += e;
        path += "/";
        e = fl_getenv("USERNAME");
        if (e)
            path += e;
        if (fs::is_directory(path))
            return path;
    }
#else
    if ((e = fl_getenv("HOME")))
    {
        path = e;
        size_t pos = path.rfind("Documents");
        if (pos != std::string::npos)
        {
            path = path.replace(pos, path.size(), "");
        }
        if (fs::is_directory(path))
            return path;
    }
    else
    {
        e = getpwuid(getuid())->pw_dir;
        if (e)
        {
            path = e;
            return path;
        }
    }
#endif

    e = fl_getenv("TMP");
    if (e)
    {
        path = e;
    }
    else
    {
        e = fl_getenv("TEMP");
        if (e)
        {
            path = e;
        }
        else
        {
            path = "/var/tmp";
        }
    }
    return path;
}

std::string studiopath()
{
    std::string out;
    const char* c = fl_getenv("MRV2_STUDIOPATH");
    if (!c || strlen(c) == 0)
        c = fl_getenv("STUDIOPATH");
    if (!c || strlen(c) == 0)
        out = homepath();
    else
        out = c;
    out += "/.filmaura/";
    return out;
}

Fl_Multiline_Input* master_key;
    
static void install_cb(Fl_Widget* b, void* data)
{
    std::string key = master_key->value(); 
    if (key.empty())
    {
        fl_alert("Please fill in the master key with the one you got.");
        return;
    }

    std::string prefs_file = studiopath() + "/mrv2_licenses.lic";

    std::ofstream s(prefs_file);
    s << key << std::endl;

    fl_alert("Saved master key file to %s", prefs_file.c_str());
    
    exit(0);
}


static void exit_cb(Fl_Widget* b, void* data)
{
    exit(1);
}

int main(int argc, char** argv)
{
    const std::string machine_id = get_machine_id();
    
    Fl_Double_Window win(640, 380, "License helper");

    win.begin();

    Fl_Tabs*    tabs = new Fl_Tabs(20, 30, 600, 350);
    tabs->labelsize(20);
    tabs->selection_color(FL_YELLOW);

    {
        Fl_Group* node_locked = new Fl_Group(20, 60, 600-20, 320,
                                             "Node-Locked License"); 
        Fl_Box*    box = new Fl_Box(20, 70, 600, 80);
        box->label("Please submit this machine id "
                   "with your Paypal donation of\nUSD $50 (yearly license) or\n"
                   "USD $150 (license to own) to ggarra13@@gmail.com.\n"
                   "It will be added to mrv2's Internet database.");

        Fl_Output* machine = new Fl_Output(20, 200, 600, 40, "Machine ID");
        machine->align(FL_ALIGN_CENTER | FL_ALIGN_TOP);
        machine->value(machine_id.c_str());
        
        Fl_Button* demo = new Fl_Button(180, 280, 250, 40, "Demo");
        demo->callback((Fl_Callback*)exit_cb, nullptr);
        
        node_locked->end();
    }
    
    {
        Fl_Group* floating = new Fl_Group(20, 60, 600-20, 320,
                                          "Floating License");
        
        Fl_Box*    box = new Fl_Box(20, 60, 580, 80);
        box->label("Please enter your master key for your facility you "
                   "got from ggarra13@@gmail.com");

        master_key = new Fl_Multiline_Input(20, 150, 580, 140, "Master Key");
        master_key->align(FL_ALIGN_CENTER | FL_ALIGN_TOP);
        
        Fl_Button* install = new Fl_Button(340, 300, 250, 40, "Install");
        install->callback((Fl_Callback*)install_cb, nullptr);
        
        Fl_Button* demo = new Fl_Button(80, 300, 220, 40, "Demo");
        demo->callback((Fl_Callback*)exit_cb, nullptr);
    
        floating->end();
    }

    tabs->end();
    
    win.callback((Fl_Callback*)exit_cb, nullptr);


    
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
