
#define _(x) x  // for now

#include "mrvFile.h"
#include "mrvMainWindow.h"
#include "mrvOS.h"

#include <FL/fl_ask.H>
#include <FL/filename.H>
#include <FL/fl_utf8.h>
#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Flex.H>
#include <FL/Fl_Group.H>
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

std::string licensepath()
{
    std::string out;
    const char* c = fl_getenv("MRV2_LICENSEPATH");
    if (!c || strlen(c) == 0)
    {
        out = homepath();
        out += "/.filmaura/";
    }
    else
    {
        out = c;
    }
    return out;
}

static void donate_cb(Fl_Widget* b, void* data)
{
    fl_alert(_("Check your browser for a valid donation.  Send your machine id in the comment."));
    
    fl_open_uri("https://www.paypal.com/donate/?hosted_button_id=PSYEULZG24QHY");
    fl_alert(_("If you submitted a valid donation, you will get a message in the mail activating your license plan."));
}

static void exit_cb(Fl_Widget* b, void* data)
{
    exit(1);
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

    std::string prefs_file = licensepath() + "/mrv2_licenses.lic";

    std::ofstream s(prefs_file);
    s << key << std::endl;

    fl_alert("Saved master key file to %s\n\nMove it some place safe and set the environment variable MRV2_LICENSEPATH pointing to it on each machine.", prefs_file.c_str());
    
    exit(0);
}


