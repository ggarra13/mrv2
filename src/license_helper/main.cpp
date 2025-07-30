

#include <FL/fl_utf8.h>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Output.H>
#include <FL/Fl_Preferences.H>

#if defined(_WIN32)
#  include <windows.h>
#else // Linux/Unix
#  include <unistd.h>
#  include <sys/types.h>
#  include <pwd.h>
#endif

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
#if defined(_WIN32)
    char buffer[128];
    FILE* pipe = _popen("wmic csproduct get uuid", "r");
    if (!pipe) return "";
    fgets(buffer, sizeof(buffer), pipe); // skip header
    fgets(buffer, sizeof(buffer), pipe); // actual UUID
    _pclose(pipe);
    return std::string(buffer);
#elif defined(__APPLE__)
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(
                                                      "ioreg -rd1 -c IOPlatformExpertDevice | grep IOPlatformUUID | cut -d '\"' -f4", "r"), pclose);
    if (pipe) {
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
    }
    return result;
#else
    std::ifstream f("/etc/machine-id");
    std::string id;
    std::getline(f, id);
    return id;
#endif
}

static void exit_cb(Fl_Widget* b, void* data)
{
    exit(1);
}

std::string homepath()
{
    std::string path;

#ifdef _WIN32
    char* e = nullptr;
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
        const char* var = fl_getenv("HOMEPATH");
        if (var)
        {
            path += var;
            var = fl_getenv("USERNAME");
            if (var)
            {
                path += "/";
                path += var;
            }
        }
        if (fs::is_directory(path))
            return path;
    }
#else
    char* e = nullptr;
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
    return ".";
}

std::string prefspath()
{
    std::string prefs = homepath();
    prefs += "/.filmaura/";
    return prefs;
}


static void create_license_cb(Fl_Widget* b, void* data)
{
    Fl_Input* license_widget = (Fl_Input*)data;
    std::string license = license_widget->value();
    if (license.empty())
    {
        return;
    }

    Fl_Preferences base(
            prefspath().c_str(), "filmaura", "mrv2.license",
            (Fl_Preferences::Root)0);
    base.set("license", license.c_str());
    base.flush();
    
    exit(0);
}

int main(int argc, char** argv)
{
    Fl_Double_Window win(640, 480, "License helper");
    const std::string& machine_id = get_machine_id();

    win.begin();

    Fl_Box    box(20, 30, 600, 80);
    box.label("Please submit this machine id "
              "with your Paypal email to ggarra13@@gmail.com");

    Fl_Output machine(20, 130, 600, 40, "Machine ID");
    machine.align(FL_ALIGN_CENTER | FL_ALIGN_TOP);
    machine.value(machine_id.c_str());

    Fl_Input license(20, 230, 600, 40, "License");
    license.align(FL_ALIGN_CENTER | FL_ALIGN_TOP);
    license.tooltip("Once you obtain a license, copy it here");

    Fl_Button exit(80, 280, 150, 40, "Exit");
    exit.callback((Fl_Callback*)exit_cb, nullptr);

    Fl_Button create(360, 280, 150, 40, "Create");
    create.callback((Fl_Callback*)create_license_cb, &license);

    
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
