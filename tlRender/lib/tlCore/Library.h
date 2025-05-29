// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Gonzalo Garramuño
// All rights reserved.

#ifdef _WIN32
#    define WIN32_LEAN_AND_MEAN // to avoid polluting with windows macros
#    include <windows.h>
#elif __linux__
#    include <dlfcn.h>
#elif __APPLE__
#    include <dlfcn.h>
#endif

#include <string>

#ifdef _WIN32
#    define strcasecmp(a, b) _stricmp(a, b)
using LibraryHandle = HMODULE;
#else
using LibraryHandle = void*;
#endif // _WIN32

namespace tl
{
    namespace library
    {
        LibraryHandle loadLibrary(const std::string& libraryPath);
        void* findFunction(
            LibraryHandle libraryHandle, const std::string& functionName);
        void closeLibrary(LibraryHandle libraryHandle);
    } // namespace library
} // namespace tl
