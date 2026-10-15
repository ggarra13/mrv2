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
#include <combaseapi.h>
#include <Shlobj.h>
#include <shellapi.h>


#include <FL/fl_utf8.h>

#include <locale>
#include <string>

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

        std::filesystem::path createTmpDir()
        {
            // The narrow GetTempPath() would give the temporary directory in
            // the ANSI code page, which u8path() then read as UTF-8; a user
            // whose account name is not ASCII got a name that was neither.
            std::filesystem::path out;
            WCHAR path[MAX_PATH];
            const DWORD r = GetTempPathW(MAX_PATH, path);
            if (r)
            {
                GUID guid;
                CoCreateGuid(&guid);
                const uint8_t* guidP = reinterpret_cast<const uint8_t*>(&guid);
                std::wstring name;
                for (int i = 0; i < 16; ++i)
                {
                    WCHAR buf[3] = L"";
                    swprintf_s(buf, 3, L"%02x", guidP[i]);
                    name += buf;
                }

                // The returned length excludes the terminator, and the path
                // already ends with a separator.
                out = std::filesystem::path(std::wstring(path, r) + name);
                CreateDirectoryW(out.c_str(), NULL);
            }
            return out;
        }
    } // namespace file
} // namespace tl
