// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCore/FileIO.h>

#include <tlCore/Assert.h>
#include <tlCore/Error.h>

#include <algorithm>
#include <array>
#include <iostream>

namespace tl
{
    namespace file
    {
        TLRENDER_ENUM_IMPL(Mode, "Read", "Write", "ReadWrite", "Append");
        TLRENDER_ENUM_SERIALIZE_IMPL(Mode);

        TLRENDER_ENUM_IMPL(ReadType, "Normal", "MemoryMapped");
        TLRENDER_ENUM_SERIALIZE_IMPL(ReadType);

        std::shared_ptr<FileIO> FileIO::create(
            const std::string& fileName, Mode mode, ReadType readType)
        {
            auto out = std::shared_ptr<FileIO>(new FileIO);
            out->_open(fileName, mode, readType);
            return out;
        }

        void FileIO::read8(int8_t* value, size_t size)
        {
            return read(value, size, 1);
        }

        void FileIO::readU8(uint8_t* value, size_t size)
        {
            return read(value, size, 1);
        }

        void FileIO::read16(int16_t* value, size_t size)
        {
            return read(value, size, 2);
        }

        void FileIO::readU16(uint16_t* value, size_t size)
        {
            return read(value, size, 2);
        }

        void FileIO::read32(int32_t* value, size_t size)
        {
            return read(value, size, 4);
        }

        void FileIO::readU32(uint32_t* value, size_t size)
        {
            return read(value, size, 4);
        }

        void FileIO::readF32(float* value, size_t size)
        {
            return read(value, size, 4);
        }

        void FileIO::write8(const int8_t* value, size_t size)
        {
            write(value, size, 1);
        }

        void FileIO::writeU8(const uint8_t* value, size_t size)
        {
            write(value, size, 1);
        }

        void FileIO::write16(const int16_t* value, size_t size)
        {
            write(value, size, 2);
        }

        void FileIO::writeU16(const uint16_t* value, size_t size)
        {
            write(value, size, 2);
        }

        void FileIO::write32(const int32_t* value, size_t size)
        {
            return write(value, size, 4);
        }

        void FileIO::writeU32(const uint32_t* value, size_t size)
        {
            return write(value, size, 4);
        }

        void FileIO::writeF32(const float* value, size_t size)
        {
            write(value, size, 4);
        }

        void FileIO::write8(int8_t value)
        {
            write8(&value, 1);
        }

        void FileIO::writeU8(uint8_t value)
        {
            writeU8(&value, 1);
        }

        void FileIO::write16(int16_t value)
        {
            write16(&value, 1);
        }

        void FileIO::writeU16(uint16_t value)
        {
            writeU16(&value, 1);
        }

        void FileIO::write32(int32_t value)
        {
            write32(&value, 1);
        }

        void FileIO::writeU32(uint32_t value)
        {
            writeU32(&value, 1);
        }

        void FileIO::writeF32(float value)
        {
            writeF32(&value, 1);
        }

        void FileIO::write(const std::string& value)
        {
            write8(
                reinterpret_cast<const int8_t*>(value.c_str()), value.size());
        }

        std::string readContents(const std::shared_ptr<FileIO>& io)
        {
            std::string out;
            if (const uint8_t* p = io->getMemoryP())
            {
                const uint8_t* end = io->getMemoryEnd();
                out = std::string(reinterpret_cast<const char*>(p), end - p);
            }
            else
            {
                const size_t fileSize = io->getSize();
                out.resize(fileSize);
                io->read(reinterpret_cast<void*>(&out[0]), fileSize);
            }
            return out;
        }

        void
        readWord(const std::shared_ptr<FileIO>& io, char* out, size_t maxLen)
        {
            TLRENDER_ASSERT(maxLen);
            out[0] = 0;
            enum class Parse { End, Word, Comment };
            Parse parse = Parse::Word;
            size_t i = 0;
            while (parse != Parse::End && !io->isEOF())
            {
                // Get the next character.
                uint8_t c = 0;
                io->read(&c, 1);

                switch (c)
                {
                case '#':
                    // Start of a comment.
                    parse = Parse::Comment;
                    break;
                case '\0':
                case '\n':
                case '\r':
                    // End of a comment or word.
                    parse = Parse::Word;
                case ' ':
                case '\t':
                    if (out[0])
                    {
                        parse = Parse::End;
                    }
                    break;
                default:
                    // Add the character to the word.
                    if (Parse::Word == parse && i < (maxLen - 1))
                    {
                        out[i++] = c;
                    }
                    break;
                }
            }
            out[i] = 0;
        }

        void
        readLine(const std::shared_ptr<FileIO>& io, char* out, size_t maxLen)
        {
            TLRENDER_ASSERT(maxLen);
            size_t i = 0;
            if (!io->isEOF())
            {
                char c = 0;
                do
                {
                    io->read(&c, 1);
                    if (c != '\n' && c != '\r')
                    {
                        out[i++] = c;
                    }
                } while (c != '\n' && c != '\r' && !io->isEOF() &&
                         i < (maxLen - 1));
            }
            out[i] = 0;
        }

        std::vector<std::string> readLines(const std::string& fileName)
        {
            std::vector<std::string> out;
            auto io = FileIO::create(fileName, Mode::Read);
            while (!io->isEOF())
            {
                char buf[string::cBufferSize] = "";
                readLine(io, buf, string::cBufferSize);
                out.push_back(buf);
            }
            return out;
        }

        void writeLines(
            const std::string& fileName, const std::vector<std::string>& lines)
        {
            auto io = FileIO::create(fileName, Mode::Write);
            for (const auto& line : lines)
            {
                io->write(line);
                io->write8('\n');
            }
        }
    } // namespace file
} // namespace tl
