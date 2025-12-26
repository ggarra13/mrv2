// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender and feather-tk project.

#include <tlCore/Path.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <algorithm>
#include <array>
#include <list>
#include <sstream>

namespace tl
{
    namespace file
    {
        std::vector<std::string> split(std::filesystem::path path)
        {
            std::list<std::string> out;
            const std::filesystem::path root = path.root_path();
            while (!path.empty() && path != root)
            {
                if (!path.filename().empty())
                {
                    out.push_front(path.filename().u8string());
                }
                path = path.parent_path();
            }
            if (!path.empty())
            {
                out.push_front(path.u8string());
            }
            return std::vector<std::string>(out.begin(), out.end());
        }

        std::string appendSeparator(const std::string& value)
        {
            std::string out = value;

            auto pos = out.find_first_of('/');
            if (pos != std::string::npos)
            {
                if (!out.empty() && out.back() != '/')
                {
                    out.push_back('/');
                }
            }
            else
            {
                pos = out.find_first_of('\\');
                if (pos != std::string::npos)
                {
                    if (!out.empty() && out.back() != '\\')
                    {
                        out.push_back('\\');
                    }
                }
                else
                {
                    if (!out.empty() && out.back() != '/')
                        out.push_back('/');
                }
            }
            return out;
        }

        TLRENDER_ENUM_IMPL(
            UserPath,
            "Home",
            "Desktop",
            "Documents",
            "Downloads");

        std::string toString(int64_t frame, int pad)
        {
            std::stringstream ss;
            if (pad > 0)
            {
                ss << std::setfill('0');
                ss << std::setw(pad);
            }
            ss << frame;
            return ss.str();
        }

        bool PathOptions::operator == (const PathOptions& other) const
        {
            return
                seqNegative == other.seqNegative &&
                seqMaxDigits == other.seqMaxDigits;
        }

        bool PathOptions::operator != (const PathOptions& other) const
        {
            return !(*this == other);
        }

        Path::Path(
            const std::string& value,
            const PathOptions& options) :
            _path(value),
            _options(options)
        {
            _parse(options);
        }

        Path::Path(
            const std::string& dir,
            const std::string& fileName,
            const PathOptions& options) :
            _path(appendSeparator(dir) + fileName),
            _options(options)
        {
            _parse(options);
        }

        void Path::setOptions(const PathOptions& value)
        {
            _options = value;
            _parse(_options);
        }

        void Path::setProtocol(const std::string& value)
        {
            _path = value + getDirectory() + getBaseName() + getNumber() + getExtension() + getRequest();
            const std::optional<math::Int64Range> tmp = _frames;
            _parse(_options);
            _frames = tmp;
        }

        void Path::setDirectory(const std::string& value)
        {
            _path = getProtocol() + value + getBaseName() + getNumber() + getExtension() + getRequest();
            const std::optional<math::Int64Range> tmp = _frames;
            _parse(_options);
            _frames = tmp;
        }

        void Path::setBaseName(const std::string& value)
        {
            _path = getProtocol() + getDirectory() + value + getNumber() + getExtension() + getRequest();
            const std::optional<math::Int64Range> tmp = _frames;
            _parse(_options);
            _frames = tmp;
        }

        void Path::setNumber(const std::string& value)
        {
            _path = getProtocol() + getDirectory() + getBaseName() + value + getExtension() + getRequest();
            const std::optional<math::Int64Range> tmp = _frames;
            _parse(_options);
            _frames = tmp;
        }

        void Path::setPadding(int value)
        {
            _pad = value;
            std::string num = getNumber();
            if (!num.empty())
            {
                num = toString(std::atoi(num.c_str()), _pad);
            }
            _path = getProtocol() + getDirectory() + getBaseName() + num + getExtension() + getRequest();
            const std::optional<math::Int64Range> tmp = _frames;
            _parse(_options);
            _frames = tmp;
        }

        void Path::setExtension(const std::string& value)
        {
            _path = getProtocol() + getDirectory() + getBaseName() + getNumber() + value + getRequest();
            const std::optional<math::Int64Range> tmp = _frames;
            _parse(_options);
            _frames = tmp;
        }

        void Path::setRequest(const std::string& value)
        {
            _path = getProtocol() + getDirectory() + getBaseName() + getNumber() + getExtension() + value;
            const std::optional<math::Int64Range> tmp = _frames;
            _parse(_options);
            _frames = tmp;
        }

