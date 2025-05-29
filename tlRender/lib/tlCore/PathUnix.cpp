// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/Path.h>

#include <tlCore/FileInfo.h>

#if defined(__APPLE__)
#    include <ApplicationServices/ApplicationServices.h>
#    include <CoreFoundation/CFBundle.h>
#    include <CoreServices/CoreServices.h>
#endif // __APPLE__

#if defined(__linux__)
#    include <linux/limits.h>
#endif // __linux__
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

namespace tl
{
    namespace file
    {
        std::string getUserPath(UserPath value)
        {
            std::string out;
#if defined(__APPLE__)
            OSType folderType = kDesktopFolderType;
            switch (value)
            {
            case UserPath::Home:
                folderType = kCurrentUserFolderType;
                break;
            case UserPath::Desktop:
                folderType = kDesktopFolderType;
                break;
            case UserPath::Documents:
                folderType = kDocumentsFolderType;
                break;
            case UserPath::Downloads:
                folderType = kCurrentUserFolderType;
                break;
            default:
                break;
            }
            FSRef ref;
            FSFindFolder(kUserDomain, folderType, kCreateFolder, &ref);
            char path[PATH_MAX];
            FSRefMakePath(&ref, (UInt8*)&path, PATH_MAX);
            if (UserPath::Downloads == value)
            {
                out = Path(path, "Downloads").get();
            }
            else
            {
                out = path;
            }
#else  // __APPLE__
            if (struct passwd* buf = ::getpwuid(::getuid()))
            {
                const std::string dir(buf->pw_dir);
                switch (value)
                {
                case UserPath::Home:
                    out = dir;
                    break;
                case UserPath::Desktop:
                    out = Path(dir, "Desktop").get();
                    break;
                case UserPath::Documents:
                    out = Path(dir, "Documents").get();
                    break;
                case UserPath::Downloads:
                    out = Path(dir, "Downloads").get();
                    break;
                default:
                    break;
                }
            }
#endif // __APPLE__
            return out;
        }

        std::vector<std::string> getDrives()
        {
            std::vector<std::string> out;
            out.push_back("/");
            std::vector<FileInfo> list;
#if defined(__APPLE__)
            file::list("/Volumes", list);
            for (const auto& fileInfo : list)
            {
                out.push_back(fileInfo.getPath().get());
            }
#else  // __APPLE__
            file::list("/mnt", list);
            for (const auto& fileInfo : list)
            {
                out.push_back(fileInfo.getPath().get());
            }
#endif // __APPLE__
            return out;
        }
    } // namespace file
} // namespace tl
