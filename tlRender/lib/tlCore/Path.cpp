// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender, mrv2 and feather-tk projects.

#include <tlCore/Path.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <algorithm>
#include <array>
#include <filesystem>
#include <list>
#include <sstream>

namespace tl
{
    namespace file
    {
        std::filesystem::path toFileSystem(const std::string& value)
        {
#if defined(__cpp_char8_t)
            return std::filesystem::path(
                std::u8string(value.begin(), value.end()));
#else
            return std::filesystem::u8path(value);
#endif // __cpp_char8_t
        }

        std::string fromFileSystem(const std::filesystem::path& value)
        {
            // Under C++17 u8string() is a std::string and this is a copy;
            // under C++20 it is a std::u8string and the iterators convert
            // the characters.
            const auto u8 = value.u8string();
            return std::string(u8.begin(), u8.end());
        }

        std::string fromFileSystemGeneric(const std::filesystem::path& value)
        {
            const auto u8 = value.generic_u8string();
            return std::string(u8.begin(), u8.end());
        }

        std::vector<std::string> split(std::filesystem::path path)
        {
            std::list<std::string> out;
            const std::filesystem::path root = path.root_path();
            while (!path.empty() && path != root)
            {
                if (!path.filename().empty())
                {
                    out.push_front(fromFileSystem(path.filename()));
                }
                path = path.parent_path();
            }
            if (!path.empty())
            {
                out.push_front(fromFileSystem(path));
            }
            return std::vector<std::string>(out.begin(), out.end());
        }

        std::string appendSeparator(const std::string& value)
        {
            // An empty directory means the current one. There is no separator to
            // find, so the search below would fall through and add one, turning
            // "" + "render.tif" into the absolute "/render.tif".
            if (value.empty())
            {
                return value;
            }
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
                // The padding is the number of digits, and the sign goes in
                // front of it rather than being padded around: four digits of
                // minus one is "-0001", not "00-1".
                ss << std::internal << std::setfill('0');
                ss << std::setw(frame < 0 ? pad + 1 : pad);
            }
            ss << frame;
            return ss.str();
        }

        FrameSeq::FrameSeq(const math::Int64Range& range, int inc) :
            range(range),
            inc(inc)
        {}

        FrameSeq::FrameSeq(int64_t min, int64_t max, int inc) :
            range(min, max),
            inc(inc)
        {}

        FrameSeq::FrameSeq(int64_t frame) :
            range(frame, frame),
            inc(1)
        {}


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

        std::vector<FrameSeq> toFrameSeq(const std::vector<int64_t>& value)
        {
            std::vector<FrameSeq> out;
            std::vector<int64_t> tmp = value;
            std::sort(tmp.begin(), tmp.end());
            tmp.erase(std::unique(tmp.begin(), tmp.end()), tmp.end());
            for (size_t i = 0; i < tmp.size(); ++i)
            {
                size_t j = i + 2;
                for (;
                     j < tmp.size() && tmp[j] - tmp[j - 1] == tmp[i + 1] - tmp[i];
                     ++j)
                {}
                if (j > i + 2)
                {
                    out.push_back(FrameSeq(tmp[i], tmp[j - 1], static_cast<int>(tmp[i + 1] - tmp[i])));
                    i = j - 1;
                }
                else
                {
                    j = i + 1;
                    for (;
                         j < tmp.size() && tmp[j] - tmp[j - 1] == tmp[i + 1] - tmp[i];
                         ++j)
                    {
                    }
                    if (j > i + 1)
                    {
                        out.push_back(FrameSeq(tmp[i], tmp[j - 1], static_cast<int>(tmp[i + 1] - tmp[i])));
                        i = j - 1;
                    }
                    else
                    {
                        out.push_back(FrameSeq(tmp[i], tmp[i]));
                    }
                }
            }
            return out;
        }

        std::vector<int64_t> toFrames(const FrameSeq& value)
        {
            std::vector<int64_t> out;
            const int64_t inc = value.inc > 0 ? value.inc : 1;
            for (int64_t frame = value.range.min();
                 frame <= value.range.max(); frame += inc)
            {
                out.push_back(frame);
            }
            return out;
        }

        std::vector<int64_t> toFrames(const std::vector<FrameSeq>& value)
        {
            std::vector<int64_t> out;
            for (const auto& i : value)
            {
                const auto frames = toFrames(i);
                out.insert(out.end(), frames.begin(), frames.end());
            }
            return out;
        }

