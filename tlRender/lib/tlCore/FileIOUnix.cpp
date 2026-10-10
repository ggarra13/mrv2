// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

// Request a 64-bit off_t on 32-bit Linux/BSD targets. This must come before
// any system header. (macOS is always 64-bit, so nothing is needed there.)
#if !defined(__APPLE__) && !defined(_FILE_OFFSET_BITS)
#    define _FILE_OFFSET_BITS 64
#endif

#include <tlCore/FileIO.h>

#include <tlCore/FileInfoPrivate.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Memory.h>
#include <tlCore/Path.h>

#include <algorithm>
#include <atomic>
#include <cstdint>
#include <filesystem>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

namespace tl
{
    namespace file
    {
        namespace
        {
            enum class ErrorType
            {
                Open,
                MMap,
                Close,
                CloseMMap,
                Read,
                ReadMMap,
                Write,
                Seek,
                SeekMMap
            };

            std::string getErrorString()
            {
                std::string out;
                char buf[string::cBufferSize] = "";
#if defined(_GNU_SOURCE)
                out = strerror_r(errno, buf, string::cBufferSize);
#else // _GNU_SOURCE
                strerror_r(errno, buf, string::cBufferSize);
                out = buf;
#endif // _GNU_SOURCE
                return out;
            }

            std::string getErrorMessage(
                ErrorType          type,
                const std::string& path,
                const std::string& message  = std::string())
            {
                std::string out;
                switch (type)
                {
                case ErrorType::Open:
                    out = string::Format("Cannot open file \"{0}\"").arg(path);
                    break;
                case ErrorType::MMap:
                    out = string::Format("Cannot memory map file \"{0}\"").arg(path);
                    break;
                case ErrorType::Close:
                    out = string::Format("Cannot close file \"{0}\"").arg(path);
                    break;
                case ErrorType::CloseMMap:
                    out = string::Format("Cannot unmap file \"{0}\"").arg(path);
                    break;
                case ErrorType::Read:
                    out = string::Format("Cannot read file \"{0}\"").arg(path);
                    break;
                case ErrorType::ReadMMap:
                    out = string::Format("Cannot read memory mapped file \"{0}\"").arg(path);
                    break;
                case ErrorType::Write:
                    out = string::Format("Cannot write file \"{0}\"").arg(path);
                    break;
                case ErrorType::Seek:
                    out = string::Format("Cannot seek file \"{0}\"").arg(path);
                    break;
                case ErrorType::SeekMMap:
                    out = string::Format("Cannot seek memory mapped file \"{0}\"").arg(path);
                    break;
                default: break;
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
            Mode                  mode = Mode::First;
            Read                  readType = Read::First;
            size_t                pos = 0;
            size_t                size = 0;
            bool                  endianConversion = false;
            int                   f = -1;
            void*                 mMap = reinterpret_cast<void*>(-1);
            const uint8_t*        memStart = nullptr;
            const uint8_t*        memEnd = nullptr;
            const uint8_t*        memP = nullptr;
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

        std::shared_ptr<FileIO> FileIO::create(
            const std::filesystem::path& path,
            const MemoryRead& memFile)
        {
            auto out = std::shared_ptr<FileIO>(new FileIO);
            out->_p->path = path;
            out->_p->mode = Mode::Read;
            out->_p->readType = Read::Normal;
            out->_p->size = memFile.size;
            out->_p->memStart = memFile.p;
            out->_p->memEnd = memFile.p + memFile.size;
            out->_p->memP = memFile.p;
            return out;
        }

        bool FileIO::isOpen() const
        {
            return _p->f != -1 || _p->memStart;
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
            return _p->memStart;
        }

        const uint8_t* FileIO::getMemoryEnd() const
        {
            return _p->memEnd;
        }

        const uint8_t* FileIO::getMemoryP() const
        {
            return _p->memP;
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
            if (!p.memStart)
            {
                out |= -1 == p.f;
            }
            out |= p.pos >= p.size;
            return out;
        }

        void FileIO::read(void* in, size_t size, size_t wordSize)
        {
            TLRENDER_P();

            if (!p.memStart && -1 == p.f)
            {
                throw std::runtime_error(
                    getErrorMessage(ErrorType::Read, fromFileSystem(p.path)));
            }

            switch (p.mode)
            {
            case Mode::Read:
            {
                if (p.memStart)
                {
                    const uint8_t* memP = p.memP + size * wordSize;
                    if (memP > p.memEnd)
                    {
                        throw std::runtime_error(
                            getErrorMessage(ErrorType::ReadMMap, fromFileSystem(p.path)));
                    }
                    if (p.endianConversion && wordSize > 1)
                    {
                        memory::swapEndian(p.memP, in, size, wordSize);
                    }
                    else
                    {
                        memcpy(in, p.memP, size * wordSize);
                    }
                    p.memP = memP;
                }
                else
                {
                    const ssize_t r = ::read(p.f, in, size * wordSize);
                    if (r < 0)
                    {
                        throw std::runtime_error(
                            getErrorMessage(ErrorType::Read, fromFileSystem(p.path), getErrorString()));
                    }
                    else if (static_cast<size_t>(r) != size * wordSize)
                    {
                        throw std::runtime_error(
                            getErrorMessage(ErrorType::Read, fromFileSystem(p.path)));
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
                const ssize_t r = ::read(p.f, in, size * wordSize);
                if (r < 0)
                {
                    throw std::runtime_error(
                        getErrorMessage(ErrorType::Read, fromFileSystem(p.path), getErrorString()));
                }
                else if (static_cast<size_t>(r) != size * wordSize)
                {
                    throw std::runtime_error(
                        getErrorMessage(ErrorType::Read, fromFileSystem(p.path)));
                }
                if (p.endianConversion && wordSize > 1)
                {
                    memory::swapEndian(in, size, wordSize);
                }
                break;
            }
            default: break;
            }
            p.pos += size * wordSize;
        }

        void FileIO::readAt(void* in, size_t pos, size_t size, size_t wordSize) const
        {
            TLRENDER_P();

            if (p.mode != Mode::Read && p.mode != Mode::ReadWrite)
            {
                throw std::runtime_error(
                    getErrorMessage(ErrorType::Read, fromFileSystem(p.path)));
            }

            const size_t byteCount = size * wordSize;
            if (pos > p.size || byteCount > p.size - pos)
            {
                throw std::runtime_error(
                    getErrorMessage(
                        p.memStart ? ErrorType::ReadMMap : ErrorType::Read,
                        fromFileSystem(p.path)));
            }

            if (p.memStart)
            {
                if (p.endianConversion && wordSize > 1)
                {
                    memory::swapEndian(p.memStart + pos, in, size, wordSize);
                }
                else
                {
                    memcpy(in, p.memStart + pos, byteCount);
                }
            }
            else if (p.f != -1)
            {
                uint8_t* out = reinterpret_cast<uint8_t*>(in);
                size_t remaining = byteCount;
                off_t offset = pos;
                while (remaining > 0)
                {
                    // A short read is not an error; large reads get broken up.
                    const ssize_t r = ::pread(p.f, out, remaining, offset);
                    if (r < 0)
                    {
                        throw std::runtime_error(
                            getErrorMessage(ErrorType::Read, fromFileSystem(p.path), getErrorString()));
                    }
                    else if (0 == r)
                    {
                        throw std::runtime_error(
                            getErrorMessage(ErrorType::Read, fromFileSystem(p.path)));
                    }
                    out       += r;
                    offset    += r;
                    remaining -= r;
                }
                if (p.endianConversion && wordSize > 1)
                {
                    memory::swapEndian(in, size, wordSize);
                }
            }
            else
            {
                throw std::runtime_error(
                    getErrorMessage(ErrorType::Read, fromFileSystem(p.path)));
            }
        }

        void FileIO::write(const void* in, size_t size, size_t wordSize)
        {
            TLRENDER_P();

            if (-1 == p.f)
            {
                throw std::runtime_error(
                    getErrorMessage(ErrorType::Write, fromFileSystem(p.path)));
            }

            const uint8_t* inP = reinterpret_cast<const uint8_t*>(in);
            std::vector<uint8_t> tmp;
            if (p.endianConversion && wordSize > 1)
            {
                tmp.resize(size * wordSize);
                memory::swapEndian(in, tmp.data(), size, wordSize);
                inP = tmp.data();
            }
            if (::write(p.f, inP, size * wordSize) == -1)
            {
                throw std::runtime_error(
                    getErrorMessage(ErrorType::Write, fromFileSystem(p.path), getErrorString()));
            }
            p.pos += size * wordSize;
            p.size = std::max(p.pos, p.size);
        }

        size_t FileIO::getObjectCount()
        {
            return objectCount;
        }

        void FileIO::_open(
            const std::filesystem::path& path,
            Mode mode,
            Read readType,
            Access access)
        {
            TLRENDER_P();

            _close();

            // Open the file.
            int openFlags = 0;
            int openMode  = 0;
            switch (mode)
            {
            case Mode::Read:
                openFlags = O_RDONLY;
                break;
            case Mode::Write:
                openFlags = O_WRONLY | O_CREAT | O_TRUNC;
                openMode  = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
                break;
            case Mode::ReadWrite:
                openFlags = O_RDWR | O_CREAT;
                openMode  = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
                break;
            case Mode::Append:
                openFlags = O_WRONLY | O_CREAT | O_APPEND;
                openMode  = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
                break;
            default: break;
            }
            p.f = ::open(fromFileSystem(path).c_str(), openFlags, openMode);
            if (-1 == p.f)
            {
                throw std::runtime_error(
                    getErrorMessage(ErrorType::Open, fromFileSystem(path), getErrorString()));
            }

            // File information.
            p.path     = path;
            p.mode     = mode;
            p.readType = readType;
            p.pos      = 0;
            p.size     = std::filesystem::file_size(path);

            // Memory mapping.
            if (Read::MemoryMapped == p.readType &&
                Mode::Read == p.mode &&
                p.size > 0)
            {
                p.mMap = mmap(0, p.size, PROT_READ, MAP_SHARED, p.f, 0);
                madvise(
                    p.mMap,
                    p.size,
                    Access::Random == access ? MADV_RANDOM : MADV_SEQUENTIAL);
                if (p.mMap == (void*)-1)
                {
                    throw std::runtime_error(
                        getErrorMessage(ErrorType::MMap, fromFileSystem(path), getErrorString()));
                }
                p.memStart = reinterpret_cast<const uint8_t*>(p.mMap);
                p.memEnd   = p.memStart + p.size;
                p.memP        = p.memStart;
            }
        }

        bool FileIO::_close(std::string* error)
        {
            TLRENDER_P();

            bool out = true;

            if (p.mMap != (void*)-1)
            {
                int r = munmap(p.mMap, p.size);
                if (-1 == r)
                {
                    out = false;
                    if (error)
                    {
                        *error = getErrorMessage(ErrorType::CloseMMap, fromFileSystem(p.path), getErrorString());
                    }
                }
                p.mMap = (void*)-1;
            }
            p.memStart = nullptr;
            p.memEnd   = nullptr;

            if (p.f != -1)
            {
                int r = ::close(p.f);
                if (-1 == r)
                {
                    out = false;
                    if (error)
                    {
                        *error = getErrorMessage(ErrorType::Close, fromFileSystem(p.path), getErrorString());
                    }
                }
                p.f = -1;
            }

            p.path = std::filesystem::path();
            p.mode = Mode::First;
            p.pos  = 0;
            p.size = 0;

            return out;
        }

        void FileIO::Private::seek(size_t value, SeekMode seekMode)
        {
            if (Mode::Read == mode && memStart)
            {
                switch (seekMode)
                {
                case SeekMode::Set:
                    memP = reinterpret_cast<const uint8_t*>(memStart) + value;
                    if (memP > memEnd)
                    {
                        throw std::runtime_error(
                            getErrorMessage(ErrorType::SeekMMap, fromFileSystem(path)));
                    }
                    break;
                case SeekMode::Forward:
                    memP += value;
                    if (memP > memEnd)
                    {
                        throw std::runtime_error(
                            getErrorMessage(ErrorType::SeekMMap, fromFileSystem(path)));
                    }
                    break;
                case SeekMode::Reverse:
                    memP -= value;
                    if (memP < memStart)
                    {
                        throw std::runtime_error(
                            getErrorMessage(ErrorType::SeekMMap, fromFileSystem(path)));
                    }
                    break;
                default: break;
                }
            }
            else
            {
                off_t offset = value;
                int whence = SEEK_SET;
                switch (seekMode)
                {
                case SeekMode::Forward:
                    whence = SEEK_CUR;
                    break;
                case SeekMode::Reverse:
                    offset = -offset;
                    whence = SEEK_CUR;
                    break;
                default: break;
                }
                if (::lseek(f, offset, whence) == (off_t)-1)
                {
                    throw std::runtime_error(
                        getErrorMessage(ErrorType::Seek, fromFileSystem(path), getErrorString()));
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
            if (!p || 0 == size)
                return;
#if defined(__linux__)
            // From the start of the page: madvise() fails on an address that is
            // not page aligned, and data inside a file -- a frame inside a
            // bundle above all -- rarely starts on a page. The prefetch failed
            // for nearly every frame, and with read ahead off for the bundle
            // each frame came in as thousands of synchronous four kilobyte
            // reads: a 7 GB/s drive read about 200 MB/s with the cores idle.
            //
            // And a window at a time: Linux reads no more than about its read
            // ahead window for each request, 128 KB unless the device says
            // otherwise, so a frame asked for in one request comes in only at
            // its start.
            static const uintptr_t pageSize = static_cast<uintptr_t>(sysconf(_SC_PAGESIZE));
            const uintptr_t start = reinterpret_cast<uintptr_t>(p) & ~(pageSize - 1);
            const uintptr_t end = reinterpret_cast<uintptr_t>(p) + size;
            const uintptr_t chunk = 128 * 1024;
            for (uintptr_t i = start; i < end; i += chunk)
            {
                madvise(
                    reinterpret_cast<void*>(i),
                    std::min(chunk, end - i),
                    MADV_WILLNEED);
            }
#else // __linux__
            // Left as it was on macOS, where the kernel's own read ahead keeps
            // up with the bundle; an aligned prefetch there made no difference
            // that stood out from the run to run noise.
            madvise(const_cast<void*>(p), size, MADV_WILLNEED);
#endif // __linux__
        }

        void release(const void* p, size_t size)
        {
            if (!p || 0 == size)
                return;
            // The whole pages the range touches: a page shared with the next
            // frame is only unmapped, not discarded, and reading it again faults
            // it back in from the file cache.
            static const uintptr_t pageSize = static_cast<uintptr_t>(sysconf(_SC_PAGESIZE));
            const uintptr_t start = reinterpret_cast<uintptr_t>(p) & ~(pageSize - 1);
            const uintptr_t end = reinterpret_cast<uintptr_t>(p) + size;
            madvise(reinterpret_cast<void*>(start), end - start, MADV_DONTNEED);
        }

        void truncateFile(const std::filesystem::path& path, size_t size)
        {
            if (::truncate(fromFileSystem(path).c_str(), size) != 0)
            {
                throw std::runtime_error(
                    getErrorMessage(ErrorType::Write, fromFileSystem(path), getErrorString()));
            }
        }

    } // namespace file
} // namespace tl
