// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <filesystem>
namespace fs = std::filesystem;

#include <tlCore/Path.h>


class ViewerUI;

namespace mrv
{
    namespace file
    {
        using tl::file::Path;
        using tl::file::PathOptions;

        /**
         * Given a tlRender's path, return whether the file can be loaded by
         * tlRender.
         *
         * @param path a tl::filePath.
         *
         * @return true if a known file to tlRender, false if not.
         */
        bool isValidType(const tl::file::Path& path);

        /**
         * Given a filename extension, return whether the extension can
         * be loaded by tlRender.
         *
         * @param extension Filename extension with period.
         *
         * @return true if a known file to tlRender, false if not.
         */
        bool isValidType(const std::string extension);

        /**
         * Given a lowercase filename extension, return whether the extension is
         * from a movie format.
         *
         * @param ext Filename extension.
         *
         * @return true if a possible movie, false if not.
         */
        bool isMovie(const std::string& ext);

        /**
         * Given a lowercase filename extension, return whether the extension is
         * from an audio format.
         *
         * @param ext Filename extension.
         *
         * @return true if a possible audio file, false if not.
         */
        bool isAudio(const std::string& ext);

        /**
         * Given a lowercase filename extension, return whether the extension is
         * from a subtitle format.
         *
         * @param ext Filename extension with period.
         *
         * @return true if a possible subtitle file, false if not.
         */
        bool isSubtitle(const std::string& ext);

        /**
         * Given a single image filename, return whether the image is
         * a sequence on disk (ie. there are several images named with a
         * similar convention)
         *
         * @param file Filename of image
         *
         * @return true if a possible sequence, false if not.
         */
        bool isSequence(const std::string& file);

        /**
         * Given a single filename, return whether the file is
         * a directory on disk
         *
         * @param directory possible path to a directory
         *
         * @return true if a directory, false if not.
         */
        bool isDirectory(const std::string& directory);

        /**
         * Return true if the file exists and is readable
         *
         * @param p std::filesystem path (or std::string).
         *
         * @return true if it exists and is readable, false if not.
         */
        bool isReadable(const fs::path& path);
        
        //! Returns an NDI filename for the current process
        std::string NDI(ViewerUI* ui);
        
        //! Returns true or false whether the filename is a temporary NDI file.
        bool isTemporaryNDI(const tl::file::Path& path);

        //! Returns true or false whether the filename is a temporary EDL.
        bool isTemporaryEDL(const tl::file::Path& path);
        
    } // namespace file

} // namespace mrv
