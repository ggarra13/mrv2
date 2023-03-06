// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

#include "mrvUtil.h"
#include "mrvSequence.h"

namespace mrv
{
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

    // Given a frame extension, return true if a possible picture file.
    bool is_valid_picture(const char* ext)
    {
        std::string tmp = ext;
        std::transform(
            tmp.begin(), tmp.end(), tmp.begin(), (int (*)(int))tolower);

        if (tmp == ".iff" || tmp == ".pic" || tmp == ".tif" || tmp == ".tiff" ||
            tmp == ".png" || tmp == ".jpg" || tmp == ".jpeg" || tmp == ".tga" ||
            tmp == ".exr" || tmp == ".dpx" || tmp == ".cin" || tmp == ".bmp" ||
            tmp == ".bit" || tmp == ".sxr" || tmp == ".ct" || tmp == ".sgi" ||
            tmp == ".st" || tmp == ".map" || tmp == ".sxr" || tmp == ".nt" ||
            tmp == ".mt" || tmp == ".psd" || tmp == ".rgb" || tmp == ".rpf" ||
            tmp == ".zt")
            return true;

        return false;
    }

    bool is_valid_sequence(const char* filename)
    {
        if (strlen(filename) == 0)
            return true;
        std::string root, frame, view, ext;
        bool ok = split_sequence(root, frame, view, ext, filename);
        if (ext == "reel" || ext == "otio")
            return false;
        return ok;
    }

    bool is_directory(const char* dir)
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

} // namespace mrv
