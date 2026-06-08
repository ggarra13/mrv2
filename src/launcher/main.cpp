#include <FL/platform.H>

#include <FL/Fl.H>
#include <FL/fl_ask.H> // For error reporting
#include <unistd.h>     // For execv
#include <vector>       // To collect arguments
#include <string>       // For storing file paths
#include <cstdio>       // For perror

#include <cstdlib>
#include <unistd.h>
#include <iostream>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;

#ifdef __APPLE__ /* assume this is OSX */
#    include <sys/param.h>

/* _NSGetExecutablePath : must add -framework CoreFoundation to link line */
#    include <mach-o/dyld.h>

#    ifndef PATH_MAX
#        define PATH_MAX MAXPATHLEN
#    endif
#endif /* APPLE */

/*
 * Mechanism to handle determining *where* the exe actually lives
 */
int get_app_path(char* pname, size_t pathsize)
{
    long result;
    /*
      from http://www.hmug.org/man/3/NSModule.html

      extern int _NSGetExecutablePath(char *buf, unsigned long
      *bufsize);

      _NSGetExecutablePath  copies  the  path  of the executable
      into the buffer and returns 0 if the path was successfully
      copied  in the provided buffer. If the buffer is not large
      enough, -1 is returned and the  expected  buffer  size  is
      copied  in  *bufsize.  Note that _NSGetExecutablePath will
      return "a path" to the executable not a "real path" to the
      executable.  That  is  the path may be a symbolic link and
      not the real file. And with  deep  directories  the  total
      bufsize needed could be more than MAXPATHLEN.
    */
    int status = -1;
    char* given_path = (char*)malloc(MAXPATHLEN * 2);
    if (!given_path)
        return status;

    uint32_t pathSize = MAXPATHLEN * 2;
    result = _NSGetExecutablePath(given_path, &pathSize);
    if (result == 0)
    { /* OK, we got something - now try and resolve the real path...
       */
        if (realpath(given_path, pname) != NULL)
        {
            if ((access(pname, 0) == 0))
                status = 0; /* file exists, return OK */
        }
    }
    free(given_path);
    return status;
}

// Store root path of mrv2's (Installation Directory)
std::string g_bin_path;

void set_root_path(const int argc, char** argv)
{
    char binpath[PATH_MAX];
    binpath[0] = 0;

    int ok = get_app_path(binpath, PATH_MAX);
    if (ok != 0)
    {
        if (argc >= 1)
            strcpy(binpath, argv[0]);
    }

    fs::path rootdir(binpath);
    fs::path parent = rootdir.parent_path(); // skip executable
    g_bin_path = parent.u8string();
}


// --- Configuration ---

// 1. Global vector to collect file paths
// We use std::string to safely copy the const char* from the callback.
std::vector<std::string> g_opened_files;

/**
 * 2. The 'fl_open_callback' function.
 * This function will be called by FLTK for EACH file that is
 * opened with the application (e.g., dropped on the icon).
 */
void open_file_callback(const char* filepath) {
    if (filepath) {
        g_opened_files.push_back(filepath);
    }
}

int main(int argc, char* argv[])
{
    // Path to the actual binary (relative to Contents/MacOS/)
    set_root_path(argc, argv);
    
    std::string script = "launcher.sh";
    std::string full_path = g_bin_path + "/" + script;


    if (!fs::exists(full_path))
    {
        std::cerr << "Could not locate " << full_path
                  << std::endl;
        return 1;
    }

    
    // 3. Set the callback.
    // This MUST be done before initializing the display.
    fl_open_callback(open_file_callback);

    // 4. Initialize the connection to the OS windowing system.
    // This is necessary to receive Apple Events but does not show a window.
    fl_open_display();

    // 5. Process any pending system events.
    // This single call will dispatch any queued "open document" events,
    // firing our 'open_file_callback' for each file.
    Fl::check();

    // 6. Build the new argument list for execv
    std::vector<const char*> new_argv;

    // By convention, argv[0] is the program name (path)
    new_argv.push_back(strdup(full_path.c_str()));

    // Add all original command-line arguments (skipping argv[0], our launcher's name)
    for (int i = 1; i < argc; i++) {
        new_argv.push_back(argv[i]);
    }

    // Add all the files collected from the 'fl_open_callback'
    for (const std::string& file : g_opened_files) {
        new_argv.push_back(file.c_str());
    }

    // The argv array for execv MUST be terminated with a NULL pointer
    new_argv.push_back(NULL);

    // 7. Call execv to replace this launcher with the real program
    // We use .data() to get the underlying C-style array from the vector.
    execv(full_path.c_str(), (char* const*)new_argv.data());

    // 8. Error handling
    // If execv returns, it means it failed.
    char error_msg[1024];
    snprintf(error_msg, sizeof(error_msg), 
             "Launcher Error:\nFailed to execute:\n%s", full_path.c_str());
    
    // Use fl_alert to show a graphical error if the execv fails
    fl_alert("%s\n\nError: %s", error_msg, strerror(errno));
    
    return 1;
}
