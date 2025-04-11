// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCoreTest/FileIOTest.h>

#include <tlCore/Assert.h>
#include <tlCore/File.h>
#include <tlCore/FileIO.h>
#include <tlCore/Path.h>

#include <limits>
#include <sstream>

using namespace tl::file;

namespace tl
{
    namespace core_tests
    {
        FileIOTest::FileIOTest(
            const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::FileIOTest", context),
            _fileName("file.txt"),
            _text("Hello"),
            _text2("world!")
        {
        }

        std::shared_ptr<FileIOTest>
        FileIOTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<FileIOTest>(new FileIOTest(context));
        }

        void FileIOTest::run()
        {
            _enums();
            _tests();
        }

        void FileIOTest::_enums()
        {
            _enum<Mode>("Mode", getModeEnums);
            _enum<ReadType>("ReadType", getReadTypeEnums);
        }

        void FileIOTest::_tests()
        {
            {
                const std::string fileName =
                    Path(createTempDir(), _fileName).get();
                auto io = FileIO::create(fileName, Mode::Write);
                TLRENDER_ASSERT(io->isOpen());
                TLRENDER_ASSERT(io->getFileName() == fileName);
            }
            {
                auto io = FileIO::createTemp();
                TLRENDER_ASSERT(io->isOpen());
            }
            {
                constexpr int8_t i8 = std::numeric_limits<int8_t>::max();
                constexpr uint8_t u8 = std::numeric_limits<uint8_t>::max();
                constexpr int16_t i16 = std::numeric_limits<int16_t>::max();
                constexpr uint16_t u16 = std::numeric_limits<uint16_t>::max();
                constexpr int32_t i32 = std::numeric_limits<int32_t>::max();
                constexpr uint32_t u32 = std::numeric_limits<uint32_t>::max();
                constexpr float f = std::numeric_limits<float>::max();
                const std::string fileName =
                    Path(createTempDir(), _fileName).get();
                auto io = FileIO::create(fileName, Mode::Write);
                io->write8(i8);
                io->writeU8(u8);
                io->write16(i16);
                io->writeU16(u16);
                io->write32(i32);
                io->writeU32(u32);
                io->writeF32(f);
                io.reset();

                for (auto readType : getReadTypeEnums())
                {
                    io = FileIO::create(fileName, Mode::Read, readType);
                    int8_t _i8 = 0;
                    uint8_t _u8 = 0;
                    int16_t _i16 = 0;
                    uint16_t _u16 = 0;
                    int32_t _i32 = 0;
                    uint32_t _u32 = 0;
                    float _f = 0.F;
                    io->read8(&_i8);
                    io->readU8(&_u8);
                    io->read16(&_i16);
                    io->readU16(&_u16);
                    io->read32(&_i32);
                    io->readU32(&_u32);
                    io->readF32(&_f);
                    TLRENDER_ASSERT(i8 == _i8);
                    TLRENDER_ASSERT(u8 == _u8);
                    TLRENDER_ASSERT(i16 == _i16);
                    TLRENDER_ASSERT(u16 == _u16);
                    TLRENDER_ASSERT(i32 == _i32);
                    TLRENDER_ASSERT(u32 == _u32);
                    TLRENDER_ASSERT(f == _f);
                }
            }
            {
                auto io = FileIO::create("大平原", Mode::Write);
                TLRENDER_ASSERT(io->isOpen());
            }
            {
                auto io = FileIO::create("大平原", Mode::Read);
                TLRENDER_ASSERT(io->isOpen());
            }
            {
                const std::string fileName =
                    Path(createTempDir(), _fileName).get();
                auto io = FileIO::create(fileName, Mode::Write);
                io->write(_text + " ");
                io.reset();

                io = FileIO::create(fileName, Mode::Append);
                io->seek(io->getSize());
                io->write(_text2);
                io.reset();

                for (auto readType : getReadTypeEnums())
                {
                    io = FileIO::create(fileName, Mode::Read, readType);
                    std::string buf = readContents(io);
                    _print(buf);
                    TLRENDER_ASSERT((_text + " " + _text2) == buf);
                    io->setPos(0);
                    TLRENDER_ASSERT(0 == io->getPos());
                }
            }
            {
                const std::string fileName =
                    Path(createTempDir(), _fileName).get();
                writeLines(
                    fileName, {"# This is a comment", _text + " " + _text2});

                for (auto readType : getReadTypeEnums())
                {
                    auto io = FileIO::create(fileName, Mode::Read, readType);
                    char buf[string::cBufferSize];
                    readWord(io, buf);
                    _print(buf);
                    TLRENDER_ASSERT(_text == buf);
                    readWord(io, buf);
                    _print(buf);
                    TLRENDER_ASSERT(_text2 == buf);
                }
            }
            {
                const std::string fileName =
                    Path(createTempDir(), _fileName).get();
                auto io = FileIO::create(fileName, Mode::Write);
                io->write(_text + "\n" + _text2);
                io.reset();

                for (auto readType : getReadTypeEnums())
                {
                    io = FileIO::create(fileName, Mode::Read, readType);
                    char buf[string::cBufferSize];
                    readLine(io, buf);
                    _print(buf);
                    TLRENDER_ASSERT(_text == buf);
                    readLine(io, buf);
                    _print(buf);
                    TLRENDER_ASSERT(_text2 == buf);
                }
            }
            {
                const std::string fileName =
                    Path(createTempDir(), _fileName).get();
                writeLines(fileName, {_text, "# This is a comment", _text2});

                const auto lines = readLines(fileName);
                for (const auto& i : lines)
                {
                    _print(i);
                }
                TLRENDER_ASSERT(_text == lines[0]);
                TLRENDER_ASSERT(_text2 == lines[2]);
            }
            {
                const std::string fileName =
                    Path(createTempDir(), _fileName).get();
                auto io = FileIO::create(fileName, Mode::Write);
                TLRENDER_ASSERT(!io->hasEndianConversion());
                io->setEndianConversion(true);
                TLRENDER_ASSERT(io->hasEndianConversion());
                uint32_t u32 = 0;
                uint8_t* p = reinterpret_cast<uint8_t*>(&u32);
                p[0] = 0;
                p[1] = 1;
                p[2] = 2;
                p[3] = 3;
                io->writeU32(u32);
                io.reset();
                for (auto readType : getReadTypeEnums())
                {
                    for (auto mode : {Mode::Read, Mode::ReadWrite})
                    {
                        io = FileIO::create(fileName, mode, readType);
                        io->setEndianConversion(false);
                        uint32_t _u32 = 0;
                        io->readU32(&_u32);
                        uint8_t* p2 = reinterpret_cast<uint8_t*>(&_u32);
                        TLRENDER_ASSERT(p[0] == p2[3]);
                        TLRENDER_ASSERT(p[1] == p2[2]);
                        TLRENDER_ASSERT(p[2] == p2[1]);
                        TLRENDER_ASSERT(p[3] == p2[0]);
                        io->setEndianConversion(true);
                        io->setPos(0);
                        io->readU32(&_u32);
                        TLRENDER_ASSERT(p[0] == p2[0]);
                        TLRENDER_ASSERT(p[1] == p2[1]);
                        TLRENDER_ASSERT(p[2] == p2[2]);
                        TLRENDER_ASSERT(p[3] == p2[3]);
                        io.reset();
                    }
                }
            }

            for (auto mode :
                 {Mode::Read, Mode::Write, Mode::ReadWrite, Mode::Append})
            {
                try
                {
                    FileIO::create(std::string(), mode);
                    TLRENDER_ASSERT(false);
                }
                catch (const std::exception& e)
                {
                    _print(e.what());
                }
            }

#if !defined(_WINDOWS)
            try
            {
                const std::string fileName =
                    Path(createTempDir(), _fileName).get();
                auto io = FileIO::create(fileName, Mode::Write);
                io.reset();
                io = FileIO::create(fileName, Mode::Read);
                uint8_t buf[16];
                //! \bug FileIO::read() doesn't fail here on Windows?
                io->read(buf, 16, 1);
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception& e)
            {
                _print(e.what());
            }

            try
            {
                const std::string fileName =
                    Path(createTempDir(), _fileName).get();
                auto io = FileIO::create(fileName, Mode::Write);
                io.reset();
                io = FileIO::create(fileName, Mode::ReadWrite);
                uint8_t buf[16];
                //! \bug FileIO::read() doesn't fail here on Windows?
                io->read(buf, 16, 1);
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception& e)
            {
                _print(e.what());
            }
#endif // _WINDOWS

            try
            {
                auto io = FileIO::create(std::string(), file::Mode::Write);
                uint8_t buf[16];
                //! \todo This does not get tested since FileIO::create throws
                //! an exception.
                io->write(buf, 16, 1);
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception& e)
            {
                _print(e.what());
            }

            try
            {
                auto io = FileIO::create(std::string(), file::Mode::Write);
                //! \todo This does not get tested since FileIO::create throws
                //! an exception.
                io->setPos(16);
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception& e)
            {
                _print(e.what());
            }

            try
            {
                auto io = FileIO::create(std::string(), file::Mode::Write);
                //! \todo This does not get tested since FileIO::create throws
                //! an exception.
                io->seek(16);
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception& e)
            {
                _print(e.what());
            }
        }
    } // namespace core_tests
} // namespace tl