int main(int argc, char** argv)
{
    const std::string machine_id = get_machine_id();
    
    MainWindow win(640, 660, _("License helper"));
    win.begin();

    Fl_Tabs*    tabs = new Fl_Tabs(20, 30, 600, win.h()-30);
    tabs->labelsize(20);
    tabs->selection_color(FL_CYAN);

    Fl_Flex* node_locked = nullptr; // message, plans, info
       Fl_Flex* message = nullptr;
          Fl_Box* intro = nullptr;
             Fl_Flex* prices = nullptr;
               Fl_Flex* plans = nullptr;
               Fl_Box* listed = nullptr; // good so far
         Fl_Box* internet = nullptr;
      Fl_Flex* what = nullptr;    
        Fl_Box* plan_type = nullptr;
        Fl_Box* plan_features = nullptr;
     Fl_Flex* info = nullptr;
         Fl_Flex* buttons = nullptr;
        
    { // node_locker
        node_locked = new Fl_Flex(20, 60, 600, tabs->h() - 90,
                                  _("Node-Locked License"));
        
        { // node_locked contents
            message = new Fl_Flex(20, 60, 600, 440);
            message->box(FL_FLAT_BOX);
            message->begin();

            Fl_Box*  intro = new Fl_Box(20, 0, 600, 40);
            intro->labelfont(FL_HELVETICA);
            intro->label(_("To unlock features, please submit the machine id below with your Paypal donation of:"));
            intro->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
            message->fixed(intro, intro->h());

            { // prices
                prices = new Fl_Flex(20, 40, 600, 380);


                {
                    plans = new Fl_Flex(20, prices->y(), 600, 140);
                    plans->type(Fl_Flex::HORIZONTAL);
                    plans->margin(190, 0, 190, 0);
                    plans->gap(5);
                    plans->box(FL_ENGRAVED_BOX);
                
                    Fl_Box* usd = new Fl_Box(20, prices->y(), 300, prices->h());
                    usd->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
                    usd->label(R"TEXT(
USD $ 25
USD $ 50
USD $ 75
USD $150
USD $300)TEXT");
                    Fl_Box* type = new Fl_Box(320, prices->y(), 300, prices->h());
                    type->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
                    type->label(R"TEXT(
Solo
Standard
Edit
Pro
Pro+ (Monthly)
)TEXT");
                    plans->end();
                }
                
                Fl_Box*    listed = new Fl_Box(20, 0, 600, 30);
                listed->label(_("Only the listed amounts unlock features; all others are regular donations."));
                listed->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
                
                prices->fixed(plans, plans->h());
                prices->fixed(listed, listed->h());

                what = new Fl_Flex(20, 0, 600, 120);
                what->box(FL_FLAT_BOX);
                what->type(Fl_Flex::HORIZONTAL);
                what->margin(80, 0, 190, 0);
                what->gap(5);
                
                plan_type = new Fl_Box(20, 20, 300, 100);
                plan_type->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
                plan_type->label(R"TEXT(
Solo
Standard
Edit
Pro
Pro+ (Monthly)
)TEXT");
                plan_features = new Fl_Box(320, 20, 300, 100);
                plan_features->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
                plan_features->label(_(R"TEXT(
Annotations and Saving only for a year
Solo + OpenEXR layers + Python
Standard + Editing
Edit + Voice Annotations
Pro + change your machine[s] as you want
)TEXT"));
                what->end();
                prices->fixed(what, what->h());
            } // what
            
            internet = new Fl_Box(20, 0, 600, 80);
            internet->box(FL_FLAT_BOX);
            internet->label(R"TEXT(Your email will be added to mrv2's Internet database.
You need an Internet connection to use.)TEXT");
            internet->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
            
            prices->gap(5);
            message->fixed(internet, internet->h());
            message->end();


            {
                info = new Fl_Flex(0, 0, 0, 120);
                info->type(Fl_Flex::VERTICAL);
                info->box(FL_ENGRAVED_BOX);

                Fl_Output* machine = new Fl_Output(20, 0, 600, 40, "Machine ID");
                machine->align(FL_ALIGN_CENTER | FL_ALIGN_TOP);
                machine->value(machine_id.c_str());

                buttons = new Fl_Flex(180, 0, 100, 40);
                buttons->type(Fl_Flex::HORIZONTAL);
                buttons->gap(10);
        
                Fl_Button* demo = new Fl_Button(180, 280, 100, 40, "Demo");
                demo->callback((Fl_Callback*)exit_cb, nullptr);
                
                Fl_Button* donate = new Fl_Button(300, 280, 100, 40, "Donate");
                donate->callback((Fl_Callback*)donate_cb, nullptr);
                
                buttons->end();
                info->fixed(machine, machine->h());
                info->end();
            }
         
            node_locked->margin(10, 10, 10, 10);
            node_locked->gap(10);
            node_locked->fixed(message, message->h());
            node_locked->fixed(info, info->h());
            node_locked->end();
    } // node_locked contents
            

    }

    {
        Fl_Group* floating = new Fl_Group(20, 60, 600-20, 320,
                                          "Floating Licenses");
        
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

    {
        
        Fl_Flex* custom_dev = new Fl_Flex(20, 60, 600, tabs->h() - 30,
                                           _("Custom Development"));
        custom_dev->box(FL_ENGRAVED_BOX);
        Fl_Flex* custom_msg = new Fl_Flex(20, 60, 600, 400);

        Fl_Box*  intro = new Fl_Box(20, 0, 600, 40);
        intro->labelfont(FL_HELVETICA);
        intro->label(_(R"TEXT(For custom development:)TEXT"));
        intro->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
                   
        Fl_Box*    prices = new Fl_Box(20, 0, 600, 360);
        prices->box(FL_ENGRAVED_BOX);
        prices->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
        prices->label(_(R"TEXT(
USD $  5000
for Remote - Open Source Development without Hardware (I may fail).

USD $ 10000
for Remote - Open Source Development With Hardware.

USD $ 40000
for Remote - Closed Source Custom Development.

USD $100000
for Buying the source as is and stop its Open Source Development.

Contact ggarra13@@gmail.com)TEXT"));


        custom_msg->gap(20);
        custom_msg->fixed(intro, intro->h());
        custom_msg->fixed(prices, prices->h());
        custom_msg->end();
        
        custom_dev->margin(10, 10, 10, 10);
        custom_dev->gap(5);
        custom_dev->fixed(custom_msg, custom_msg->h());
        custom_dev->end();
    }

    tabs->end();
    
    win.callback((Fl_Callback*)exit_cb, nullptr);


    
    win.end();               
    win.show();
    win.always_on_top();
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
