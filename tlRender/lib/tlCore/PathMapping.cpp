
#include <cstring>

#include <FL/fl_utf8.h>

#include <tlCore/File.h>
#include <tlCore/PathMapping.h>
#include <tlCore/String.h>

#include <iostream>
#include <filesystem>
#include <map>
#include <string>

#ifndef _WIN32
#    include <unistd.h>
#    include <sys/types.h>
#    include <pwd.h>
#endif

namespace fs = std::filesystem;

namespace tl
{
    namespace
    {
        std::string sgetenv(const char* const n)
        {
            if (fl_getenv(n))
                return fl_getenv(n);
            else
                return std::string();
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

        //! Path to user's home directory (without a trailing slash)
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

        std::string prefspath()
        {
            std::string prefs = homepath();
            prefs += "/.filmaura/";
            return prefs;
        }

        std::string add_slash(const std::string& in)
        {
            std::string out = in;
            int len = out.size() - 1;
            if (len < 0)
                return out;
            if (out[len] != '/' && out[len] != '\\')
                out += '/';
            return out;
        }

    } // namespace

    namespace path_mapping
    {
        std::map<std::string, std::string> mapping;

        void init()
        {
            std::string mappingpath = studiopath();
            if (!file::isReadable(mappingpath + "/mrv2.paths"))
                mappingpath = prefspath();

            Fl_Preferences path_mapping(
                mappingpath.c_str(), "filmaura", "mrv2.paths",
                (Fl_Preferences::Root)0);
            char key[256];
            char tmpS[2048];
            int num = path_mapping.entries();
            for (int i = 0; i < num; ++i)
            {
                snprintf(key, 256, "Path #%d", i + 1);
                path_mapping.get(key, tmpS, "", 2048);
                if (strlen(tmpS) == 0)
                    continue;

                auto splitArray = string::split(tmpS, '\t');
                std::string remote = add_slash(splitArray[0]);
                std::string local = add_slash(splitArray[1]);

                mapping.insert(std::make_pair(remote, local));
            }
        }

        bool replace_path(std::string& file)
        {
            if (file.empty())
                return false;

            if (file::isReadable(file))
                return true;

            for (const auto& item : mapping)
            {
                const std::string& remote = item.first;
                const std::string& local = item.second;

                if (file.substr(0, remote.size()) == remote)
                {
                    std::string outFile = file;
                    outFile.replace(0, remote.size(), local);

                    if (file::isReadable(outFile))
                    {
                        file = outFile;
                        return true;
                    }
                }
            }
            return false;
        }
    } // namespace path_mapping
} // namespace tl
