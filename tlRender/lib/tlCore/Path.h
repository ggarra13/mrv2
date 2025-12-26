// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender and feather-tk project.

#pragma once

#include <tlCore/Range.h>
#include <tlCore/Util.h>

#include <filesystem>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

namespace tl
{
    namespace file
    {
        //! \name File Paths
        ///@{

        //! Does the file name start with a dot?
        bool isDotFile(const std::string&);

        //! Split a path.
        std::vector<std::string> split(std::filesystem::path);

        //! Append a path separator.
        std::string appendSeparator(const std::string&);

        //! Get the list of file system drives.
        std::vector<std::string> getDrives();

        //! User paths.
        enum class UserPath
        {
            Home,
            Desktop,
            Documents,
            Downloads,

            Count,
            First = Home
        };
        TLRENDER_ENUM(UserPath);

        //! Get a user path.
        std::string getUserPath(UserPath);

        //! Convert a frame number to a string.
        std::string toString(int64_t frame, int pad = 0);

        //! File path options.
        struct PathOptions
        {
            bool   seqNegative  = true;
            size_t seqMaxDigits = 9;

            bool operator == (const PathOptions&) const;
            bool operator != (const PathOptions&) const;
        };
    
        //! File path.
        //! 
        //! Example: file:///tmp/render.0001.exr?user=foo;password=bar
        //! * protocol: file://
        //! * dir: /tmp/
        //! * base: render.
        //! * number: 0001
        //! * padding: 4
        //! * extension: .exr
        //! * request: ?user=foo;password=bar
        //! * file name: render.0001.exr
        class Path
        {
        public:
            Path() = default;
            explicit Path(
                const std::string&,
                const PathOptions& = PathOptions());
            Path(
                const std::string& dir,
                const std::string& fileName,
                const PathOptions& = PathOptions());

            //! \name Path Options
            ///@{

            const PathOptions& getOptions();
            void setOptions(const PathOptions&);

            ///@}

            //! \name Path Components
            ///@{

            const std::string& get() const;
            bool isEmpty() const;

            bool hasProtocol() const;
            bool hasDirectory() const;
            bool hasBaseName() const;
            bool hasNumber() const;
            bool hasExtension() const;
            bool hasRequest() const;

            std::string getProtocol() const;
            std::string getDirectory() const;
            std::string getBaseName() const;
            std::string getNumber() const;
            int getPadding() const;
            std::string getExtension() const;
            std::string getRequest() const;
            std::string getFileName(bool dir = false) const;

            void setProtocol(const std::string&);
            void setDirectory(const std::string&);
            void setBaseName(const std::string&);
            void setNumber(const std::string&);
            void setPadding(int);
            void setExtension(const std::string&);
            void setRequest(const std::string&);
            void setFileName(const std::string&);

            ///@}

            //! \name File Sequences
            ///@{

            const std::optional<math::Int64Range>& getFrames() const;
            void setFrames(const math::Int64Range&);

            //! Get whether this is a sequence.
            bool isSequence() const;

            //! Get whether this has a sequence wildcard ('#').
            bool hasSeqWildcard() const;

            //! Get a file name with the given frame number.
            std::string getFrame(int64_t frame, bool dir = false) const;

            //! Get the frame range string.
            std::string getFrameRange() const;

            //! Get whether a path is part of this sequence.
            bool sequence(const Path&) const;

            //! Add a path to this sequence.
            bool addSeq(const Path&);
            
            ///@}

            //! \name Utility
            ///@{
            
            //! Get whether the path is absolute.
            bool isAbsolute() const;

            //! Test whether this extension matches one in the given list.
            bool testExt(const std::vector<std::string>&) const;

            ///@}

            //! \name Constants
            ///@{
            
            static const std::string numbers;
            static const std::string pathSeparators;

            ///@}

            bool operator == (const Path&) const;
            bool operator != (const Path&) const;

        private:
            void _parse(const PathOptions&);

            std::string _path;
            PathOptions _options;
            static const std::pair<size_t, size_t> _invalid;
            std::pair<size_t, size_t> _protocol = _invalid;
            std::pair<size_t, size_t> _dir = _invalid;
            std::pair<size_t, size_t> _base = _invalid;
            std::pair<size_t, size_t> _num = _invalid;
            int _pad = 0;
            std::pair<size_t, size_t> _ext = _invalid;
            std::pair<size_t, size_t> _request = _invalid;
            std::optional<math::Int64Range> _frames;
        };

        //! Directory list sorting.
        enum class DirListSort
        {
            Name,
            Extension,
            Size,
            Time,
            
            Count,
            First = Name
        };
        TLRENDER_ENUM(DirListSort);

        //! Directory list options.
        struct DirListOptions
        {
            DirListSort              sort         = DirListSort::Name;
            bool                     sortReverse  = false;
            std::string              filter;
            bool                     filterFiles  = false;
            std::vector<std::string> filterExt;
            bool                     seq          = true;
            std::vector<std::string> seqExts;
            bool                     seqNegative  = true;
            size_t                   seqMaxDigits = 9;
            bool                     hidden       = false;

            bool operator == (const DirListOptions&) const;
            bool operator != (const DirListOptions&) const;
        };

        //! Directory list entry.
        struct DirEntry
        {
            Path                            path;
            bool                            isDir = false;
            size_t                          size = 0;
            std::filesystem::file_time_type time;

            bool operator == (const DirEntry&) const;
            bool operator != (const DirEntry&) const;
        };

        //! List directory contents.
        std::vector<DirEntry> dirList(
            const std::filesystem::path&,
            const DirListOptions& = DirListOptions());

        //! Expand a file sequence. This function will search the directory for
        //! other frames that match the given file name.
        Path expandSeq(
            const Path&,
            const PathOptions& = PathOptions());

        void to_json(nlohmann::json&, const PathOptions&);
        void to_json(nlohmann::json&, const DirListOptions&);

        void from_json(const nlohmann::json&, PathOptions&);
        void from_json(const nlohmann::json&, DirListOptions&);
        
        ///@}
    }
}

#include <tlCore/PathInline.h>
