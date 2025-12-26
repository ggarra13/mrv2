// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/FileInfoPrivate.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <algorithm>
#include <array>
#include <functional>

namespace tl
{
    namespace file
    {
        TLRENDER_ENUM_IMPL(Type, "File", "Directory");
        TLRENDER_ENUM_SERIALIZE_IMPL(Type);

        FileInfo::FileInfo() {}

        FileInfo::FileInfo(const Path& path) :
            _path(path)
        {
            std::string error;
            _stat(&error);
        }

        void FileInfo::sequence(const FileInfo& value)
        {
            if (_path.addSeq(value._path))
            {
                _size += value._size;
                _permissions = std::min(_permissions, value._permissions);
                _time = std::max(_time, value._time);
            }
        }

        TLRENDER_ENUM_IMPL(ListSort, "Name", "Extension", "Size", "Time");
        TLRENDER_ENUM_SERIALIZE_IMPL(ListSort);

        bool ListOptions::operator==(const ListOptions& other) const
        {
            return sort == other.sort && reverseSort == other.reverseSort &&
                   sortDirectoriesFirst == other.sortDirectoriesFirst &&
                   dotAndDotDotDirs == other.dotAndDotDotDirs &&
                   dotFiles == other.dotFiles && sequence == other.sequence &&
                   sequenceExtensions == other.sequenceExtensions &&
                   negativeNumbers == other.negativeNumbers &&
                   maxNumberDigits == other.maxNumberDigits;
        }

        bool ListOptions::operator!=(const ListOptions& other) const
        {
            return !(*this == other);
        }

        bool listFilter(const std::string& fileName, const ListOptions& options)
        {
            bool out = false;
            if (!options.dotAndDotDotDirs && 1 == fileName.size() &&
                '.' == fileName[0])
            {
                out = true;
            }
            else if (
                !options.dotAndDotDotDirs && 2 == fileName.size() &&
                '.' == fileName[0] && '.' == fileName[1])
            {
                out = true;
            }
            else if (
                !options.dotFiles && fileName.size() > 0 && '.' == fileName[0])
            {
                out = true;
            }
            return out;
        }

        void listSequence(
            const std::string& path, const std::string& fileName,
            std::vector<FileInfo>& out, const ListOptions& options)
        {
            PathOptions pathOptions;
            pathOptions.seqMaxDigits =
                options.sequence ? options.maxNumberDigits : 0;
            const Path p(path, fileName, pathOptions);
            const FileInfo f(p);
            bool sequence = false;
            if (options.sequence && p.hasNumber() &&
                f.getType() != Type::Directory)
            {
                bool sequenceExtension = true;
                if (!options.sequenceExtensions.empty())
                {
                    sequenceExtension =
                        std::find(
                            options.sequenceExtensions.begin(),
                            options.sequenceExtensions.end(),
                            string::toLower(p.getExtension())) !=
                        options.sequenceExtensions.end();
                }
                if (sequenceExtension)
                {
                    for (auto& i : out)
                    {
                        if (i.getPath().sequence(p))
                        {
                            sequence = true;
                            i.sequence(f);
                            break;
                        }
                    }
                }
            }
            if (!sequence)
            {
                out.push_back(f);
            }
        }

        void list(
            const std::string& path, std::vector<FileInfo>& out,
            const ListOptions& options)
        {
            out.clear();

            _list(path, out, options);

            std::function<int(const FileInfo& a, const FileInfo& b)> sort;
            switch (options.sort)
            {
            case ListSort::Name:
                sort = [](const FileInfo& a, const FileInfo& b)
                { return a.getPath().get() < b.getPath().get(); };
                break;
            case ListSort::Extension:
                sort = [](const FileInfo& a, const FileInfo& b) {
                    return a.getPath().getExtension() <
                           b.getPath().getExtension();
                };
                break;
            case ListSort::Size:
                sort = [](const FileInfo& a, const FileInfo& b)
                { return a.getSize() < b.getSize(); };
                break;
            case ListSort::Time:
                sort = [](const FileInfo& a, const FileInfo& b)
                { return a.getTime() < b.getTime(); };
                break;
            default:
                break;
            }
            if (sort)
            {
                if (options.reverseSort)
                {
                    std::sort(out.rbegin(), out.rend(), sort);
                }
                else
                {
                    std::sort(out.begin(), out.end(), sort);
                }
            }
            if (options.sortDirectoriesFirst)
            {
                std::stable_sort(
                    out.begin(), out.end(),
                    [](const FileInfo& a, const FileInfo& b)
                    {
                        return static_cast<size_t>(a.getType()) >
                               static_cast<size_t>(b.getType());
                    });
            }
        }
    } // namespace file
} // namespace tl
