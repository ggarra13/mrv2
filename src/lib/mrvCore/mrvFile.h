// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <filesystem>
namespace fs = std::filesystem;

#include <tlCore/String.h>
#include <tlCore/Path.h>

class ViewerUI;

namespace mrv
{
    namespace file
    {
        using tl::file::Path;
        using tl::file::PathOptions;

        /** 
         * Given a path with backslashes (\) returns a string with forward
         * slashes / replaced.
         * 
         * @param path path with backslashes
         * 
         * @return normalized path
         */
        std::string normalizePath(const std::string& path);
        
        /**
         * Given a tlRender's path, return whether the file can be loaded by
         * tlRender.
         *
         * @param path a tl::filePath.
         *
         * @return true if a known file to tlRender, false if not.
         */
        bool isValidType(const Path& path);

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

        inline bool isMovie(const Path& path)
        {
            return isMovie(tl::string::toLower(path.getExtension()));
        }

        /**
         * Given a lowercase filename extension, return whether the extension is
         * from an audio format.
         *
         * @param ext Filename extension.
         *
         * @return true if a possible audio file, false if not.
         */
        bool isAudio(const std::string& ext);

        inline bool isAudio(const Path& path)
        {
            return isAudio(tl::string::toLower(path.getExtension()));
        }

        /**
         * Given a lowercase filename extension, return whether the extension is
         * from a subtitle format.
         *
         * @param ext Filename extension with period.
         *
         * @return true if a possible subtitle file, false if not.
         */
        bool isSubtitle(const std::string& ext);

        inline bool isSubtitle(const Path& path)
        {
            return isSubtitle(tl::string::toLower(path.getExtension()));
        }

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

        inline bool isSequence(const Path& path)
        {
            return isSequence(path.get());
        }
        
        /**
         * Given a lowercase filename extension, return whether the extension is
         * from an OpenUSD scene.
         *
         * @param ext Filename extension with period.
         *
         * @return true if a possible subtitle file, false if not.
         */
        bool isUSD(const std::string& ext);

        inline bool isUSD(const Path& path)
        {
            return isSubtitle(tl::string::toLower(path.getExtension()));
        }

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
        bool exists(const fs::path& path);
        
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

        //! Returns whether the string is a network path.
        bool isNetwork(const std::string& path);

        //! Returns whether the string is a potential URI string.
        //! It must have at least one slash and one period.
        bool isPotentialURI(const std::string& path);
        
        //! Returns whether the filename is a temporary NDI file.
        bool isTemporaryNDI(const Path& path);

        //! Returns whether the filename is a temporary EDL.
        bool isTemporaryEDL(const Path& path);

        //! Returns whether the filename is an .otio or .otioz file.
        bool isOTIO(const Path& path);
            
        //! Returns whether command is in the PATH environment variable.
        bool isInPath(const std::string& command);
        
    } // namespace file

} // namespace mrv