        void Path::setFileName(const std::string& value)
        {
            _path = getProtocol() + getDirectory() + value + getRequest();
            const std::optional<math::Int64Range> tmp = _frames;
            _parse(_options);
            _frames = tmp;
        }

        void Path::setFrames(const math::Int64Range& value)
        {
            _frames = value;
        }

        bool Path::addSeq(const Path& other)
        {
            bool out = sequence(other);
            if (out)
            {
                const std::optional<math::Int64Range> frames = _frames.has_value() && other._frames.has_value() ?
                                                               math::expand(_frames.value(), other._frames.value()) :
                                                       other._frames;
                const int pad = std::max(_pad, other._pad);
                if (frames != _frames || pad != _pad || hasSeqWildcard())
                {
                    _frames = frames;
                    _pad = pad;
                    if (_frames.has_value())
                    {
                        setNumber(toString(_frames.value().getMin(), _pad));
                    }
                }
            }
            return out;
        }

        bool Path::isAbsolute() const
        {
            bool out = false;
            if (hasDirectory())
            {
                const std::string dir = getDirectory();
                if (pathSeparators.find_first_of(dir[0]) != std::string::npos)
                {
                    out = true;
                }
                else if (dir.size() > 1 &&
                         dir[0] >= 'A' &&
                         dir[0] <= 'Z' &&
                         ':' == dir[1])
                {
                    out = true;
                }
            }
            return out;
        }

        bool Path::testExt(const std::vector<std::string>& exts) const
        {
            return !exts.empty() ?
                std::find(exts.begin(), exts.end(),
                          string::toLower(getExtension())) != exts.end() :
                true;
        }

        const std::string Path::numbers = "0123456789#";
        const std::string Path::pathSeparators = "/\\";

        void Path::_parse(const PathOptions& options)
        {
            // Find the request.
            size_t size = _path.size();
            size_t requestPos = std::string::npos;
            if (size > 0)
            {
                for (int i = 0; i < size; ++i)
                {
                    if ('?' == _path[i])
                    {
                        requestPos = i;
                        break;
                    }
                }
            }
            if (requestPos != std::string::npos)
            {
                const size_t sizeTmp = size - requestPos;
                _request = std::pair<size_t, size_t>(requestPos, sizeTmp);
                size -= sizeTmp;
            }

            // Find the protocol.
            size_t protocolEnd = std::string::npos;
            size_t protocolSize = 0;
            if (size > 2)
            {
                for (int i = 0; i < size - 3; ++i)
                {
                    if (':' == _path[i] &&
                        '/' == _path[i + 1] &&
                        '/' == _path[i + 2])
                    {
                        protocolEnd = i + 2;
                        protocolSize = protocolEnd + 1;
                        break;
                    }
                }
            }
            if (protocolEnd != std::string::npos)
            {
                _protocol = std::pair<size_t, size_t>(0, protocolSize);
            }

            // Find the directory.
            size_t dirEnd = std::string::npos;
            size_t dirSize = 0;
            if (size > 0)
            {
                for (int i = size - 1; i >= static_cast<int>(protocolSize); --i)
                {
                    if (pathSeparators.find(_path[i]) != std::string::npos)
                    {
                        dirEnd = i;
                        dirSize = dirEnd + 1 - protocolSize;
                        break;
                    }
                }
            }
            if (std::string::npos == dirEnd &&
                size > 1 &&
                _path[0] >= 'A' && _path[0] <= 'Z' &&
                ':' == _path[1])
            {
                dirEnd = 1;
                dirSize = 2;
                _dir = std::pair<size_t, size_t>(0, dirSize);
            }
            else if (dirEnd != std::string::npos)
            {
                _dir = std::pair<size_t, size_t>(protocolSize, dirSize);
            }
            const size_t protocolDirSize = protocolSize + dirSize;

            // Find the extension.
            size_t extPos = std::string::npos;
            if (size > 0)
            {
                for (int i = size - 1; i >= static_cast<int>(protocolDirSize); --i)
                {
                    if ('.' == _path[i])
                    {
                        extPos = i;
                        break;
                    }
                }
            }
            if (extPos != std::string::npos &&
                extPos > protocolDirSize &&
                extPos < size - 1)
            {
                const size_t sizeTmp = size - extPos;
                _ext = std::pair<size_t, size_t>(extPos, sizeTmp);
                size -= sizeTmp;
            }

            // Find the number.
            size_t numPos = std::string::npos;
            if (size > 0)
            {
                for (int i = size - 1; i >= static_cast<int>(protocolDirSize); --i)
                {
                    if (numbers.find(_path[i]) != std::string::npos)
                    {
                        numPos = i;
                    }
                    else
                    {
                        break;
                    }
                }
                if (numPos != std::string::npos &&
                    size - numPos > options.seqMaxDigits)
                {
                    numPos = std::string::npos;
                }
                if (options.seqNegative &&
                    numPos != std::string::npos &&
                    numPos > protocolDirSize &&
                    '-' == _path[numPos - 1])
                {
                    --numPos;
                }
            }
            if (numPos != std::string::npos)
            {
                const size_t sizeTmp = size - numPos;
                _num = std::pair<size_t, size_t>(numPos, sizeTmp);
                if ('0' == _path[numPos])
                {
                    _pad = sizeTmp;
                }
                else if ('#' == _path[numPos])
                {
                    _pad = sizeTmp;
                }
                if (options.seqNegative &&
                    '-' == _path[numPos] &&
                    numPos < size - 1 &&
                    '0' == _path[numPos + 1])
                {
                    _pad = sizeTmp - 1;
                }
                if (_path[numPos] != '#')
                {
                    const int64_t frame = std::atoi(getNumber().c_str());
                    _frames = math::Int64Range(frame, frame);
                }
                size -= sizeTmp;
            }

            // Find the base name.
            if (size - protocolDirSize > 0)
            {
                _base = std::pair<size_t, size_t>(
                    protocolDirSize,
                    size - protocolDirSize);
            }
        }