        std::string getLabel(const FrameSeq& value, int pad)
        {
            std::stringstream ss;
            if (value.range.equal())
            {
                ss << toString(value.range.min(), pad);
            }
            else
            {
                ss << toString(value.range.min(), pad) << "-" <<
                    toString(value.range.max(), pad);
                if (value.inc > 1)
                {
                    ss << ":" << value.inc;
                }
            }
            return ss.str();
        }

        std::string getLabel(const std::vector<FrameSeq>& value, int pad)
        {
            std::vector<std::string> tmp;
            for (const auto& i : value)
            {
                tmp.push_back(getLabel(i, pad));
            }
            return string::join(tmp, ',');
        }

        namespace
        {
            int64_t seqInc(const FrameSeq& value)
            {
                return value.inc > 0 ? value.inc : 1;
            }

            bool seqContains(const FrameSeq& value, int64_t frame)
            {
                return
                    frame >= value.range.min() &&
                    frame <= value.range.max() &&
                    0 == (frame - value.range.min()) % seqInc(value);
            }

            // Merge a sequence with the one that follows it when together they
            // form a single sequence with a constant increment. A sequence holding
            // one frame has no increment of its own, so it takes the increment of
            // whichever neighbor it is merged with.
            void mergeSeq(std::vector<FrameSeq>& seqs, size_t i)
            {
                if (i + 1 >= seqs.size())
                {
                    return;
                }
                FrameSeq& a = seqs[i];
                const FrameSeq& b = seqs[i + 1];
                const int64_t inc = a.range.equal() ?
                                    b.range.min() - a.range.min() :
                                    seqInc(a);
                if (inc <= 0 ||
                    b.range.min() != a.range.max() + inc ||
                    (!b.range.equal() && seqInc(b) != inc))
                {
                    return;
                }
                a.range = math::Int64Range(a.range.min(), b.range.max());
                a.inc = static_cast<int>(inc);
                seqs.erase(seqs.begin() + i + 1);
            }
        }


        void addFrame(std::vector<FrameSeq>& seqs, int64_t frame)
        {
            // Find the first sequence starting after the frame.
            const size_t i = std::upper_bound(
                seqs.begin(),
                seqs.end(),
                frame,
                [](int64_t frame, const FrameSeq& seq)
                    {
                        return frame < seq.range.min();
                    }) - seqs.begin();

            // Try extending the preceding sequence.
            if (i > 0)
            {
                FrameSeq& prev = seqs[i - 1];
                if (seqContains(prev, frame))
                {
                    return;
                }
                if (frame > prev.range.max())
                {
                    const int64_t inc = prev.range.equal() ?
                                        frame - prev.range.min() :
                                        seqInc(prev);
                    if (frame == prev.range.max() + inc)
                    {
                        prev.range = math::Int64Range(prev.range.min(), frame);
                        prev.inc = static_cast<int>(inc);
                        mergeSeq(seqs, i - 1);
                        return;
                    }
                }
                else
                {
                    // The frame falls within the preceding sequence but off its
                    // increment, so that sequence has to be split. Rebuild it
                    // along with its neighbors, since splitting it can leave
                    // pieces that belong with them.
                    const size_t begin = i >= 2 ? i - 2 : 0;
                    const size_t end = std::min(i + 1, seqs.size());
                    std::vector<int64_t> frames;
                    frames.push_back(frame);
                    for (size_t j = begin; j < end; ++j)
                    {
                        const std::vector<int64_t> tmp = toFrames(seqs[j]);
                        frames.insert(frames.end(), tmp.begin(), tmp.end());
                    }
                    const std::vector<FrameSeq> split = toFrameSeq(frames);
                    seqs.erase(seqs.begin() + begin, seqs.begin() + end);
                    seqs.insert(seqs.begin() + begin, split.begin(), split.end());
                    return;
                }
            }

            // Try extending the following sequence backwards.
            if (i < seqs.size())
            {
                FrameSeq& next = seqs[i];
                const int64_t inc = next.range.equal() ?
                                    next.range.min() - frame :
                                    seqInc(next);
                if (inc > 0 && frame == next.range.min() - inc)
                {
                    next.range = math::Int64Range(frame, next.range.max());
                    next.inc = static_cast<int>(inc);
                    if (i > 0)
                    {
                        mergeSeq(seqs, i - 1);
                    }
                    return;
                }
            }

            seqs.insert(seqs.begin() + i, FrameSeq(frame));
        }

