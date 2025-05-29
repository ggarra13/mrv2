// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <algorithm> // for std::sort

#include <tlCore/FileInfoPrivate.h>

#include <tlCore/String.h>

#ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#    define NOMINMAX
#endif // NOMINMAX
#include <sys/stat.h>
#include <windows.h>

namespace tl
{
    namespace file
    {
        bool FileInfo::_stat(std::string* error)
        {
            struct _stati64 info;
            memset(&info, 0, sizeof(struct _stati64));
            if (_wstati64(string::toWide(_path.get()).c_str(), &info) != 0)
            {
                if (error)
                {
                    char tmp[string::cBufferSize] = "";
                    strerror_s(tmp, string::cBufferSize, errno);
                    *error = tmp;
                }
                return false;
            }

            _exists = true;
            if (info.st_mode & _S_IFDIR)
            {
                _type = Type::Directory;
            }
            _size = info.st_size;
            _permissions |= (info.st_mode & _S_IREAD)
                                ? static_cast<int>(Permissions::Read)
                                : 0;
            _permissions |= (info.st_mode & _S_IWRITE)
                                ? static_cast<int>(Permissions::Write)
                                : 0;
            _permissions |= (info.st_mode & _S_IEXEC)
                                ? static_cast<int>(Permissions::Exec)
                                : 0;
            _time = info.st_mtime;

            return true;
        }

        void _list(
            const std::string& path, std::vector<FileInfo>& out,
            const ListOptions& options)
        {
            const std::string glob =
                appendSeparator(!path.empty() ? path : std::string(".")) + "*";
            WIN32_FIND_DATAW ffd;
            HANDLE hFind = FindFirstFileW(string::toWide(glob).c_str(), &ffd);
            if (hFind != INVALID_HANDLE_VALUE)
            {
                // Container to store file names
                std::vector<std::wstring> fileNames;

                // Collect file names
                do
                {
                    const std::wstring fileName(ffd.cFileName);
                    // Skip current and parent directories
                    if (fileName != L"." && fileName != L"..")
                    {
                        fileNames.push_back(fileName);
                    }
                } while (FindNextFileW(hFind, &ffd) != 0);

                FindClose(hFind);

                // Sort the file names alphabetically
                std::sort(fileNames.begin(), fileNames.end());

                // Process the sorted file names
                for (const auto& fileName : fileNames)
                {
                    const std::string fileNameStr = string::fromWide(fileName);
                    if (!listFilter(fileNameStr, options))
                    {
                        listSequence(path, fileNameStr, out, options);
                    }
                }
            }
        }

    } // namespace file
} // namespace tl
