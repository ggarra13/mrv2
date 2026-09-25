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
        enum class Read {
            Normal,
            MemoryMapped,

            Count,
            First = Normal
        };
        TLRENDER_ENUM(Read);
        TLRENDER_ENUM_SERIALIZE(Read);

        //! Expected access pattern, used as a read ahead hint.
        //!
        //! Sequential asks the operating system to read ahead and to drop what has
        //! been passed. That is wrong for a file read as scattered ranges, where the
        //! read ahead is thrown away and costs more than it saves; a bundle of media
        //! is read that way.
        enum class Access
        {
            Sequential,
            Random,

            Count,
            First = Sequential
        };
        TLRENDER_ENUM(Access);
        TLRENDER_ENUM_SERIALIZE(Access);

        //! Read files from memory.
        struct MemoryRead
        {
            MemoryRead() = default;
            MemoryRead(const std::shared_ptr<void>&, const uint8_t*,
                       size_t size);

            std::shared_ptr<void> f;
            const uint8_t* p = nullptr;
            size_t size = 0;

            bool operator==(const MemoryRead&) const;
            bool operator!=(const MemoryRead&) const;
        };

        //! Seek modes.
        enum class SeekMode
        {
            Set,
            Forward,
            Reverse,

            Count,
            First = Set
        };
        TLRENDER_ENUM(SeekMode);
        TLRENDER_ENUM_SERIALIZE(Access);

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
                const std::filesystem::path&,
                Mode,
                Read r = Read::MemoryMapped,
                Access = Access::Sequential);

            //! Create a new file I/O object.
            static std::shared_ptr<FileIO> create(
                const std::string&,
                Mode,
                Read = Read::MemoryMapped,
                Access = Access::Sequential);

            //! Create a read-only file I/O object from memory.
            static std::shared_ptr<FileIO> create(
                const std::filesystem::path&,
                const MemoryRead&);

            //! Create a read-only file I/O object from memory.
            static std::shared_ptr<FileIO> create(
                const std::string&,
                const MemoryRead&);

            //! Get whether the file is open.
            bool isOpen() const;

            //! \name Information
            ///@{

            //! Get the file path.
            const std::filesystem::path& getPath() const;

            //! Get the file size.
            size_t getSize() const;

            ///@}

            //! \name Position
            ///@{

            //! Get the current file position.
            size_t getPos() const;

            //! Advance the current file position.
            void seek(size_t, SeekMode);

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

            //! Read from an absolute position, leaving the current position
            //! unchanged.
            //!
            //! This touches none of the object's state, so several threads may
            //! read from one file at once; seek() followed by read() cannot be
            //! used that way. It may not run concurrently with the position
            //! changing calls.
            void readAt(void*, size_t pos, size_t size, size_t wordSize = 1) const;

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

            //! Get the number of objects currenty instantiated.
            static size_t getObjectCount();

        private:
            void _open(const std::filesystem::path&, Mode, Read, Access);
            bool _close(std::string* error = nullptr);

            TLRENDER_PRIVATE();
        };

        //! Ask the operating system to populate a range of a memory mapped
        //! file.
        //!
        //! Reading a mapped range without this faults it in a page at a time,
        //! and the faults are synchronous and contend with each other when
        //! several threads read from one mapping.
        void prefetch(const void*, size_t);

        //! Read the contents from a file.
        std::string read(const std::shared_ptr<FileIO>&);

        //! Read a word from a file.
        void readWord(
            const std::shared_ptr<FileIO>&, char*,
            size_t maxLen = string::cBufferSize);

        //! Read a line from a file.
        std::string readLine(
            const std::shared_ptr<FileIO>&);

        //! Read all the lines from a file.
        std::vector<std::string> readLines(const std::string& fileName);

        //! Read all the lines from a file.
        std::vector<std::string> readLines(const std::string&);

        //! Write lines to a file.
        void writeLines(
            const std::string& fileName, const std::vector<std::string>&);

        //! Truncate a file.
        void truncate(const std::filesystem::path&, size_t);

        //! Truncate a file.
        void truncate(const std::string& fileName, size_t);

    } // namespace file
} // namespace tl

#include <tlCore/FileIOInline.h>