        const std::pair<size_t, size_t> Path:: _invalid(std::string::npos, std::string::npos);

        TLRENDER_ENUM_IMPL(
            DirListSort,
            "Name",
            "Extension",
            "Size",
            "Time");

        bool DirListOptions::operator == (const DirListOptions& other) const
        {
            return
                sort == other.sort &&
                sortReverse == other.sortReverse &&
                filter == other.filter &&
                filterFiles == other.filterFiles &&
                filterExt == other.filterExt &&
                seq == other.seq &&
                seqExts == other.seqExts &&
                seqNegative == other.seqNegative &&
                seqMaxDigits == other.seqMaxDigits &&
                hidden == other.hidden;
        }

        bool DirListOptions::operator != (const DirListOptions& other) const
        {
            return !(*this == other);
        }

        bool DirEntry::operator == (const DirEntry& other) const
        {
            return
                path == other.path &&
                isDir == other.isDir &&
                size == other.size &&
                time == other.time;
        }

        bool DirEntry::operator != (const DirEntry& other) const
        {
            return !(*this == other);
        }

        std::vector<DirEntry> dirList(
            const std::filesystem::path& path,
            const DirListOptions& options)
        {
            std::vector<DirEntry> out;
            PathOptions pathOptions;
            pathOptions.seqNegative = options.seqNegative;
            pathOptions.seqMaxDigits = options.seqMaxDigits;
            try
            {
                for (const auto& i : std::filesystem::directory_iterator(path))
                {
                    const Path path(i.path().u8string(), pathOptions);
                    const std::string fileName = i.path().filename().u8string();

                    // Apply filters.
                    bool keep = true;
                    if (keep && !options.hidden && isDotFile(fileName))
                    {
                        keep = false;
                    }
                    const bool isDir = std::filesystem::is_directory(i.path());
                    if (keep && !isDir && !options.filterExt.empty())
                    {
                        keep = std::find(
                            options.filterExt.begin(),
                            options.filterExt.end(),
                            string::toLower(path.getExtension())) !=
                               options.filterExt.end();
                    }
                    if (keep && !options.filter.empty())
                    {
                        keep = string::contains(
                            fileName,
                            options.filter,
                            string::Compare::CaseInsensitive);
                    }
                    if (keep && options.filterFiles && !isDir)
                    {
                        keep = false;
                    }

                    if (keep)
                    {
                        // Check for sequences.
                        bool seq = false;
                        if (!isDir && options.seq && path.testExt(options.seqExts))
                        {
                            for (auto& j : out)
                            {
                                if (j.path.addSeq(path))
                                {
                                    seq = true;
                                    j.size += std::filesystem::file_size(i.path());
                                    j.time = std::max(
                                        j.time,
                                        std::filesystem::last_write_time(i.path()));
                                    break;
                                }
                            }
                        }

                        if (!seq)
                        {
                            // Add the entry.
                            out.push_back({
                                    path,
                                    isDir,
                                    isDir ? 0 : std::filesystem::file_size(i.path()),
                                    std::filesystem::last_write_time(i.path()) });
                        }
                    }
                }
            }
            catch (const std::exception&)
            {}

            // Sort the entries.
            std::function<int(const DirEntry& a, const DirEntry& b)> sort;
            switch (options.sort)
            {
            case DirListSort::Name:
                sort = [](const DirEntry& a, const DirEntry& b)
                    {
                        return a.path.getFileName() < b.path.getFileName();
                    };
                break;
            case DirListSort::Extension:
                sort = [](const DirEntry& a, const DirEntry& b)
                    {
                        return a.path.getExtension() < b.path.getExtension();
                    };
                break;
            case DirListSort::Size:
                sort = [](const DirEntry& a, const DirEntry& b)
                    {
                        return a.size < b.size;
                    };
                break;
            case DirListSort::Time:
                sort = [](const DirEntry& a, const DirEntry& b)
                    {
                        return a.time < b.time;
                    };
                break;
            default: break;
            }
            if (sort)
            {
                if (options.sortReverse)
                {
                    std::sort(out.rbegin(), out.rend(), sort);
                }
                else
                {
                    std::sort(out.begin(), out.end(), sort);
                }
            }

            // Sort the directories.
            std::stable_sort(
                out.begin(),
                out.end(),
                [](const DirEntry& a, const DirEntry& b)
                    {
                        return a.isDir > b.isDir;
                    });

            return out;
        }

