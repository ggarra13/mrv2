// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <filesystem>
namespace fs = std::filesystem;

#include <tlIO/System.h>

#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvHome.h"

#include "mrvApp/mrvApp.h"

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
            const file::Path path(filename);
            // @todo: add view to tlRender's path.
            const std::string baseName = path.getBaseName();
            // const std::string view = path.getView();
            const std::string number = path.getNumber();
            const std::string ext = path.getExtension();
            if (ext == ".mrv2s" || ext == ".otio" || ext == ".prefs")
                return false;
            bool ok = (!baseName.empty() && !number.empty() && !ext.empty());
            return ok;
        }

        bool isDirectory(const std::string& dir)
        {
            return fs::is_directory(dir);
        }

        bool isReadable(const fs::path& p)
        {
            const std::string fileName = p.generic_string();
            if (fileName.substr(0, 5) == "http:" ||
                fileName.substr(0, 6) == "https:" ||
                fileName.substr(0, 5) == "rtmp:" ||
                fileName.substr(0, 4) == "rtp:")
            {
                return true;
            }

            std::error_code ec; // For noexcept overload usage.
            if (!fs::exists(p, ec) || fileName.empty())
                return false;
            auto perms = fs::status(p, ec).permissions();
            if ((perms & fs::perms::owner_read) != fs::perms::none &&
                (perms & fs::perms::group_read) != fs::perms::none &&
                (perms & fs::perms::others_read) != fs::perms::none)
            {
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
        
    } // namespace file
} // namespace mrv
