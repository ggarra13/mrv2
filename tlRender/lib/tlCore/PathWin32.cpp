// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/Path.h>

#ifndef WIN32_LEAN_AND_MEAN
#    define WIN32_LEAN_AND_MEAN
#endif // WIN32_LEAN_AND_MEAN
#ifndef NOMINMAX
#    define NOMINMAX
#endif // NOMINMAX
#include <windows.h>
#include <direct.h>
#include <Shlobj.h>
#include <shellapi.h>

#include <locale>

namespace tl
{
    namespace file
    {
        std::string getUserPath(UserPath value)
        {
            std::string out;
            KNOWNFOLDERID id;
            memset(&id, 0, sizeof(KNOWNFOLDERID));
            switch (value)
            {
            case UserPath::Home:
                id = FOLDERID_Profile;
                break;
            case UserPath::Desktop:
                id = FOLDERID_Desktop;
                break;
            case UserPath::Documents:
                id = FOLDERID_Documents;
                break;
            case UserPath::Downloads:
                id = FOLDERID_Downloads;
                break;
            default:
                break;
            }
            wchar_t* path = nullptr;
            HRESULT result = SHGetKnownFolderPath(id, 0, NULL, &path);

            if (S_OK == result && path)
            {
                // 1. Ask FLTK how many bytes the UTF-8 version needs.
                // Passing -1 as the length tells FLTK to look for the null terminator.
                int size = fl_utf8fromwc(nullptr, 0, path, -1);

                if (size > 0) {
                    out.resize(size);
                    // 2. Perform the actual conversion into the std::string buffer.
                    // size + 1 allows space for the internal null terminator during conversion.
                    fl_utf8fromwc(&out[0], size + 1, path, -1);
                } else {
                    out.clear();
                }
            }

            if (path) {
                CoTaskMemFree(path);
            }
            return out;
        }

        std::vector<std::string> getDrives() {
            std::vector<std::string> out;
    
            // 1. Get the required buffer size for the wide strings
            DWORD result = GetLogicalDriveStringsW(0, NULL);
            if (result > 0) {
                std::vector<WCHAR> buf(result);
                result = GetLogicalDriveStringsW(result, buf.data());

                if (result > 0) {
                    WCHAR* p = buf.data();
                    // GetLogicalDriveStringsW returns: "C:\\0D:\\0E:\\0\0"
                    // We loop until we hit the double-null terminator
                    while (*p) {
                        size_t wlen = wcslen(p);
                
                        // Determine UTF-8 size for this specific drive string
                        int utf8_sz = fl_utf8fromwc(nullptr, 0, p, (int)wlen);
                        if (utf8_sz > 0) {
                            std::string drive;
                            drive.resize(utf8_sz);
                            fl_utf8fromwc(&drive[0], utf8_sz + 1, p, (int)wlen);
                    
                            // Optional: remove trailing backslash if your app prefers "C:"
                            if (!drive.empty() && drive.back() == '\\') {
                                drive.pop_back();
                            }
                    
                            out.push_back(drive);
                        }
                
                        // Move to the next string in the multi-string buffer
                        p += wlen + 1;
                    }
                }
            }
            return out;
        }

    } // namespace file
} // namespace tl
