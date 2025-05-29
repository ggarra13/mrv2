// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Gonzalo Garramuño
// All rights reserved.

#include <tlCore/Library.h>

#include <stdexcept>

namespace tl
{
    namespace library
    {
        // Function to load the library
        LibraryHandle loadLibrary(const std::string& libraryPath)
        {
#ifdef _WIN32
            LibraryHandle libraryHandle = LoadLibraryA(libraryPath.c_str());
            if (libraryHandle == nullptr)
            {
                throw std::runtime_error(
                    "Failed to load library: " + libraryPath);
            }
            return libraryHandle;
#elif __linux__ || __APPLE__
            LibraryHandle libraryHandle =
                dlopen(libraryPath.c_str(), RTLD_LAZY);
            if (libraryHandle == nullptr)
            {
                throw std::runtime_error(
                    "Failed to load library: " + libraryPath +
                    ", error: " + (dlerror() ? dlerror() : "unknown"));
            }
            return libraryHandle;
#else
            throw std::runtime_error("Unsupported platform.");
#endif
        }

        void* findFunction(
            LibraryHandle libraryHandle, const std::string& functionName)
        {
#ifdef _WIN32
            FARPROC functionPointer =
                GetProcAddress(libraryHandle, functionName.c_str());
            if (functionPointer == nullptr)
            {
                throw std::runtime_error(
                    "Failed to find function: " + functionName);
            }
            return reinterpret_cast<void*>(functionPointer);
#elif __linux__ || __APPLE__
            void* functionPointer = dlsym(libraryHandle, functionName.c_str());
            if (functionPointer == nullptr)
            {
                throw std::runtime_error(
                    "Failed to find function: " + functionName +
                    ", error: " + (dlerror() ? dlerror() : "unknown"));
            }
            return functionPointer;
#else
            throw std::runtime_error("Unsupported platform.");
#endif
        }

        void closeLibrary(LibraryHandle libraryHandle)
        {
#ifdef _WIN32
            if (libraryHandle != nullptr)
            {
                FreeLibrary(libraryHandle);
            }
#elif __linux__ || __APPLE__
            if (libraryHandle != nullptr)
            {
                dlclose(libraryHandle);
            }
#endif
        }
    } // namespace library
} // namespace tl
