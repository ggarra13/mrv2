// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/FileIO.h>

#include <tlCore/File.h>
#include <tlCore/Memory.h>
#include <tlCore/StringFormat.h>

#include <atomic>

#if defined(__linux__)
#    include <linux/limits.h>
#endif // __linux__
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>

namespace tl
{
    namespace file
    {
        namespace
        {
            enum class ErrorType {
                Open,
                Stat,
                MemoryMap,
                Close,
                CloseMemoryMap,
                Read,
                ReadMemoryMap,
                Write,
                Seek,
                SeekMemoryMap
            };

            std::string getErrorString()
            {
                std::string out;
                char buf[string::cBufferSize] = "";
#if defined(_GNU_SOURCE)
                out = strerror_r(errno, buf, string::cBufferSize);
#else  // _GNU_SOURCE
                strerror_r(errno, buf, string::cBufferSize);
                out = buf;
#endif // _GNU_SOURCE
                return out;
            }

            std::string getErrorMessage(
                ErrorType type,
                const std::string& path,
                const std::string& message = std::string())
            {
                std::string out;
                switch (type)
                {
                case ErrorType::Open:
                    out = string::Format("{0}: Cannot open file").arg(path);
                    break;
                case ErrorType::Stat:
                    out = string::Format("{0}: Cannot stat file").arg(path);
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
            Read read = Read::First;
            size_t pos = 0;
            size_t size = 0;
            bool endianConversion = false;
            int f = -1;
            void* mMap = reinterpret_cast<void*>(-1);
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

        size_t FileIO::getObjectCount()
        {
            return objectCount;
        }

        std::shared_ptr<FileIO>
        FileIO::create(const std::filesystem::path& path, const MemoryRead& memory)
        {
            auto out = std::shared_ptr<FileIO>(new FileIO);
            out->_p->path = path;
            out->_p->mode = Mode::Read;
            out->_p->read = Read::Normal;
            out->_p->size = memory.size;
            out->_p->memoryStart = memory.p;
            out->_p->memoryEnd = memory.p + memory.size;
            out->_p->memoryP = memory.p;
            return out;
        }

        bool FileIO::isOpen() const
        {
            return _p->f != -1 || _p->memoryStart;
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
                out |= -1 == p.f;
            }
            out |= p.pos >= p.size;
            return out;
        }

        void FileIO::read(void* in, size_t size, size_t wordSize)
        {
            TLRENDER_P();

            if (!p.memoryStart && -1 == p.f)
            {
                throw std::runtime_error(
                    getErrorMessage(ErrorType::Read, p.path));
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
                        throw std::runtime_error(getErrorMessage(
                            ErrorType::ReadMemoryMap, p.path));
                    }
                    if (p.endianConversion && wordSize > 1)
                    {
                        memory::swapEndian(p.memoryP, in, size, wordSize);
                    }
                    else
                    {
                        memcpy(in, p.memoryP, size * wordSize);
                    }
                    p.memoryP = memoryP;
                }
                else
                {
                    const ssize_t r = ::read(p.f, in, size * wordSize);
                    if (-1 == r)
                    {
                        throw std::runtime_error(getErrorMessage(
                            ErrorType::Read, p.path, getErrorString()));
                    }
                    else if (r != size * wordSize)
                    {
                        throw std::runtime_error(
                            getErrorMessage(ErrorType::Read, p.path));
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
                if (-1 == r)
                {
                    throw std::runtime_error(getErrorMessage(
                        ErrorType::Read, p.path, getErrorString()));
                }
                else if (r != size * wordSize)
                {
                    throw std::runtime_error(
                        getErrorMessage(ErrorType::Read, p.path));
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
                        p.memoryStart ? ErrorType::ReadMemoryMap : ErrorType::Read,
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
                    memcpy(in, p.memoryStart + pos, byteCount);
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
                            getErrorMessage(ErrorType::Read, p.path.u8string(), getErrorString()));
                    }
                    else if (0 == r)
                    {
                        throw std::runtime_error(
                            getErrorMessage(ErrorType::Read, p.path.u8string()));
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
                    getErrorMessage(ErrorType::Read, p.path.u8string()));
            }
        }

        void FileIO::write(const void* in, size_t size, size_t wordSize)
        {
            TLRENDER_P();

            if (-1 == p.f)
            {
                throw std::runtime_error(
                    getErrorMessage(ErrorType::Write, p.path));
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
                throw std::runtime_error(getErrorMessage(
                    ErrorType::Write, p.path, getErrorString()));
            }
            p.pos += size * wordSize;
            p.size = std::max(p.pos, p.size);
        }