        Path expandSeq(
            const Path& path,
            const PathOptions& pathOptions)
        {
            Path out = path;
            if (out.hasNumber() && !out.isSequence() || out.hasSeqWildcard())
            {
                // Find matching sequence files.
                bool init = true;
                const std::filesystem::path stdpath = std::filesystem::u8path(out.get());
                for (const auto& i : std::filesystem::directory_iterator(stdpath.parent_path()))
                {
                    const Path entry(i.path().u8string(), pathOptions);
                    const bool isDir = std::filesystem::is_directory(i.path());
                    if (init && !isDir)
                    {
                        if (out.sequence(entry))
                        {
                            init = false;
                            out = entry;
                        }
                    }
                    if (!init)
                    {
                        out.addSeq(entry);
                    }
                }
            }
            return out;
        }

        void to_json(nlohmann::json& json, const PathOptions& value)
        {
            json["SeqNegative"] = value.seqNegative;
            json["SeqMaxDigits"] = value.seqMaxDigits;
        }

        void to_json(nlohmann::json& json, const DirListOptions& value)
        {
            json["Sort"] = to_string(value.sort);
            json["SortReverse"] = value.sortReverse;
            json["Filter"] = value.filter;
            json["FilterFiles"] = value.filterFiles;
            json["FilterExt"] = value.filterExt;
            json["Seq"] = value.seq;
            json["SeqExts"] = value.seqExts;
            json["SeqNegative"] = value.seqNegative;
            json["SeqMaxDigits"] = value.seqMaxDigits;
            json["Hidden"] = value.hidden;
        }

        void from_json(const nlohmann::json& json, PathOptions& value)
        {
            json.at("SeqNegative").get_to(value.seqNegative);
            json.at("SeqMaxDigits").get_to(value.seqMaxDigits);
        }

        void from_json(const nlohmann::json& json, DirListOptions& value)
        {
            from_string(json.at("Sort").get<std::string>(), value.sort);
            json.at("SortReverse").get_to(value.sortReverse);
            json.at("Filter").get_to(value.filter);
            json.at("FilterFiles").get_to(value.filterFiles);
            json.at("FilterExt").get_to(value.filterExt);
            json.at("Seq").get_to(value.seq);
            json.at("SeqExts").get_to(value.seqExts);
            json.at("SeqNegative").get_to(value.seqNegative);
            json.at("SeqMaxDigits").get_to(value.seqMaxDigits);
            json.at("Hidden").get_to(value.hidden);
        }

    }
}
