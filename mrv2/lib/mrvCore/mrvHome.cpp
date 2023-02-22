// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <stdlib.h>
#include <filesystem>
namespace fs = std::filesystem;

#include <FL/filename.H>
#include <FL/fl_utf8.h>

#include "mrvHome.h"

#if defined(_WIN32) && !defined(_WIN64_)
#    include <windows.h>
#else
#    include <unistd.h>
#    include <sys/types.h>
#    include <pwd.h>
#endif

namespace mrv
{

    std::string sgetenv(const char* const n)
    {
        if (fl_getenv(n))
            return fl_getenv(n);
        else
            return std::string();
    }

    std::string rootpath()
    {
        const char* root = fl_getenv("MRV_ROOT");
        if (!root)
            root = "..";
        return root;
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
            path += sgetenv("HOMEPATH");
            path += "/" + sgetenv("USERNAME");
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
        std::string studio = mrv::studiopath();
        if (fs::is_directory(studio))
        {
            studio += '/';
            return studio;
        }
        std::string prefs = mrv::homepath();
        prefs += "/.filmaura/";
        return prefs;
    }

    std::string lockfile()
    {
        std::string lockfile = mrv::homepath();
        lockfile += "/.filmaura/mrv2.lock.prefs";
        return lockfile;
    }

    std::string shaderpath()
    {
        std::string path = mrv::rootpath();
        path += "/shaders/";
#ifdef TLRENDER_OPENGL
        path += "/opengl/";
#else
        path += "/metal/";
#endif
        return path;
    }

} // namespace mrv
