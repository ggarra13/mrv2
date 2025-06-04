// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Path.h>

#include <iostream>
#include <set>

namespace tl
{
    namespace file
    {
        //! File types.
        enum class Type {
            File,      //!< Regular file
            Directory, //!< Directory

            Count,
            First = File
        };
        TLRENDER_ENUM(Type);
        TLRENDER_ENUM_SERIALIZE(Type);

        //! File permissions.
        enum class Permissions {
            Read = 1,  //!< Readable
            Write = 2, //!< Writable
            Exec = 4,  //!< Executable
        };

        //! File system information.
        class FileInfo
        {
        public:
            FileInfo();
            explicit FileInfo(const Path&);

            //! Get the path.
            const Path& getPath() const;

            //! Get the file type.
            Type getType() const;

            //! Get the file size.
            uint64_t getSize() const;

            //! Get the file permissions.
            int getPermissions() const;

            //! Get the last modification time.
            time_t getTime() const;

            //! Expand the sequence.
            void sequence(const FileInfo&);

        private:
            bool _stat(std::string* error);

            Path _path;
            bool _exists = false;
            Type _type = Type::File;
            uint64_t _size = 0;
            int _permissions = 0;
            time_t _time = 0;
        };

        //! Directory sorting.
        enum class ListSort {
            Name,
            Extension,
            Size,
            Time,

            Count,
            First = Name
        };
        TLRENDER_ENUM(ListSort);
        TLRENDER_ENUM_SERIALIZE(ListSort);

        //! Directory list options.
        struct ListOptions
        {
            ListSort sort = ListSort::Name;
            bool reverseSort = false;
            bool sortDirectoriesFirst = true;
            bool dotAndDotDotDirs = false;
            bool dotFiles = false;
            bool sequence = true;
            std::set<std::string> sequenceExtensions;
            bool negativeNumbers = false;
            size_t maxNumberDigits = 9;

            bool operator==(const ListOptions&) const;
            bool operator!=(const ListOptions&) const;
        };

        //! Get the contents of the given directory.
        void list(
            const std::string&, std::vector<FileInfo>&,
            const ListOptions& = ListOptions());
    } // namespace file
} // namespace tl

#include <tlCore/FileInfoInline.h>
