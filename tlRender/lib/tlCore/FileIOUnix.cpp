// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/FileIO.h>

#include <tlCore/File.h>
#include <tlCore/Memory.h>
#include <tlCore/StringFormat.h>

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

#define _STAT struct stat
#define _STAT_FNC stat

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
                ErrorType type, const std::string& fileName,
                const std::string& message = std::string())
            {
                std::string out;
                switch (type)
                {
                case ErrorType::Open:
                    out = string::Format("{0}: Cannot open file").arg(fileName);
                    break;
                case ErrorType::Stat:
                    out = string::Format("{0}: Cannot stat file").arg(fileName);
                    break;
                case ErrorType::MemoryMap:
                    out =
                        string::Format("{0}: Cannot memory map").arg(fileName);
                    break;
                case ErrorType::Close:
                    out = string::Format("{0}: Cannot close").arg(fileName);
                    break;
                case ErrorType::CloseMemoryMap:
                    out = string::Format("{0}: Cannot unmap").arg(fileName);
                    break;
                case ErrorType::Read:
                    out = string::Format("{0}: Cannot read").arg(fileName);
                    break;
                case ErrorType::ReadMemoryMap:
                    out = string::Format("{0}: Cannot read memory map")
                              .arg(fileName);
                    break;
                case ErrorType::Write:
                    out = string::Format("{0}: Cannot write").arg(fileName);
                    break;
                case ErrorType::Seek:
                    out = string::Format("{0}: Cannot seek").arg(fileName);
                    break;
                case ErrorType::SeekMemoryMap:
                    out = string::Format("{0}: Cannot seek memory map")
                              .arg(fileName);
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
            void setPos(size_t, bool seek);

            std::string fileName;
            Mode mode = Mode::First;
            ReadType readType = ReadType::First;
            size_t pos = 0;
            size_t size = 0;
            bool endianConversion = false;
            int f = -1;
            void* mMap = reinterpret_cast<void*>(-1);
            const uint8_t* memoryStart = nullptr;
            const uint8_t* memoryEnd = nullptr;
            const uint8_t* memoryP = nullptr;
        };

        FileIO::FileIO() :
            _p(new Private)
        {
        }

        FileIO::~FileIO()
        {
            _close();
        }

        std::shared_ptr<FileIO>
        FileIO::create(const std::string& fileName, const MemoryRead& memory)
        {
            auto out = std::shared_ptr<FileIO>(new FileIO);
            out->_p->fileName = fileName;
            out->_p->mode = Mode::Read;
            out->_p->readType = ReadType::Normal;
            out->_p->size = memory.size;
            out->_p->memoryStart = memory.p;
            out->_p->memoryEnd = memory.p + memory.size;
            out->_p->memoryP = memory.p;
            return out;
        }

        std::shared_ptr<FileIO> FileIO::createTemp()
        {
            auto out = std::shared_ptr<FileIO>(new FileIO);

            // Open the file.
            const std::string fileName = getTemp() + "/XXXXXX";
            const size_t size = fileName.size();
            std::vector<char> buf(size + 1);
            memcpy(buf.data(), fileName.c_str(), size);
            buf[size] = 0;
            out->_p->f = mkstemp(buf.data());
            if (-1 == out->_p->f)
            {
                throw std::runtime_error(getErrorMessage(
                    ErrorType::Open, fileName, getErrorString()));
            }

            // Stat the file.
            _STAT info;
            memset(&info, 0, sizeof(_STAT));
            if (_STAT_FNC(buf.data(), &info) != 0)
            {
                throw std::runtime_error(getErrorMessage(
                    ErrorType::Stat, fileName, getErrorString()));
            }
            out->_p->fileName = std::string(buf.data());
            out->_p->mode = Mode::ReadWrite;
            out->_p->readType = ReadType::Normal;
            out->_p->pos = 0;
            out->_p->size = info.st_size;

            return out;
        }

        bool FileIO::isOpen() const
        {
            return _p->f != -1 || _p->memoryStart;
        }

        const std::string& FileIO::getFileName() const
        {
            return _p->fileName;
        }

        size_t FileIO::getSize() const
        {
            return _p->size;
        }

        size_t FileIO::getPos() const
        {
            return _p->pos;
        }

        void FileIO::setPos(size_t in)
        {
            _p->setPos(in, false);
        }

        void FileIO::seek(size_t in)
        {
            _p->setPos(in, true);
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
                    getErrorMessage(ErrorType::Read, p.fileName));
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
                            ErrorType::ReadMemoryMap, p.fileName));
                    }
                    if (p.endianConversion && wordSize > 1)
                    {
                        memory::endian(p.memoryP, in, size, wordSize);
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
                            ErrorType::Read, p.fileName, getErrorString()));
                    }
                    else if (r != size * wordSize)
                    {
                        throw std::runtime_error(
                            getErrorMessage(ErrorType::Read, p.fileName));
                    }
                    if (p.endianConversion && wordSize > 1)
                    {
                        memory::endian(in, size, wordSize);
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
                        ErrorType::Read, p.fileName, getErrorString()));
                }
                else if (r != size * wordSize)
                {
                    throw std::runtime_error(
                        getErrorMessage(ErrorType::Read, p.fileName));
                }
                if (p.endianConversion && wordSize > 1)
                {
                    memory::endian(in, size, wordSize);
                }
                break;
            }
            default:
                break;
            }
            p.pos += size * wordSize;
        }

        void FileIO::write(const void* in, size_t size, size_t wordSize)
        {
            TLRENDER_P();

            if (-1 == p.f)
            {
                throw std::runtime_error(
                    getErrorMessage(ErrorType::Write, p.fileName));
            }

            const uint8_t* inP = reinterpret_cast<const uint8_t*>(in);
            std::vector<uint8_t> tmp;
            if (p.endianConversion && wordSize > 1)
            {
                tmp.resize(size * wordSize);
                memory::endian(in, tmp.data(), size, wordSize);
                inP = tmp.data();
            }
            if (::write(p.f, inP, size * wordSize) == -1)
            {
                throw std::runtime_error(getErrorMessage(
                    ErrorType::Write, p.fileName, getErrorString()));
            }
            p.pos += size * wordSize;
            p.size = std::max(p.pos, p.size);
        }

        void
        FileIO::_open(const std::string& fileName, Mode mode, ReadType readType)
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
            p.f = ::open(fileName.c_str(), openFlags, openMode);
            if (-1 == p.f)
            {
                throw std::runtime_error(getErrorMessage(
                    ErrorType::Open, fileName, getErrorString()));
            }

            // Stat the file.
            _STAT info;
            memset(&info, 0, sizeof(_STAT));
            if (_STAT_FNC(fileName.c_str(), &info) != 0)
            {
                throw std::runtime_error(getErrorMessage(
                    ErrorType::Stat, fileName, getErrorString()));
            }
            p.fileName = fileName;
            p.mode = mode;
            p.readType = readType;
            p.pos = 0;
            p.size = info.st_size;

            // Memory mapping.
            if (ReadType::MemoryMapped == p.readType && Mode::Read == p.mode &&
                p.size > 0)
            {
                p.mMap = mmap(0, p.size, PROT_READ, MAP_SHARED, p.f, 0);
                madvise(p.mMap, p.size, MADV_SEQUENTIAL | MADV_SEQUENTIAL);
                if (p.mMap == (void*)-1)
                {
                    throw std::runtime_error(getErrorMessage(
                        ErrorType::MemoryMap, fileName, getErrorString()));
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

            p.fileName = std::string();

            if (p.mMap != (void*)-1)
            {
                int r = munmap(p.mMap, p.size);
                if (-1 == r)
                {
                    out = false;
                    if (error)
                    {
                        *error = getErrorMessage(
                            ErrorType::CloseMemoryMap, p.fileName,
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
                            ErrorType::Close, p.fileName, getErrorString());
                    }
                }
                p.f = -1;
            }

            p.mode = Mode::First;
            p.pos = 0;
            p.size = 0;

            return out;
        }

        void FileIO::Private::setPos(size_t in, bool seek)
        {
            switch (mode)
            {
            case Mode::Read:
            {
                if (memoryStart)
                {
                    if (!seek)
                    {
                        memoryP =
                            reinterpret_cast<const uint8_t*>(memoryStart) + in;
                    }
                    else
                    {
                        memoryP += in;
                    }
                    if (memoryP > memoryEnd)
                    {
                        throw std::runtime_error(getErrorMessage(
                            ErrorType::SeekMemoryMap, fileName));
                    }
                }
                else
                {
                    if (::lseek(f, in, !seek ? SEEK_SET : SEEK_CUR) ==
                        (off_t)-1)
                    {
                        throw std::runtime_error(getErrorMessage(
                            ErrorType::Seek, fileName, getErrorString()));
                    }
                }
                break;
            }
            case Mode::Write:
            case Mode::ReadWrite:
            case Mode::Append:
            {
                if (::lseek(f, in, !seek ? SEEK_SET : SEEK_CUR) == (off_t)-1)
                {
                    throw std::runtime_error(getErrorMessage(
                        ErrorType::Seek, fileName, getErrorString()));
                }
                break;
            }
            default:
                break;
            }
            if (!seek)
            {
                pos = in;
            }
            else
            {
                pos += in;
            }
        }

        void truncate(const std::string& fileName, size_t size)
        {
            if (::truncate(fileName.c_str(), size) != 0)
            {
                throw std::runtime_error(getErrorMessage(
                    ErrorType::Write, fileName, getErrorString()));
            }
        }
    } // namespace file
} // namespace tl