        void
        FileIO::_open(const std::filesystem::path& path, Mode mode, Read read,
                      Access access)
        {
            TLRENDER_P();

            _close();

            // Open the file.
            int openFlags = 0;
            int openMode = 0;
            switch (mode)
            {
            case Mode::Read:
                openFlags = O_RDONLY;
                break;
            case Mode::Write:
                openFlags = O_WRONLY | O_CREAT | O_TRUNC;
                openMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
                break;
            case Mode::ReadWrite:
                openFlags = O_RDWR | O_CREAT;
                openMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
                break;
            case Mode::Append:
                openFlags = O_WRONLY | O_CREAT | O_APPEND;
                openMode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
                break;
            default:
                break;
            }
            p.f = ::open(path.u8string().c_str(), openFlags, openMode);
            if (-1 == p.f)
            {
                throw std::runtime_error(getErrorMessage(
                    ErrorType::Open, path, getErrorString()));
            }

            p.path = path;
            p.mode = mode;
            p.read = read;
            p.pos = 0;
            p.size = std::filesystem::file_size(path);

            // Memory mapping.
            if (Read::MemoryMapped == p.read && Mode::Read == p.mode &&
                p.size > 0)
            {
                p.mMap = mmap(0, p.size, PROT_READ, MAP_SHARED, p.f, 0);
                madvise(p.mMap, p.size, MADV_SEQUENTIAL | MADV_SEQUENTIAL);
                if (p.mMap == (void*)-1)
                {
                    throw std::runtime_error(getErrorMessage(
                        ErrorType::MemoryMap, path, getErrorString()));
                }
                p.memoryStart = reinterpret_cast<const uint8_t*>(p.mMap);
                p.memoryEnd = p.memoryStart + p.size;
                p.memoryP = p.memoryStart;
            }
        }

        bool FileIO::_close(std::string* error)
        {
            TLRENDER_P();

            bool out = true;

            p.path = std::string();

            if (p.mMap != (void*)-1)
            {
                int r = munmap(p.mMap, p.size);
                if (-1 == r)
                {
                    out = false;
                    if (error)
                    {
                        *error = getErrorMessage(
                            ErrorType::CloseMemoryMap, p.path,
                            getErrorString());
                    }
                }
                p.mMap = (void*)-1;
            }
            p.memoryStart = nullptr;
            p.memoryEnd = nullptr;

            if (p.f != -1)
            {
                int r = ::close(p.f);
                if (-1 == r)
                {
                    out = false;
                    if (error)
                    {
                        *error = getErrorMessage(
                            ErrorType::Close, p.path, getErrorString());
                    }
                }
                p.f = -1;
            }

            p.mode = Mode::First;
            p.pos = 0;
            p.size = 0;

            return out;
        }

        void FileIO::Private::seek(size_t value, SeekMode seekMode)
        {
            if (Mode::Read == mode && memoryStart)
            {
                switch (seekMode)
                {
                case SeekMode::Set:
                    memoryP = reinterpret_cast<const uint8_t*>(memoryStart) + value;
                    if (memoryP > memoryEnd)
                    {
                        throw std::runtime_error(
                            getErrorMessage(ErrorType::SeekMemoryMap, path.u8string()));
                    }
                    break;
                case SeekMode::Forward:
                    memoryP += value;
                    if (memoryP > memoryEnd)
                    {
                        throw std::runtime_error(
                            getErrorMessage(ErrorType::SeekMemoryMap, path.u8string()));
                    }
                    break;
                case SeekMode::Reverse:
                    memoryP -= value;
                    if (memoryP < memoryStart)
                    {
                        throw std::runtime_error(
                            getErrorMessage(ErrorType::SeekMemoryMap, path.u8string()));
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
                        getErrorMessage(ErrorType::Seek, path.u8string(), getErrorString()));
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
                madvise(const_cast<void*>(p), size, MADV_WILLNEED);
            }
        }

        void truncate(const std::filesystem::path& path, size_t size)
        {
            if (::truncate(path.u8string().c_str(), size) != 0)
            {
                throw std::runtime_error(
                    getErrorMessage(ErrorType::Write, path.u8string(), getErrorString()));
            }
        }

        void truncate(const std::string& path, size_t size)
        {
            if (::truncate(path.c_str(), size) != 0)
            {
                throw std::runtime_error(getErrorMessage(
                    ErrorType::Write, path, getErrorString()));
            }
        }
    } // namespace file
} // namespace tl
