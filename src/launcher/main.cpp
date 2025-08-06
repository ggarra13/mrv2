#include <cstdlib>
#include <unistd.h>
#include <stdio.h>
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
    g_bin_path = parent.string();
}

int main(int argc, char* argv[])
{
    // Path to the actual binary (relative to Contents/MacOS/)
    // const char* real_binary = "./vmrv2.sh";
    set_root_path(argc, argv);

    std::string binary = "vmrv2.sh";
    std::string full_path = g_bin_path + "/" + binary;

    std::cout << "Launching " << full_path << std::endl;

    // Execute the real app binary, replacing the stub process
    execv(full_path.c_str(), argv);

    // If execv returns, there was an error
    perror("execv failed");
    return 1;
}
