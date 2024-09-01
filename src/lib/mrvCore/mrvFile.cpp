// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <regex>
#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

#include <tlIO/System.h>

#include <FL/Fl.H>

#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvHome.h"

#include "mrvApp/mrvApp.h"

#ifdef _WIN32
#    include <process.h>
#else
#    include <unistd.h> // For access()
#endif

namespace mrv
{
    namespace file
    {
        bool isValidType(const std::string ext)
        {
            std::string extension = tl::string::toLower(ext);
            if (extension[0] != '.')
                extension = '.' + extension;

            auto context = App::app->getContext();
            auto ioSystem = context->getSystem<tl::io::System>();

            bool validFile = false;
            switch (ioSystem->getFileType(extension))
            {
            case tl::io::FileType::Sequence:
            case tl::io::FileType::Movie:
            case tl::io::FileType::Audio:
                validFile = true;
                break;
            default:
                if (".otio" == extension || ".otioz" == extension)
                {
                    validFile = true;
                }
                break;
            }
            return validFile;
        }

        bool isValidType(const tl::file::Path& path)
        {
            std::string extension = tl::string::toLower(path.getExtension());
            return isValidType(extension);
        }

        // Given a frame extension, return true if a possible movie file.
        bool isMovie(const std::string& ext)
        {
            std::string extension = tl::string::toLower(ext);
            if (extension[0] != '.')
                extension = '.' + extension;

            auto context = App::app->getContext();
            auto ioSystem = context->getSystem<tl::io::System>();
            return (
                ioSystem->getFileType(extension) == tl::io::FileType::Movie);
        }

        // Given a frame extension, return true if a possible audio file.
        bool isAudio(const std::string& ext)
        {
            std::string extension = tl::string::toLower(ext);
            if (extension[0] != '.')
                extension = '.' + extension;

            auto context = App::app->getContext();
            auto ioSystem = context->getSystem<tl::io::System>();
            return (
                ioSystem->getFileType(extension) == tl::io::FileType::Audio);

            return false;
        }

        // Given a frame extension, return true if a possible audio file.
        bool isSubtitle(const std::string& ext)
        {
            std::string tmp = ext;

            std::string extension = tl::string::toLower(ext);
            if (extension[0] != '.')
                extension = '.' + extension;

            // @todo: add subtitle support to tlRender.
            if (extension == ".srt" || extension == ".sub" ||
                extension == ".ass" || extension == ".vtt")
                return true;

            return false;
        }

        bool isSequence(const std::string& filename)
        {
            auto context = App::app->getContext();
            auto ioSystem = context->getSystem<tl::io::System>();

            const file::Path path(filename);
            const std::string& extension = path.getExtension();
            switch (ioSystem->getFileType(extension))
            {
            case tl::io::FileType::Movie:
            case tl::io::FileType::Audio:
                return false;
                break;
            default:
                if (extension == ".mrv2s" || extension == ".otio" ||
                    extension == ".prefs" || extension == ".json")
                    return false;
                break;
            };
            const std::string baseName = path.getBaseName();
            // \@todo: add view to tlRender's file::Path class.
            // const std::string view = path.getView();
            const std::string number = path.getNumber();
            bool ok =
                (!baseName.empty() && !number.empty() && !extension.empty());
            return ok;
        }

        bool isDirectory(const std::string& dir)
        {
            return fs::is_directory(dir);
        }

        bool isReadable(const fs::path& p)
        {
            const std::string& filePath = p.generic_string();
            if (filePath.empty())
                return false;

            if (isNetwork(filePath))
                return true;

            std::ifstream f(filePath);
            if (f.is_open())
            {
                f.close();
                return true;
            }

            return false;
        }

        static size_t ndiIndex = 1;

        std::string NDI(ViewerUI* ui)
        {
            char buf[256];
#ifdef _WIN32
            snprintf(buf, 256, "NDI0x%p.%zu.ndi", ui, ndiIndex);
#else
            snprintf(buf, 256, "NDI%p.%zu.ndi", ui, ndiIndex);
#endif
            ++ndiIndex;
            auto out = tmppath() + '/' + buf;
            return out;
        }
        
        bool isUSD(const std::string& ext)
        {
            static const std::string extensions[] =
                {
                    ".usd", ".usda", ".usdc", ".usdz"
                };
            
            for (const std::string& extension : extensions)
            {
                if (ext == extension)
                    return true;
            }
            return false;
        }

        bool isNetwork(const std::string& path)
        {
            static const std::string schemes[] =
                {
                    "crypto", "ftp", "http", "https", "httpproxy",
                    "rtmp", "rtp", "tcp", "tls"
                };

            for (const std::string& scheme : schemes)
            {
                if (path.find(scheme + ":") == 0)
                    return true;
            }
            return false;
        }
        
        bool isTemporaryNDI(const tl::file::Path& path)
        {
            bool out = true;

            const std::string tmpdir = tmppath() + '/';
            auto dir = path.getDirectory();
            auto base = path.getBaseName();
            auto extension = path.getExtension();
            if (dir != tmpdir || base.substr(0, 5) != "NDI0x" ||
                extension != ".ndi")
            {
                out = false;
            }
            return out;
        }

        bool isTemporaryEDL(const tl::file::Path& path)
        {
            const std::string tmpdir = tmppath() + '/';

            auto dir = path.getDirectory();
            auto base = path.getBaseName();
            auto extension = path.getExtension();
            if (dir != tmpdir || base.substr(0, 5) != "EDL0x" ||
                extension != ".otio")
            {
                return false;
            }
            return true;
        }

        bool isOTIO(const tl::file::Path& path)
        {
            auto extension = string::toLower(path.getExtension());
            if (extension != ".otio" && extension != ".otioz")
            {
                return false;
            }
            return true;
        }
        
        bool isInPath(const std::string& command)
        {
#ifdef _WIN32
            char full_path[_MAX_PATH];
            _searchenv(command.c_str(), "PATH", full_path);
            if (*full_path != '\0')
            {
                return true; // Command found in path
            }
            else
            {
                return false; // Command not found
            }
#else
            const char* path = fl_getenv("PATH");
            if (path == NULL)
            {
                return false; // Error: PATH not found
            }

            auto dirs = string::split(path, ':');
            for (const auto& current_dir : dirs)
            {
                char full_path[PATH_MAX];
                snprintf(
                    full_path, PATH_MAX, "%s/%s", current_dir.c_str(),
                    command.c_str());
                if (access(full_path, X_OK) == 0)
                {
                    return true; // Command found
                }
            }
            return false; // Command not found in path
#endif
        }
    } // namespace file

} // namespace mrv
