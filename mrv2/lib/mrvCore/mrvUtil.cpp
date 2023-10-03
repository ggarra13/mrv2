// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <algorithm>
#include <filesystem>
namespace fs = std::filesystem;

#include <tlIO/System.h>

#include "mrvCore/mrvUtil.h"
#include "mrvCore/mrvSequence.h"

#include "mrvApp/App.h"

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
    bool is_valid_movie(
        const std::string& ext,
        const std::shared_ptr<tl::system::Context>& context)
    {
        std::string extension = tl::string::toLower(ext);
        if (extension[0] != '.')
            extension = '.' + extension;

        auto ioSystem = context->getSystem<tl::io::System>();
        return (ioSystem->getFileType(extension) == tl::io::FileType::Movie);
    }

    // Given a frame extension, return true if a possible audio file.
    bool is_valid_audio(
        const std::string& ext,
        const std::shared_ptr<tl::system::Context>& context)
    {
        std::string extension = tl::string::toLower(ext);
        if (extension[0] != '.')
            extension = '.' + extension;

        auto ioSystem = context->getSystem<tl::io::System>();
        return (ioSystem->getFileType(extension) == tl::io::FileType::Audio);

        return false;
    }

    // Given a frame extension, return true if a possible audio file.
    bool is_valid_subtitle(const std::string& ext)
    {
        std::string tmp = ext;

        std::string extension = tl::string::toLower(ext);
        if (extension[0] != '.')
            extension = '.' + extension;

        // @todo: add subtitle support to tlRender.
        if (extension == ".srt" || extension == ".sub" || extension == ".ass" ||
            extension == ".vtt")
            return true;

        return false;
    }

    // Given a frame extension, return true if a possible movie file.
    bool is_valid_movie(const std::string& ext)
    {
        auto context = App::app->getContext();
        return is_valid_movie(ext, context);
    }

    // Given a frame extension, return true if a possible audio file.
    bool is_valid_audio(const std::string& ext)
    {
        auto context = App::app->getContext();
        return is_valid_audio(ext, context);
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

    void parse_directory(
        const std::string& dir, std::vector<std::string>& movies,
        std::vector<std::string>& sequences, std::vector<std::string>& audios)
    {

        // default constructor yields path iter. end
        std::vector<std::string> files;
        fs::directory_iterator e;
        for (fs::directory_iterator i(dir); i != e; ++i)
        {
            if (!fs::exists(*i) || fs::is_directory(*i))
                continue;

            std::string file = (*i).path().string();
            files.push_back(file);
        }
        std::sort(files.begin(), files.end());

        std::string root, frame, view, ext;
        SequenceList tmpseqs;

        for (const auto& file : files)
        {
            bool ok = mrv::split_sequence(root, frame, view, ext, file);
            if (is_valid_movie(ext))
                movies.push_back(file);
            else if (is_valid_audio(ext))
                audios.push_back(file);
            else if (ok)
            {
                Sequence s;
                s.root = root;
                s.view = view;
                s.number = frame;
                s.ext = ext;

                tmpseqs.push_back(s);
            }
        }

        //
        // Then, sort sequences and collapse them into a single file entry
        //
        std::sort(tmpseqs.begin(), tmpseqs.end(), SequenceSort());

        std::string first;
        std::string number;
        int padding = -1;

        for (const auto& i : tmpseqs)
        {

            const char* s = i.number.c_str();
            int z = 0;
            for (; *s == '0'; ++s)
                ++z;

            if (i.root != root || i.view != view || i.ext != ext ||
                (padding != z && z != padding - 1))
            {
                // New sequence
                root = i.root;
                padding = z;
                number = first = i.number;
                view = i.view;
                ext = i.ext;

                std::string file = root;
                file += first;
                file += view;
                file += ext;
                sequences.push_back(file);
            }
            else
            {
                padding = z;
                number = i.number;
            }
        }
    }

} // namespace mrv