        size_t getFrameCount(const FrameSeq& value)
        {
            const int64_t inc = seqInc(value);
            return static_cast<size_t>(
                (value.range.max() - value.range.min()) / inc + 1);
        }

        size_t getFrameCount(const std::vector<FrameSeq>& value)
        {
            size_t out = 0;
            for (const auto& i : value)
            {
                out += getFrameCount(i);
            }
            return out;
        }

        std::optional<math::Int64Range> getRange(const std::vector<FrameSeq>& value)
        {
            std::optional<math::Int64Range> out;
            for (const auto& i : value)
            {
                out = out.has_value() ? expand(out.value(), i.range) : i.range;
            }
            return out;
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
            _path = value + getDirectory() + getBaseName() + getNumber() + getSuffix() + getExtension() + getRequest();
            const std::vector<FrameSeq> tmp = _seq;
            _parse(_options);
            _setSeq(tmp);
        }

        void Path::setDirectory(const std::string& value)
        {
            _path = getProtocol() + value + getBaseName() + getNumber() + getSuffix() + getExtension() + getRequest();
            const std::vector<FrameSeq> tmp = _seq;
            _parse(_options);
            _setSeq(tmp);
        }

        void Path::setBaseName(const std::string& value)
        {
            _path = getProtocol() + getDirectory() + value + getNumber() + getSuffix() + getExtension() + getRequest();
            const std::vector<FrameSeq> tmp = _seq;
            _parse(_options);
            _setSeq(tmp);
        }

        void Path::setSuffix(const std::string& value)
        {
            _path = getProtocol() + getDirectory() + getBaseName() + getNumber() + value + getExtension() + getRequest();
            const std::vector<FrameSeq> tmp = _seq;
            _parse(_options);
            _setSeq(tmp);
        }

        void Path::setNumber(const std::string& value)
        {
            _path = getProtocol() + getDirectory() + getBaseName() + value + getSuffix() + getExtension() + getRequest();
            const std::vector<FrameSeq> tmp = _seq;
            _parse(_options);
            _setSeq(tmp);
        }

        void Path::setPadding(int value)
        {
            _pad = value;
            std::string num = getNumber();
            if (!num.empty())
            {
                num = toString(std::atoi(num.c_str()), _pad);
            }
            _path = getProtocol() + getDirectory() + getBaseName() + num + getSuffix() + getExtension() + getRequest();
            const std::vector<FrameSeq> tmp = _seq;
            _parse(_options);
            _setSeq(tmp);
        }

        void Path::setExtension(const std::string& value)
        {
            _path = getProtocol() + getDirectory() + getBaseName() + getNumber() + getSuffix() + value + getRequest();
            const std::vector<FrameSeq> tmp = _seq;
            _parse(_options);
            _setSeq(tmp);
        }

        void Path::setRequest(const std::string& value)
        {
            _path = getProtocol() + getDirectory() + getBaseName() + getNumber() + getSuffix() + getExtension() + value;
            const std::vector<FrameSeq> tmp = _seq;
            _parse(_options);
            _setSeq(tmp);
        }

        void Path::setFileName(const std::string& value)
        {
            _path = getProtocol() + getDirectory() + value + getRequest();
            const std::vector<FrameSeq> tmp = _seq;
            _parse(_options);
            _setSeq(tmp);
        }

        void Path::setFrames(const math::Int64Range& value)
        {
            _seq = { FrameSeq(value) };
            _frames = value;
        }

