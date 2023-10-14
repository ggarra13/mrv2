// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <algorithm>
#include <random>
#include <filesystem>
namespace fs = std::filesystem;

#include <tlIO/System.h>

#include "mrvCore/mrvFile.h"
#include "mrvCore/mrvSequence.h"
#include "mrvCore/mrvUtil.h"

#include "mrvApp/App.h"

namespace mrv
{
    std::string randomString()
    {
        static const std::string charset =
            "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        std::string out;

        const int length = 16;
        std::random_device rd;
        std::default_random_engine generator(rd());
        std::uniform_int_distribution<int> distribution(0, charset.size() - 1);

        for (int i = 0; i < length; ++i)
        {
            out += charset[distribution(generator)];
        }
        return out;
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
        std::string out;

        for (char c : input)
        {
            if (c == match)
            {
                out += "\\";
                out += match;
            }
            else
            {
                out += c;
            }
        }

        return out;
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
            file::Path path(file);
            const std::string root = path.getBaseName();
            const std::string frame = path.getNumber();
            const std::string view = ""; // @todo: path.getView();
            const std::string ext = path.getExtension();
            if (file::isMovie(ext))
                movies.push_back(file);
            else if (file::isAudio(ext))
                audios.push_back(file);
            else if (file::isSequence(file))
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
