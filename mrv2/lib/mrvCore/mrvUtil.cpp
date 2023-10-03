// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <algorithm>
#include <filesystem>
namespace fs = std::filesystem;

#include <tlIO/System.h>

#include "mrvUtil.h"
#include "mrvSequence.h"

namespace mrv
{
    bool is_valid_file_type(
        const std::string extension,
        const std::shared_ptr<tl::system::Context>& context)
    {
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

    bool is_valid_file_type(
        const tl::file::Path& path,
        const std::shared_ptr<tl::system::Context>& context)
    {
        std::string extension = tl::string::toLower(path.getExtension());
        return is_valid_file_type(extension, context);
    }

    // Given a frame extension, return true if a possible movie file.
    bool is_valid_movie(const char* ext)
    {
        if (ext == NULL)
            return false;

        std::string tmp = ext;
        std::transform(
            tmp.begin(), tmp.end(), tmp.begin(), (int (*)(int))tolower);
        if (tmp[0] != '.')
            tmp = '.' + tmp;

        if (tmp == ".3gp" || tmp == ".asf" || tmp == ".avc" ||
            tmp == ".avchd" || tmp == ".avi" || tmp == ".divx" ||
            tmp == ".dv" || tmp == ".flv" || tmp == ".m2ts" || tmp == ".m2t" ||
            tmp == ".m4v" || tmp == ".mkv" || tmp == ".mov" || tmp == ".mp4" ||
            tmp == ".mpeg" || tmp == ".mpg" || tmp == ".mvb" || tmp == ".mxf" ||
            tmp == ".ogg" || tmp == ".ogm" || tmp == ".ogv" || tmp == ".qt" ||
            tmp == ".otio" || tmp == ".rm" || tmp == ".ts" || tmp == ".vob" ||
            tmp == ".vp9" || tmp == ".wmv")
            return true;
        return false;
    }

    // Given a frame extension, return true if a possible audio file.
    bool is_valid_audio(const char* ext)
    {
        std::string tmp = ext;
        std::transform(
            tmp.begin(), tmp.end(), tmp.begin(), (int (*)(int))tolower);
        if (tmp[0] != '.')
            tmp = '.' + tmp;

        if (tmp == ".aiff" || tmp == ".flac" || tmp == ".mp3" ||
            tmp == ".ogg" || tmp == ".opus" || tmp == ".snd" ||
            tmp == ".vorbis" || tmp == ".wav")
            return true;

        return false;
    }

    // Given a frame extension, return true if a possible audio file.
    bool is_valid_subtitle(const char* ext)
    {
        std::string tmp = ext;
        std::transform(
            tmp.begin(), tmp.end(), tmp.begin(), (int (*)(int))tolower);
        if (tmp[0] != '.')
            tmp = '.' + tmp;

            // @todo: add subtitle support like mrViewer
#if 0
        if ( tmp == ".srt" ||
             tmp == ".sub" ||
             tmp == ".ass" ||
             tmp == ".vtt" )
            return true;
#endif

        return false;
    }

    bool is_valid_sequence(const std::string& filename)
    {
        std::string root, frame, view, ext;
        bool ok = split_sequence(root, frame, view, ext, filename);
        if (ext == "mrv2s" || ext == "otio" || ext == "prefs")
            return false;
        return ok;
    }

    bool is_directory(const std::string& dir)
    {
        return fs::is_directory(dir);
    }

    int padded_digits(const std::string& frame)
    {
        if (frame == "#" && frame.size() == 1)
            return 4;
        size_t pos;
        if ((pos = frame.find('-')) != std::string::npos)
            return frame.substr(0, pos).size();
        std::string c = frame.substr(0, 1);
        if (c == "@" || c == "#" || c == "0")
            return (int)frame.size();
        if (c == "%")
            return atoi(frame.substr(1, frame.size() - 2).c_str());
        return 1;
    }

    std::string commentCharacter(const std::string& input, const char match)
    {
        std::string result;

        for (char c : input)
        {
            if (c == match)
            {
                result += "\\";
                result += match;
            }
            else
            {
                result += c;
            }
        }

        return result;
    }

} // namespace mrv
