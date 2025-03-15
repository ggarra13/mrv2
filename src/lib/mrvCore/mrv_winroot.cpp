#include <string>
#include <filesystem>
#include <cstdlib>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace fs = std::filesystem;

// Simple logging function (replace with your preferred logging mechanism)
#ifdef _WIN32
static void log_error(const wchar_t* msg) {
    FILE* f = _wfopen(L"error.log", L"a");
    if (f) {
        fwprintf(f, L"%s\n", msg);
        fclose(f);
    }
}
#else
static void log_error(const char* msg) {
    FILE* f = fopen("error.log", "a");
    if (f) {
        fprintf(f, "%s\n", msg);
        fclose(f);
    }
}
#endif

/*
 * Mechanism to handle determining *where* the exe actually lives
 */
#ifdef _WIN32
int get_app_path(wchar_t* pname, size_t pathsize)
{
    // Use a larger buffer for long paths (Windows supports up to 32767 chars)
    DWORD result = GetModuleFileNameW(NULL, pname, DWORD(pathsize));
    if (result == 0 || result >= pathsize) {
        log_error(L"GetModuleFileNameW failed or buffer too small");
        return -1;
    }

    // Check if the file exists
    if (GetFileAttributesW(pname) == INVALID_FILE_ATTRIBUTES) {
        log_error(L"File does not exist or is inaccessible");
        return -1;
    }

    // Replace backslashes with forward slashes
    wchar_t* p = pname;
    while (*p) {
        if (*p == L'\\') *p = L'/';
        p++;
    }

    return 0; // Success
}
#else
int get_app_path(char* pname, size_t pathsize)
{
    long result;

#ifdef __linux__
    pathsize--; // Preserve space for NULL
    result = readlink("/proc/self/exe", pname, pathsize);
    if (result > 0) {
        pname[result] = 0; // Add NULL terminator
        if (access(pname, 0) == 0)
            return 0;
        log_error("Linux: Path exists but access failed");
    }
#elif defined(SOLARIS)
    char* p = getexecname();
    if (p) {
        if (p[0] == '/') {
            strncpy(pname, p, pathsize);
            if (access(pname, 0) == 0)
                return 0;
        } else {
            getcwd(pname, pathsize);
            result = strlen(pname);
            strncat(pname, "/", (pathsize - result));
            result++;
            strncat(pname, p, (pathsize - result));
            if (access(pname, 0) == 0)
                return 0;
            log_error("Solaris: Constructed path invalid");
        }
    }
#elif defined(__APPLE__)
    char* given_path = (char*)malloc(MAXPATHLEN * 2);
    if (!given_path) {
        log_error("macOS: Memory allocation failed");
        return -1;
    }
    uint32_t pathSize = MAXPATHLEN * 2;
    result = _NSGetExecutablePath(given_path, &pathSize);
    if (result == 0) {
        if (realpath(given_path, pname) != NULL) {
            if (access(pname, 0) == 0) {
                free(given_path);
                return 0;
            }
        }
        log_error("macOS: Realpath resolution failed");
    }
    free(given_path);
#else
#    error Unknown OS
#endif
    return -1; // Path lookup failed
}
#endif

#ifdef _WIN32
int setenv(const wchar_t* name, const wchar_t* value, int overwrite)
{
    if (!SetEnvironmentVariableW(name, value)) {
        log_error(L"SetEnvironmentVariableW failed");
        return -1;
    }
    if (_wputenv_s(name, value) != 0) {
        log_error(L"_wputenv_s failed");
        return -1;
    }
    return 0;
}
#else
int setenv(const char* name, const char* value, int overwrite)
{
    return ::setenv(name, value, overwrite);
}
#endif

namespace mrv
{
    void set_root_path(const int argc, char** argv)
    {
#ifdef _WIN32
        // Check existing environment variable
        char* root = fl_getenv(L"MRV2_ROOT");
        if (!root) {
            wchar_t binpath[32767]; // Max path length on Windows
            binpath[0] = L'\0';

            int ok = get_app_path(binpath, 32767);
            if (ok != 0) {
                if (argc >= 1) {
                    // Convert argv[0] from char* to wchar_t*
                    size_t converted;
                    mbstowcs_s(&converted, binpath, argv[0], 32767);
                }
                log_error(L"get_app_path failed, using argv[0]");
            }

            fs::path rootdir(binpath);
            fs::path parent = rootdir.parent_path(); // Skip executable
            rootdir = parent.parent_path();          // Skip bin/ directory

            std::wstring root_str = rootdir.wstring();
            if (setenv(L"MRV2_ROOT", root_str.c_str(), 1) != 0) {
                log_error(L"Failed to set MRV2_ROOT");
            }
        }
#else
        char* root = fl_getenv("MRV2_ROOT");
        if (!root) {
            char binpath[PATH_MAX];
            binpath[0] = 0;

            int ok = get_app_path(binpath, PATH_MAX);
            if (ok != 0) {
                if (argc >= 1)
                    strcpy(binpath, argv[0]);
                log_error("get_app_path failed, using argv[0]");
            }

            fs::path rootdir(binpath);
            fs::path parent = rootdir.parent_path(); // Skip executable
            rootdir = parent.parent_path();          // Skip bin/ directory

            std::string root_str = rootdir.string();
            if (setenv("MRV2_ROOT", root_str.c_str(), 1) != 0) {
                log_error("Failed to set MRV2_ROOT");
            }
        }
#endif
    }
} // namespace mrv
