// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/FileIO.h>

#include <tlCore/Error.h>
#include <tlCore/Memory.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <atomic>
#include <cstring>
#include <exception>

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
        namespace
        {
            enum class ErrorType {
                Open,
                OpenTemp,
                MemoryMap,
                Close,
                CloseMemoryMap,
                Read,
                ReadMemoryMap,
                Write,
                Seek,
                SeekMemoryMap
            };

            std::string getErrorMessage(
                ErrorType type, const std::string& path,
                const std::string& message = std::string())
            {
                std::string out;
                switch (type)
                {
                case ErrorType::Open:
                    out = string::Format("{0}: Cannot open file").arg(path);
                    break;
                case ErrorType::OpenTemp:
                    out = string::Format("Cannot open temporary file");
                    break;
                case ErrorType::MemoryMap:
                    out =
                        string::Format("{0}: Cannot memory map").arg(path);
                    break;
                case ErrorType::Close:
                    out = string::Format("{0}: Cannot close").arg(path);
                    break;
                case ErrorType::CloseMemoryMap:
                    out = string::Format("{0}: Cannot unmap").arg(path);
                    break;
                case ErrorType::Read:
                    out = string::Format("{0}: Cannot read").arg(path);
                    break;
                case ErrorType::ReadMemoryMap:
                    out = string::Format("{0}: Cannot read memory map")
                              .arg(path);
                    break;
                case ErrorType::Write:
                    out = string::Format("{0}: Cannot write").arg(path);
                    break;
                case ErrorType::Seek:
                    out = string::Format("{0}: Cannot seek").arg(path);
                    break;
                case ErrorType::SeekMemoryMap:
                    out = string::Format("{0}: Cannot seek memory map")
                              .arg(path);
                    break;
                default:
                    break;
                }
                if (!message.empty())
                {
                    out = string::Format("{0}: {1}").arg(out).arg(message);
                }
                return out;
            }

        } // namespace

        struct FileIO::Private
        {
            void seek(size_t, SeekMode);

            std::filesystem::path path;
            Mode mode = Mode::First;
            Read readType = Read::First;
            size_t pos = 0;
            size_t size = 0;
            bool endianConversion = false;
            HANDLE f = INVALID_HANDLE_VALUE;
            void* memoryMap = nullptr;
            const uint8_t* memoryStart = nullptr;
            const uint8_t* memoryEnd = nullptr;
            const uint8_t* memoryP = nullptr;
        };

        namespace
        {
            std::atomic<size_t> objectCount = 0;
        }

        FileIO::FileIO() :
            _p(new Private)
        {
            ++objectCount;
        }

        FileIO::~FileIO()
        {
            _close();
            --objectCount;
        }

        std::shared_ptr<FileIO>
        FileIO::create(const std::filesystem::path& path, const MemoryRead& memory)
        {
            auto out = std::shared_ptr<FileIO>(new FileIO);
            out->_p->path = path;
            out->_p->mode = Mode::Read;
            out->_p->readType = Read::Normal;
            out->_p->size = memory.size;
            out->_p->memoryStart = memory.p;
            out->_p->memoryEnd = memory.p + memory.size;
            out->_p->memoryP = memory.p;
            return out;
        }

        size_t FileIO::getObjectCount()
        {
            return objectCount;
        }

        bool FileIO::isOpen() const
        {
            return _p->f != INVALID_HANDLE_VALUE || _p->memoryStart;
        }

        const std::filesystem::path& FileIO::getPath() const
        {
            return _p->path;
        }

        size_t FileIO::getSize() const
        {
            return _p->size;
        }

        size_t FileIO::getPos() const
        {
            return _p->pos;
        }

        void FileIO::seek(size_t in, SeekMode mode)
        {
            _p->seek(in, mode);
        }

        const uint8_t* FileIO::getMemoryStart() const
        {
            return _p->memoryStart;
        }

        const uint8_t* FileIO::getMemoryEnd() const
        {
            return _p->memoryEnd;
        }

        const uint8_t* FileIO::getMemoryP() const
        {
            return _p->memoryP;
        }

        bool FileIO::hasEndianConversion() const
        {
            return _p->endianConversion;
        }

        void FileIO::setEndianConversion(bool in)
        {
            _p->endianConversion = in;
        }

        bool FileIO::isEOF() const
        {
            TLRENDER_P();
            bool out = false;
            if (!p.memoryStart)
            {
                out |= p.f == INVALID_HANDLE_VALUE;
            }
            out |= p.pos >= p.size;
            return out;
        }

        void FileIO::read(void* in, size_t size, size_t wordSize)
        {
            TLRENDER_P();

            if (!p.memoryStart && !p.f)
            {
                throw std::runtime_error(
                    getErrorMessage(ErrorType::Read, p.path.u8string()));
            }

            switch (p.mode)
            {
            case Mode::Read:
            {
                if (p.memoryStart)
                {
                    const uint8_t* memoryP = p.memoryP + size * wordSize;
                    if (memoryP > p.memoryEnd)
                    {
                        throw std::runtime_error(
                            getErrorMessage(
                                ErrorType::ReadMemoryMap, p.path.u8string()));
                    }
                    if (p.endianConversion && wordSize > 1)
                    {
                        memory::swapEndian(p.memoryP, in, size, wordSize);
                    }
                    else
                    {
                        std::memcpy(in, p.memoryP, size * wordSize);
                    }
                    p.memoryP = memoryP;
                }
                else
                {
                    DWORD n;
                    if (!::ReadFile(
                            p.f, in, static_cast<DWORD>(size * wordSize), &n,
                            0))
                    {
                        throw std::runtime_error(
                            getErrorMessage(
                                ErrorType::Read, p.path.u8string(),
                                error::getLastError()));
                    }
                    if (p.endianConversion && wordSize > 1)
                    {
                        memory::swapEndian(in, size, wordSize);
                    }
                }
                break;
            }
            case Mode::ReadWrite:
            {
                DWORD n;
                if (!::ReadFile(
                        p.f, in, static_cast<DWORD>(size * wordSize), &n, 0))
                {
                    throw std::runtime_error(
                        getErrorMessage(
                            ErrorType::Read, p.path.u8string(),
                            error::getLastError()));
                }
                if (p.endianConversion && wordSize > 1)
                {
                    memory::swapEndian(in, size, wordSize);
                }
                break;
            }
            default:
                break;
            }
            p.pos += size * wordSize;
        }

        void FileIO::readAt(void* in, size_t pos, size_t size, size_t wordSize) const
        {
            TLRENDER_P();

            if (p.mode != Mode::Read && p.mode != Mode::ReadWrite)
            {
                throw std::runtime_error(
                    getErrorMessage(ErrorType::Read, p.path.u8string()));
            }

            const size_t byteCount = size * wordSize;
            if (pos > p.size || byteCount > p.size - pos)
            {
                throw std::runtime_error(
                    getErrorMessage(
                        p.memoryStart ?
                        ErrorType::ReadMemoryMap :
                        ErrorType::Read,
                        p.path.u8string()));
            }

            if (p.memoryStart)
            {
                if (p.endianConversion && wordSize > 1)
                {
                    memory::swapEndian(p.memoryStart + pos, in, size, wordSize);
                }
                else
                {
                    std::memcpy(in, p.memoryStart + pos, byteCount);
                }
            }
            else if (p.f != INVALID_HANDLE_VALUE)
            {
                uint8_t* out = reinterpret_cast<uint8_t*>(in);
                size_t remaining = byteCount;
                uint64_t offset = pos;
                while (remaining > 0)
                {
                    // The offset goes in an OVERLAPPED rather than a seek so that
                    // two threads cannot interleave between the two calls. The
                    // handle is synchronous, so this still moves the shared file
                    // position, and Windows serializes the reads; the position is
                    // put back below to keep read() working afterwards.
                    OVERLAPPED overlapped;
                    std::memset(&overlapped, 0, sizeof(overlapped));
                    overlapped.Offset     = static_cast<DWORD>(offset);
                    overlapped.OffsetHigh = static_cast<DWORD>(offset >> 32);
                    const DWORD request = static_cast<DWORD>(std::min<size_t>(
                                                                 remaining,
                                                                 std::numeric_limits<DWORD>::max()));
                    DWORD n = 0;
                    if (!::ReadFile(p.f, out, request, &n, &overlapped))
                    {
                        throw std::runtime_error(
                            getErrorMessage(ErrorType::Read, p.path.u8string(),
                                            error::getLastError()));
                    }
                    if (0 == n)
                    {
                        throw std::runtime_error(
                            getErrorMessage(ErrorType::Read, p.path.u8string()));
                    }
                    out       += n;
                    offset    += n;
                    remaining -= n;
                }
                LARGE_INTEGER v;
                v.QuadPart = p.pos;
                if (!::SetFilePointerEx(p.f, v, 0, FILE_BEGIN))
                {
                    throw std::runtime_error(
                        getErrorMessage(ErrorType::Seek, p.path.u8string(),
                                        error::getLastError()));
                }
                if (p.endianConversion && wordSize > 1)
                {
                    memory::swapEndian(in, size, wordSize);
                }
            }
            else
            {
                throw std::runtime_error(
                    getErrorMessage(ErrorType::Read, p.path.u8string()));
            }
        }

        void FileIO::write(const void* in, size_t size, size_t wordSize)
        {
            TLRENDER_P();

            if (!p.f)
            {
                throw std::runtime_error(
                    getErrorMessage(ErrorType::Write, p.path.u8string()));
            }

            const uint8_t* inP = reinterpret_cast<const uint8_t*>(in);
            std::vector<uint8_t> tmp;
            if (p.endianConversion && wordSize > 1)
            {
                tmp.resize(size * wordSize);
                memory::swapEndian(in, tmp.data(), size, wordSize);
                inP = tmp.data();
            }

            DWORD n = 0;
            if (!::WriteFile(
                    p.f, inP, static_cast<DWORD>(size * wordSize), &n, 0))
            {
                throw std::runtime_error(getErrorMessage(
                    ErrorType::Write, p.path.u8string(), error::getLastError()));
            }
            p.pos += size * wordSize;
            p.size = std::max(p.pos, p.size);
        }

        void
        FileIO::_open(const std::filesystem::path& path, Mode mode,
                      Read readType, Access access)
        {
            TLRENDER_P();

            _close();

            // Open the file.
            DWORD desiredAccess = 0;
            DWORD shareMode = 0;
            DWORD disposition = 0;
            // The read ahead hint. These are only settable here, when the file is
            // opened, so it cannot be changed later the way madvise() can.
            DWORD flags = Access::Random == access ?
                          FILE_FLAG_RANDOM_ACCESS :
                          FILE_FLAG_SEQUENTIAL_SCAN;
            switch (mode)
            {
            case Mode::Read:
                desiredAccess = GENERIC_READ;
                shareMode = FILE_SHARE_READ;
                disposition = OPEN_EXISTING;
                break;
            case Mode::Write:
                desiredAccess = GENERIC_WRITE;
                disposition = CREATE_ALWAYS;
                break;
            case Mode::ReadWrite:
                desiredAccess = GENERIC_READ | GENERIC_WRITE;
                shareMode = FILE_SHARE_READ;
                disposition = OPEN_EXISTING;
                break;
            case Mode::Append:
                desiredAccess = GENERIC_WRITE;
                disposition = OPEN_EXISTING;
                break;
            default:
                break;
            }
            try
            {
                p.f = CreateFileW(
                    path.wstring().c_str(), desiredAccess, shareMode, 0, disposition,
                    flags, 0);
            }
            catch (const std::exception&)
            {
                p.f = INVALID_HANDLE_VALUE;
            }
            if (INVALID_HANDLE_VALUE == p.f)
            {
                throw std::runtime_error(
                    getErrorMessage(
                        ErrorType::Open, path.u8string(), error::getLastError()));
            }
            p.path = path;
            p.mode = mode;
            p.readType = readType;
            p.pos = 0;
            p.size = std::filesystem::file_size(path);

            // Memory mapping.
            if (Read::MemoryMapped == p.readType &&
                Mode::Read == p.mode &&
                p.size > 0)
            {
                p.memoryMap = CreateFileMapping(p.f, 0, PAGE_READONLY, 0, 0, 0);
                if (!p.memoryMap)
                {
                    throw std::runtime_error(
                        getErrorMessage(ErrorType::MemoryMap, path.u8string(),
                                        error::getLastError()));
                }

                p.memoryStart = reinterpret_cast<const uint8_t*>(MapViewOfFile(p.memoryMap, FILE_MAP_READ, 0, 0, 0));
                if (!p.memoryStart)
                {
                    throw std::runtime_error(
                        getErrorMessage(ErrorType::MemoryMap, path.u8string()));
                }

                p.memoryEnd = p.memoryStart + p.size;
                p.memoryP = p.memoryStart;
            }

            // Seek to the end when appending.
            if (Mode::Append == mode)
            {
                seek(p.size, SeekMode::Forward);
            }
        }

        bool FileIO::_close(std::string* error)
        {
            TLRENDER_P();

            bool out = true;

            if (p.memoryMap)
            {
                if (p.memoryStart)
                {
                    if (!::UnmapViewOfFile((void*)p.memoryStart))
                    {
                        out = false;
                        if (error)
                        {
                            *error = getErrorMessage(
                                ErrorType::CloseMemoryMap, p.path.u8string(),
                                error::getLastError());
                        }
                    }
                    p.memoryStart = nullptr;
                }

                if (!::CloseHandle(p.memoryMap))
                {
                    out = false;
                    if (error)
                    {
                        *error = getErrorMessage(
                            ErrorType::Close, p.path.u8string(),
                            error::getLastError());
                    }
                }
                p.memoryMap = nullptr;
            }
            p.memoryEnd = nullptr;
            p.memoryP = nullptr;

            if (p.f != INVALID_HANDLE_VALUE)
            {
                CloseHandle(p.f);
                p.f = INVALID_HANDLE_VALUE;
            }

            p.path = std::filesystem::path();
            p.mode = Mode::First;
            p.pos = 0;
            p.size = 0;

            return out;
        }

        void FileIO::Private::seek(size_t value, SeekMode seekMode)
        {
            if (Mode::Read == mode && memoryStart)
            {
                switch(seekMode)
                {
                case SeekMode::Set:
                    memoryP =
                        reinterpret_cast<const uint8_t*>(memoryStart) +
                        value;
                    if (memoryP > memoryEnd)
                    {
                        throw std::runtime_error(
                            getErrorMessage(
                                ErrorType::SeekMemoryMap, path.u8string()));
                    }
                    break;
                case SeekMode::Forward:
                    memoryP += value;
                    if (memoryP > memoryEnd)
                    {
                        throw std::runtime_error(
                            getErrorMessage(ErrorType::SeekMemoryMap,
                                            path.u8string()));
                    }
                    break;
                case SeekMode::Reverse:
                    memoryP -= value;
                    if (memoryP < memoryStart)
                    {
                        throw std::runtime_error(
                            getErrorMessage(ErrorType::SeekMemoryMap,
                                            path.u8string()));
                    }
                    break;
                default: break;
                }
            }
            else
            {
                LARGE_INTEGER v;
                v.QuadPart = value;
                DWORD move = FILE_BEGIN;
                switch (seekMode)
                {
                case SeekMode::Forward:
                    move = FILE_CURRENT;
                    break;
                case SeekMode::Reverse:
                    v.QuadPart = -v.QuadPart;
                    move = FILE_CURRENT;
                    break;
                default: break;
                }
                if (!::SetFilePointerEx(f, v, 0, move))
                {
                    throw std::runtime_error(
                        getErrorMessage(
                            ErrorType::Seek, path.u8string(),
                            error::getLastError()));
                }
            }

            switch (seekMode)
            {
            case SeekMode::Set: pos = value; break;
            case SeekMode::Forward: pos += value; break;
            case SeekMode::Reverse: pos -= value; break;
            default: break;
            }
        }

        void prefetch(const void* p, size_t size)
        {
            if (p && size > 0)
            {
                WIN32_MEMORY_RANGE_ENTRY range;
                range.VirtualAddress = const_cast<void*>(p);
                range.NumberOfBytes = size;
                PrefetchVirtualMemory(GetCurrentProcess(), 1, &range, 0);
            }
        }

        void truncate(const std::filesystem::path& path, size_t size)
        {
            HANDLE h = INVALID_HANDLE_VALUE;
            try
            {
                h = CreateFileW(
                    path.wstring().c_str(),
                    GENERIC_WRITE,
                    0,
                    0,
                    OPEN_EXISTING,
                    FILE_ATTRIBUTE_NORMAL,
                    0);
            }
            catch (const std::exception&)
            {
                h = INVALID_HANDLE_VALUE;
            }
            if (INVALID_HANDLE_VALUE == h)
            {
                throw std::runtime_error(
                    getErrorMessage(ErrorType::Open, path.u8string(),
                                    error::getLastError()));
            }
            LARGE_INTEGER v;
            v.QuadPart = size;
            if (!::SetFilePointerEx(
                    h,
                    static_cast<LARGE_INTEGER>(v),
                    0,
                    FILE_BEGIN))
            {
                CloseHandle(h);
                throw std::runtime_error(
                    getErrorMessage(ErrorType::Seek, path.u8string(),
                                    error::getLastError()));
            }
            if (!::SetEndOfFile(h))
            {
                CloseHandle(h);
                throw std::runtime_error(
                    getErrorMessage(ErrorType::Write, path.u8string(),
                                    error::getLastError()));
            }
            CloseHandle(h);
        }
    } // namespace file
} // namespace tl
