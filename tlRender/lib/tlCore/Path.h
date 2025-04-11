// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Range.h>
#include <tlCore/Util.h>

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

namespace tl
{
    namespace file
    {
        //! Path separators.
        const std::vector<char> pathSeparators = {'/', '\\'};

        //! Path separator.
#if defined(_WINDOWS)
        const char pathSeparator = '\\';
#else  // _WINDOWS
        const char pathSeparator = '/';
#endif // _WINDOWS

        //! Path options.
        struct PathOptions
        {
            size_t maxNumberDigits = 9;

            constexpr bool operator==(const PathOptions&) const;
            constexpr bool operator!=(const PathOptions&) const;
        };

        //! Path types.
        enum class PathType { Full, Path, FileName };

        //! File path.
        class Path
        {
        public:
            Path();
            explicit Path(
                const std::string&, const PathOptions& = PathOptions());
            Path(
                const std::string& directory, const std::string&,
                const PathOptions& = PathOptions());
            Path(
                const std::string& directory, const std::string& baseName,
                const std::string& number, size_t padding,
                const std::string& extension,
                const std::string& protocol = std::string(),
                const std::string& request = std::string());

            //! Get the path.
            std::string get(int number = -1, PathType = PathType::Full) const;

            //! Get the protocol.
            const std::string& getProtocol() const;

            //! Get the protocol name.
            const std::string& getProtocolName() const;

            //! Get whether the protocol is "file:".
            bool isFileProtocol() const;

            //! Set the protocol.
            void setProtocol(const std::string&);

            //! Get the directory.
            const std::string& getDirectory() const;

            //! Set the directory.
            void setDirectory(const std::string&);

            //! Get the base name.
            const std::string& getBaseName() const;

            //! Set the base name.
            void setBaseName(const std::string&);

            //! Get the number.
            const std::string& getNumber() const;

            //! Set the number.
            void setNumber(const std::string&);

            //! Get the number zero padding.
            size_t getPadding() const;

            //! Set the number zero padding.
            void setPadding(size_t);

            //! Get the number sequence.
            const math::IntRange& getSequence() const;

            //! Set the number sequence.
            void setSequence(const math::IntRange&);

            //! Get whether this path is a sequence.
            bool isSequence() const;

            //! Get whether the given path is part of this sequence.
            bool sequence(const Path&) const;

            //! Get the sequence string.
            std::string getSequenceString() const;

            //! Get the extension.
            const std::string& getExtension() const;

            //! Set the extension.
            void setExtension(const std::string&);

            //! Get the request.
            const std::string& getRequest() const;

            //! Set the request.
            void setRequest(const std::string&);

            //! Is the path empty?
            bool isEmpty() const;

            //! Is the path absolute?
            bool isAbsolute() const;

            bool operator==(const Path&) const;
            bool operator!=(const Path&) const;

        private:
            void _protocolUpdate();
            void _numberUpdate();

            std::string _protocol;
            std::string _protocolName;
            std::string _directory;
            std::string _baseName;
            std::string _number;
            int _numberValue = 0;
            size_t _numberDigits = 0;
            math::IntRange _sequence;
            size_t _padding = 0;
            std::string _extension;
            std::string _request;
        };

        //! Get whether the given character is a path separator.
        bool isPathSeparator(char);

        //! Append a path separator.
        std::string appendSeparator(const std::string&);

        //! Get the parent directory.
        std::string getParent(const std::string&);

        //! Get the list of file system drives.
        std::vector<std::string> getDrives();

        //! User paths.
        enum class UserPath {
            Home,
            Desktop,
            Documents,
            Downloads,

            Count,
            First = Home
        };
        TLRENDER_ENUM(UserPath);
        TLRENDER_ENUM_SERIALIZE(UserPath);

        //! Get a user path.
        std::string getUserPath(UserPath);
    } // namespace file
} // namespace tl

#include <tlCore/PathInline.h>
