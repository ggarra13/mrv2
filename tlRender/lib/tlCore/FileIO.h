// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/String.h>
#include <tlCore/Util.h>

#include <nlohmann/json.hpp>

#include <memory>

namespace tl
{
    namespace file
    {
        //! File I/O modes.
        enum class Mode {
            Read,
            Write,
            ReadWrite,
            Append,

            Count,
            First = Read
        };
        TLRENDER_ENUM(Mode);
        TLRENDER_ENUM_SERIALIZE(Mode);

        //! File reading type.
        enum class ReadType {
            Normal,
            MemoryMapped,

            Count,
            First = Normal
        };
        TLRENDER_ENUM(ReadType);
        TLRENDER_ENUM_SERIALIZE(ReadType);

        //! Read files from memory.
        struct MemoryRead
        {
            MemoryRead();
            MemoryRead(const uint8_t*, size_t size);

            const uint8_t* p = nullptr;
            size_t size = 0;

            bool operator==(const MemoryRead&) const;
            bool operator!=(const MemoryRead&) const;
        };

        //! File I/O.
        class FileIO
        {
            TLRENDER_NON_COPYABLE(FileIO);

        protected:
            FileIO();

        public:
            ~FileIO();

            //! Create a new file I/O object.
            static std::shared_ptr<FileIO> create(
                const std::string& fileName, Mode,
                ReadType = ReadType::MemoryMapped);

            //! Create a read-only file I/O object from memory.
            static std::shared_ptr<FileIO>
            create(const std::string& fileName, const MemoryRead&);

            //! Create a read-write temporary file I/O object.
            static std::shared_ptr<FileIO> createTemp();

            //! Get whether the file is open.
            bool isOpen() const;

            //! \name Information
            ///@{

            //! Get the file name.
            const std::string& getFileName() const;

            //! Get the file size.
            size_t getSize() const;

            ///@}

            //! \name Position
            ///@{

            //! Get the current file position.
            size_t getPos() const;

            //! Set the current file position.
            void setPos(size_t);

            //! Advance the current file position.
            void seek(size_t);

            //! Get whether the file position is at the end of the file.
            bool isEOF() const;

            ///@}

            //! \name Read
            ///@{

            void read(void*, size_t, size_t wordSize = 1);

            void read8(int8_t*, size_t = 1);
            void readU8(uint8_t*, size_t = 1);
            void read16(int16_t*, size_t = 1);
            void readU16(uint16_t*, size_t = 1);
            void read32(int32_t*, size_t = 1);
            void readU32(uint32_t*, size_t = 1);
            void readF32(float*, size_t = 1);

            ///@}

            //! \name Write
            ///@{

            void write(const void*, size_t, size_t wordSize = 1);

            void write8(const int8_t*, size_t);
            void writeU8(const uint8_t*, size_t);
            void write16(const int16_t*, size_t);
            void writeU16(const uint16_t*, size_t);
            void write32(const int32_t*, size_t);
            void writeU32(const uint32_t*, size_t);
            void writeF32(const float*, size_t);

            void write8(int8_t);
            void writeU8(uint8_t);
            void write16(int16_t);
            void writeU16(uint16_t);
            void write32(int32_t);
            void writeU32(uint32_t);
            void writeF32(float);

            void write(const std::string&);

            ///@}

            //! \name Memory Mapping
            ///@{

            //! Get a pointer to the start of the memory-map.
            const uint8_t* getMemoryStart() const;

            //! Get a pointer to the end of the memory-map.
            const uint8_t* getMemoryEnd() const;

            //! Get the current memory-map position.
            const uint8_t* getMemoryP() const;

            ///@}

            //! \name Endian
            ///@{

            //! Get whether automatic endian conversion is performed.
            bool hasEndianConversion() const;

            //! Set whether automatic endian conversion is performed.
            void setEndianConversion(bool);

            ///@}

        private:
            void _open(const std::string& fileName, Mode, ReadType);
            bool _close(std::string* error = nullptr);

            TLRENDER_PRIVATE();
        };

        //! Read the contents from a file.
        std::string readContents(const std::shared_ptr<FileIO>&);

        //! Read a word from a file.
        void readWord(
            const std::shared_ptr<FileIO>&, char*,
            size_t maxLen = string::cBufferSize);

        //! Read a line from a file.
        //! \todo Should we handle comments like readWord()?
        void readLine(
            const std::shared_ptr<FileIO>&, char*,
            size_t maxLen = string::cBufferSize);

        //! Read all the lines from a file.
        std::vector<std::string> readLines(const std::string& fileName);

        //! Write lines to a file.
        void writeLines(
            const std::string& fileName, const std::vector<std::string>&);

        //! Truncate a file.
        void truncate(const std::string& fileName, size_t);
    } // namespace file
} // namespace tl

#include <tlCore/FileIOInline.h>