        void Path::setSeq(const std::vector<FrameSeq>& value)
        {
            _setSeq(value);
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
                        setNumber(toString(_frames.value().min(), _pad));
                    }
                }
            }
            return out;
        }

        void Path::normalizeSeq()
        {
            if (_seq.size() > 1)
            {
                _setSeq(toFrameSeq(toFrames(_seq)));
            }
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

            // Find the suffix (optional non-digit text between the frame number and extension).
            // Scan backwards past non-digit characters; those become the suffix.
            // The number detection that follows then operates on the shortened stem.
            //
            // Examples:
            //   render_0001_stable  →  suffix="_stable",  number = "0001"
            //   render_v003_0001    →  suffix="",         number = "0001"
            //   render_v003_cap2_0001 → suffix="",        number = "0001"
            if (size > protocolDirSize)
            {
                int i = static_cast<int>(size) - 1;
                while (i >= static_cast<int>(protocolDirSize) &&
                       numbers.find(_path[i]) == std::string::npos)
                {
                    --i;
                }
                const size_t sufPos = static_cast<size_t>(i) + 1;
                if (sufPos < size)
                {
                    _suf = std::pair<size_t, size_t>(sufPos, size - sufPos);
                    size = sufPos;   // strip suffix so number detection sees a clean stem
                }
            }

            // Find the number.
            size_t numPos = std::string::npos;
            if (size > 0)
            {
                // It must end in 'd', start with '%', and have only digits in between.
                if (size > 1 && 'd' == _path[size - 1])
                {
                    for (int i = size - 2; i >= static_cast<int>(protocolDirSize); --i)
                    {
                        if ('%' == _path[i])
                        {
                            bool isPrintf = true;
                            // Ensure everything between % and d is a digit
                            for (int j = i + 1; j < size - 1; ++j)
                            {
                                if (!isdigit(_path[j]))
                                {
                                    isPrintf = false;
                                    break;
                                }
                            }

                            if (isPrintf)
                            {
                                numPos = i;
                            }
                            // Once we hit a %, we stop searching backwards regardless of validity
                            // to avoid misinterpreting earlier % signs.
                            break;
                        }
                    }
                }
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
                    '-' == _path[numPos - 1] &&
                    (numPos < 2 ||
                     !std::isalnum(static_cast<unsigned char>(_path[numPos - 2]))))
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
                else if ('%' == _path[numPos])
                {
                    if (sizeTmp > 2)
                    {
                        std::string widthStr = _path.substr(numPos + 1, sizeTmp - 2);
                        _pad = std::atoi(widthStr.c_str());
                    }
                    else
                    {
                        _pad = 0; // case for %d
                    }
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

        void Path::_setSeq(const std::vector<FrameSeq>& value)
        {
            _seq = value;
            _frames = getRange(_seq);
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
                    const Path path(fromFileSystem(i.path()), pathOptions);
                    const std::string fileName = fromFileSystem(i.path().filename());

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

        std::vector<FrameSeq> findSeq(
            const Path& path,
            const PathOptions& pathOptions)
        {
            std::vector<int64_t> frames;
            if (path.hasNumber() || path.hasSeqWildcard())
            {
                const auto abs = std::filesystem::absolute(
                    toFileSystem(path.get()));
                const auto parent = abs.parent_path();
                if (std::filesystem::exists(parent))
                {
                    for (const auto& i : std::filesystem::directory_iterator(parent))
                    {
                        if (std::filesystem::is_directory(i.path()))
                        {
                            continue;
                        }
                        const Path entry(fromFileSystem(i.path()), pathOptions);
                        if (path.sequence(entry) &&
                            entry.getFrames().has_value())
                        {
                            frames.push_back(
                                entry.getFrames().value().min());
                        }
                    }
                }
            }
            return toFrameSeq(frames);
        }

        Path expandSeq(
            const Path& path,
            const PathOptions& pathOptions)
        {
            Path out = path;
            if ((out.hasNumber() && !out.isSequence()) || out.hasSeqWildcard())
            {
                // Find matching sequence files.
                const auto abs = std::filesystem::absolute(toFileSystem(out.get()));
                const auto parent = abs.parent_path();
                if (std::filesystem::exists(parent))
                {
                    bool init = true;
                    for (const auto& i : std::filesystem::directory_iterator(parent))
                    {
                        const Path entry(fromFileSystem(i.path()), pathOptions);
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
                    out.normalizeSeq();
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

        void to_json(nlohmann::json& j, const Path& p)
        {
            j = nlohmann::json{
                {"path", p.get()},
                {"options", p.getOptions()}
            };

            // Explicitly handle the std::optional for maximum compatibility across json versions
            if (const auto& frames = p.getFrames())
            {
                j["frames"] = *frames;
            }
            else
            {
                j["frames"] = nullptr;
            }
        }

        void from_json(const nlohmann::json& j, Path& p)
        {
            std::string pathStr;
            j.at("path").get_to(pathStr);

            PathOptions options;
            if (j.contains("options"))
            {
                j.at("options").get_to(options);
            }

            // Reconstruct the Path to ensure internal _parse() is triggered correctly
            p = Path(pathStr, options);

            // Re-apply frames if they are present in the JSON payload
            if (j.contains("frames") && !j.at("frames").is_null())
            {
                p.setFrames(j.at("frames").get<math::Int64Range>());
            }
        }

    }
}
