// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/filename.H>
#include <FL/fl_utf8.h>

#include "mrvHome.h"
#include "mrvFile.h"
#include "mrvOS.h"
#include "mrvRoot.h"
#include "mrvString.h"


#include <stdlib.h>
#include <filesystem>
#include <cstring>

namespace fs = std::filesystem;

#if defined(_WIN32) && !defined(_WIN64_)
#    include <windows.h>
#else
#    include <unistd.h>
#    include <sys/types.h>
#    include <pwd.h>
#endif

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
            return "../Resources/";
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
            path += fl_getenv("HOMEPATH");
            path += "/";
            path += fl_getenv("USERNAME");
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
    
    std::string studiopath()
    {
        const char* c = fl_getenv("MRV2_STUDIOPATH");
        if (!c || strlen(c) == 0)
            c = fl_getenv("STUDIOPATH");
        if (!c || strlen(c) == 0)
            return "";
        std::string r = c;
        r += "/.filmaura/";
        return r;
    }

    std::string prefspath()
    {
        std::string prefs = mrv::homepath();
        prefs += "/.filmaura/";
        return prefs;
    }


} // namespace mrv
