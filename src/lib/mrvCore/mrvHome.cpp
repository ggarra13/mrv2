// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <stdlib.h>
#include <filesystem>
namespace fs = std::filesystem;

#include <FL/filename.H>
#include <FL/fl_utf8.h>

#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvOS.h"
#include "mrvCore/mrvRoot.h"
#include "mrvCore/mrvString.h"

#if defined(_WIN32) && !defined(_WIN64_)
#    include <windows.h>
#else
#    include <unistd.h>
#    include <sys/types.h>
#    include <pwd.h>
#endif

#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "env ";
}

namespace mrv
{

    std::string username()
    {
        const char* user = fl_getenv("USER");
        if (!user)
        {
            user = fl_getenv("USERNAME");
            if (!user)
            {
                user = "Unknown";
            }
        }
        return user;
    }

    std::string rootpath()
    {
       // If not initialized (as a fallback), you might still use a default.
        // But with this approach, it should always be initialized correctly.
        if (g_root_path.empty()) {
            // This block should ideally never be hit.
            // Add logging here if it is.
            LOG_ERROR("Root path not set");
            return "..";
        }
        return g_root_path;
    }

    std::string tmppath()
    {
        char* e = nullptr;
        std::string path;
        if ((e = fl_getenv("TMP")))
        {
            path = e;
            if (fs::is_directory(path))
                return path;
        }
        if ((e = fl_getenv("TEMP")))
        {
            path = e;
            if (fs::is_directory(path))
                return path;
        }
        path = "/usr/tmp";
        if (fs::is_directory(path))
            return path;

        path = "/tmp";
        return path;
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
            path += os::sgetenv("HOMEPATH");
            path += "/" + os::sgetenv("USERNAME");
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

        path = tmppath();
        return path;
    }

    std::string iconpath()
    {
        std::string iconroot = rootpath();
        iconroot += "/icons/";
        return iconroot;
    }

    std::string studiopath()
    {
        const char* c = fl_getenv("STUDIOPATH");
        if (!c)
            return "";
        std::string r = c;
        if (r.substr(r.size() - 1, r.size()) != "/")
            r += "/";
        return r;
    }

    std::string prefspath()
    {
        std::string prefs = mrv::homepath();
        prefs += "/.filmaura/";
        return prefs;
    }

    std::string pythonpath()
    {
        std::string path = mrv::rootpath();
        path += "/python/demos/";
        return path;
    }

    std::string presetspath()
    {
        std::string path = studiopath() + "/presets/";
        if (!fs::is_directory(path))
            path = mrv::rootpath() + "/presets/";
        return path;
    }

    std::vector<std::string> python_plugin_paths()
    {
        std::vector<std::string> out;
        const char* c = fl_getenv("MRV2_PYTHON_PLUGINS");
        if (!c)
            return out;

#ifdef _WIN32
        out = string::split(c, ';');
#else
        out = string::split(c, ':');
#endif
        return out;
    }

    const char* docs_list[] = {"en", "es", nullptr};

    std::string docspath()
    {
        std::string docs;

        const char* language = getenv("LANGUAGE");
        if (!language || strlen(language) == 0)
            language = "en";

        // Just the language code "es" without the "es_AR.UTF-8" if any.
        std::string code = language;
        code = code.substr(0, 2);

        bool found = false;
        const char** d = docs_list;
        for (; *d; ++d)
        {
            if (code == *d)
            {
                found = true;
                break;
            }
        }
        if (!found)
            code = "en";

        std::string local_docs =
            mrv::rootpath() + "/docs/" + code + "/index.html";
        if (file::isReadable(local_docs))
        {
            docs = "file://" + local_docs;
        }
        else
        {
            std::string online_docs =
                "mrv2.sourceforge.io/docs/" + code + "/index.html";
            docs = "https://" + online_docs;
        }
        return docs;
    }

    //! Path to NDI (if installed)
    std::string NDI_library()
    {
#ifdef _WIN32
        const std::string library = "Processing.NDI.Lib.x64.dll";
#endif
#ifdef __linux__
        const std::string library = "libndi.so";
#endif
#ifdef __APPLE__
        const std::string library = "libndi.dylib";
#endif
        std::string libpath = rootpath() + "/lib/";
        std::string fullpath = libpath + library;
        if (!file::isReadable(fullpath))
        {
            libpath = os::sgetenv("NDI_RUNTIME_DIR_V6");
            if (!libpath.empty())
            {
                fullpath = libpath + library;
            }
            else
            {
                libpath = "/usr/local/lib/";
                fullpath = libpath + library;
                if (!file::isReadable(fullpath))
                {
                    fullpath = "";
                    LOG_ERROR("NDI was not found.  "
                              "Please download it from "
                              "http://ndi.link/NDIRedistV6");
                }
            }
        }

        return fullpath;
    }

} // namespace mrv
