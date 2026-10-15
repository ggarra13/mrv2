// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlIOTest/IOTest.h>

#include <tlIO/SeqDecode.h>
#include <tlIO/System.h>

#include <tlCore/Assert.h>
#include <tlCore/Context.h>
#include <tlCore/Image.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Path.h>

#include <atomic>
#include <cstring>
#include <map>
#include <sstream>
#include <thread>

namespace tl
{
    namespace io_tests
    {
        IOTest::IOTest(const std::shared_ptr<system::Context>& context) :
            ITest("io_tests::IOTest", context)
        {}

        std::shared_ptr<IOTest> IOTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<IOTest>(new IOTest(context));
        }

        void IOTest::run()
        {
            _videoData();
            _ioSystem();
            _decode();
            _seqDecode();
            _missingFrames();
            _seqRange();
            _structural();
        }

        void IOTest::_videoData()
        {
            {
                const io::VideoData v;
                TLRENDER_ASSERT(!v.image);
            }
            {
                const auto time = OTIO_NS::RationalTime(1.0, 24.0);
                const uint16_t layer = 1;
                const auto image = image::Image::create(160, 80, image::PixelType::L_U8);
                const io::VideoData v(time, layer, image);
                TLRENDER_ASSERT(time.strictly_equal(v.time));
                TLRENDER_ASSERT(layer == v.layer);
                TLRENDER_ASSERT(image == v.image);
            }
            {
                const auto time = OTIO_NS::RationalTime(1.0, 24.0);
                const uint16_t layer = 1;
                const auto image = image::Image::create(16, 16, image::PixelType::L_U8);
                const io::VideoData a(time, layer, image);
                io::VideoData b(time, layer, image);
                TLRENDER_ASSERT(a == b);
                b.time = OTIO_NS::RationalTime(2.0, 24.0);
                TLRENDER_ASSERT(a != b);
                TLRENDER_ASSERT(a < b);
            }
        }

        void IOTest::_ioSystem()
        {
            auto readSystem = _context->getSystem<io::ReadSystem>();
            {
                for (const auto& plugin : readSystem->getPlugins())
                {
                    const auto& exts = plugin->getExts();
                    std::stringstream ss;
                    ss << plugin->getPluginName() << ": " <<
                        string::join(std::vector<std::string>(exts.begin(), exts.end()), ", ");
                    _print(ss.str());
                }
            }
            TLRENDER_ASSERT(!readSystem->videoRead(file::Path()));
            TLRENDER_ASSERT(!readSystem->audioRead(file::Path()));
            auto writeSystem = _context->getSystem<io::WriteSystem>();
            TLRENDER_ASSERT(!writeSystem->write(file::Path(), io::Info()));
        }

        void IOTest::_decode()
        {
            auto readSystem = _context->getSystem<io::ReadSystem>();
            auto writeSystem = _context->getSystem<io::WriteSystem>();
            const image::Size size(16, 16);

            // A decoder has to produce what the reader for the same format
            // produces; the reader is what everything still goes through.
            for (const std::string& ext : { ".exr", ".png", ".tif", ".tga" })
            {
                const file::Path path(
                    file::fromFileSystem(_getTempDir() / ("IOTestDecode" + ext)));
                auto writePlugin = writeSystem->getPlugin(path);
                auto readPlugin = readSystem->getPlugin(path);
                if (!writePlugin || !readPlugin)
                {
                    continue;
                }
                auto decode = readPlugin->decode();
                if (!decode)
                {
                    _print(string::Format("{0}: no decoder").arg(ext));
                    continue;
                }

                const image::Info imageInfo = writePlugin->getInfo(
                    image::Info(size, image::PixelType::RGB_U8));
                if (image::PixelType::kNone == imageInfo.pixelType)
                {
                    continue;
                }
                auto image = image::Image::create(imageInfo);
                image->zero();
                io::Info info;
                info.video.push_back(imageInfo);
                const OTIO_NS::RationalTime time(0.0, 24.0);
                writeSystem->write(path, info)->writeVideo(time, image);

                // From the file, and from the same bytes in memory.
                auto fileIO = file::FileIO::create(path.get(), file::Mode::Read);
                std::vector<uint8_t> bytes(fileIO->getSize());
                fileIO->read(bytes.data(), bytes.size());
                const file::MemoryRead memFile(nullptr, bytes.data(), bytes.size());

                const io::Info decodeInfo = decode->getInfo(path.get());
                TLRENDER_ASSERT(!decodeInfo.video.empty());
                TLRENDER_ASSERT(decodeInfo.video[0].size == size);

                // One file says nothing about the sequence it is part of, so
                // the decoder leaves the time range alone.
                TLRENDER_ASSERT(!decodeInfo.videoTime.has_value());

                const auto viaDecode = decode->readVideo(path.get(), nullptr, time);
                const auto viaMem = decode->readVideo(path.get(), &memFile, time);

                // The image that was written is the oracle, and the two
                // decode paths have to agree with it and with each other.
                TLRENDER_ASSERT(viaDecode.image && viaMem.image);
                const size_t byteCount = image->getDataByteCount();
                TLRENDER_ASSERT(viaDecode.image->getDataByteCount() == byteCount);
                TLRENDER_ASSERT(viaMem.image->getDataByteCount() == byteCount);
                TLRENDER_ASSERT(0 == memcmp(
                    viaDecode.image->getData(), image->getData(), byteCount));
                TLRENDER_ASSERT(0 == memcmp(
                    viaMem.image->getData(), image->getData(), byteCount));

                // A decoder holds no state, so it can be used from several
                // threads at once.
                std::atomic<bool> ok(true);
                std::vector<std::thread> threads;
                for (size_t i = 0; i < 8; ++i)
                {
                    threads.push_back(std::thread(
                        [decode, path, &memFile, time, image, byteCount, &ok]
                        {
                            for (size_t j = 0; j < 8; ++j)
                            {
                                const auto v = decode->readVideo(
                                    path.get(), &memFile, time);
                                if (!v.image || 0 != memcmp(
                                    v.image->getData(),
                                    image->getData(),
                                    byteCount))
                                {
                                    ok = false;
                                }
                            }
                        }));
                }
                for (auto& thread : threads)
                {
                    thread.join();
                }
                TLRENDER_ASSERT(ok);

                _print(string::Format("{0}: decoder matches what was written").arg(ext));
            }
        }

        void IOTest::_seqDecode()
        {
            // A sequence held in memory, which is how a bundle presents one:
            // the same frames, indexed off the path number rather than found
            // on disk.
            auto readSystem = _context->getSystem<io::ReadSystem>();
            auto writeSystem = _context->getSystem<io::WriteSystem>();
            const image::Size size(16, 16);
            const size_t frameCount = 5;
            // The sequence itself is never on disk: its frames are memory,
            // so only the one file the bytes come from is written.
            const file::Path path(
                file::fromFileSystem(_getTempDir() / "IOTestSeq.0001.png"));
            const file::Path srcPath(
                file::fromFileSystem(_getTempDir() / "IOTestSeqSrc.png"));
            auto writePlugin = writeSystem->getPlugin(path);
            auto readPlugin = readSystem->getPlugin(path);
            if (!writePlugin || !readPlugin)
            {
                return;
            }
            auto decode = readPlugin->decode();
            TLRENDER_ASSERT(decode);

            const image::Info imageInfo = writePlugin->getInfo(
                image::Info(size, image::PixelType::RGB_U8));
            io::Info writeInfo;
            writeInfo.video.push_back(imageInfo);

            // Each frame is filled differently, so reading the wrong one
            // shows up rather than matching by accident.
            std::vector<std::shared_ptr<image::Image> > images;
            std::vector<std::vector<uint8_t> > bytes(frameCount);
            std::vector<file::MemoryRead> mem;
            for (size_t i = 0; i < frameCount; ++i)
            {
                auto image = image::Image::create(imageInfo);
                memset(image->getData(), static_cast<int>(17 + i * 40),
                       image->getDataByteCount());
                images.push_back(image);
                // A trailing digit would be read as a frame number and every
                // frame would overwrite the same file.
                const std::string name =
                    std::string("IOTestSeqSrc") +
                    static_cast<char>('a' + i) + ".png";
                const file::Path framePath(file::fromFileSystem(_getTempDir() / name));
                writeSystem->write(framePath, writeInfo)->writeVideo(
                    OTIO_NS::RationalTime(0.0, 24.0), image);
                auto fileIO = file::FileIO::create(framePath.get(), file::Mode::Read);
                bytes[i].resize(fileIO->getSize());
                fileIO->read(bytes[i].data(), bytes[i].size());
                mem.push_back(file::MemoryRead(nullptr, bytes[i].data(), bytes[i].size()));
            }
            auto image = images[0];

            io::Options options;
            options["SequenceIO/DefaultSpeed"] = "24";
            auto seq = io::SeqDecode::create(path, mem, decode, options);
            const io::Info seqInfo = seq->getInfo();

            // The sequence knows the time range that no single file does.
            TLRENDER_ASSERT(seqInfo.videoTime.has_value());
            TLRENDER_ASSERT(seqInfo.videoTime->duration().value() == frameCount);
            TLRENDER_ASSERT(!seqInfo.video.empty());

            for (size_t i = 0; i < frameCount; ++i)
            {
                const OTIO_NS::RationalTime time(
                    static_cast<double>(1 + i), 24.0);
                const io::VideoData a = seq->readVideo(time);
                TLRENDER_ASSERT(a.image);
                TLRENDER_ASSERT(0 == memcmp(
                    a.image->getData(),
                    images[i]->getData(),
                    images[i]->getDataByteCount()));
            }

            // No thread of its own, so the caller decides where reads run.
            std::atomic<bool> ok(true);
            std::vector<std::thread> threads;
            for (size_t i = 0; i < 8; ++i)
            {
                threads.push_back(std::thread(
                    [frameCount, seq, &images, &ok]
                    {
                        for (size_t j = 0; j < frameCount; ++j)
                        {
                            const auto v = seq->readVideo(
                                OTIO_NS::RationalTime(
                                    static_cast<double>(1 + j), 24.0));
                            if (!v.image || 0 != memcmp(
                                v.image->getData(),
                                images[j]->getData(),
                                images[j]->getDataByteCount()))
                            {
                                ok = false;
                            }
                        }
                    }));
            }
            for (auto& thread : threads)
            {
                thread.join();
            }
            TLRENDER_ASSERT(ok);
            _print("sequence decode reads the right frame from memory");
        }

        void IOTest::_missingFrames()
        {
            // A sequence on disk with a gap in it, read under each of the
            // policies. Frames are found by reading them, so this covers the
            // path a render in progress actually takes.
            auto readSystem = _context->getSystem<io::ReadSystem>();
            auto writeSystem = _context->getSystem<io::WriteSystem>();
            const image::Size size(16, 16);
            const file::Path path(
                file::fromFileSystem(_getTempDir() / "IOTestGap.0001.png"));
            auto writePlugin = writeSystem->getPlugin(path);
            auto readPlugin = readSystem->getPlugin(path);
            if (!writePlugin || !readPlugin)
            {
                return;
            }
            auto decode = readPlugin->decode();
            TLRENDER_ASSERT(decode);

            const image::Info imageInfo = writePlugin->getInfo(
                image::Info(size, image::PixelType::RGB_U8));
            io::Info writeInfo;
            writeInfo.video.push_back(imageInfo);

            // Frames 1, 2 and 5 exist; 3 and 4 do not. Each is filled
            // differently so that holding the wrong one shows up.
            const std::vector<int64_t> frames = { 1, 2, 5 };
            std::map<int64_t, std::shared_ptr<image::Image> > images;
            {
                // A sequence writer names each file from the time it is given,
                // so writing the gap is a matter of not asking for those two.
                auto write = writeSystem->write(path, writeInfo);
                for (int64_t frame : frames)
                {
                    auto image = image::Image::create(imageInfo);
                    memset(
                        image->getData(),
                        static_cast<int>(17 + frame * 40),
                        image->getDataByteCount());
                    images[frame] = image;
                    write->writeVideo(
                        OTIO_NS::RationalTime(static_cast<double>(frame), 24.0),
                        image);
                }
            }

            file::Path seqPath(path);
            seqPath.setFrames(math::Int64Range(1, 5));

            const auto readFrame = [&](io::MissingFrames policy, int64_t frame)
                {
                    io::Options options;
                    options["SequenceIO/DefaultSpeed"] = "24";
                    options["SequenceIO/MissingFrames"] = to_string(policy);
                    auto seq = io::SeqDecode::create(seqPath, {}, decode, options);
                    return seq->readVideo(
                        OTIO_NS::RationalTime(static_cast<double>(frame), 24.0));
                };

            // A frame that is there reads the same under every policy.
            for (auto policy : io::getMissingFramesEnums())
            {
                const io::VideoData v = readFrame(policy, 2);
                TLRENDER_ASSERT(v.image);
                TLRENDER_ASSERT(0 == memcmp(
                    v.image->getData(),
                    images[2]->getData(),
                    images[2]->getDataByteCount()));
            }

            // Error: the read fails, as it did before there was a policy.
            {
                bool threw = false;
                try
                {
                    readFrame(io::MissingFrames::Error, 3);
                }
                catch (const std::exception&)
                {
                    threw = true;
                }
                TLRENDER_ASSERT(threw);
            }

            // Hold: both frames of the gap give frame 2, the last one before
            // it, and the time is still the time that was asked for.
            for (int64_t frame : { 3, 4 })
            {
                const io::VideoData v = readFrame(io::MissingFrames::Hold, frame);
                TLRENDER_ASSERT(v.image);
                TLRENDER_ASSERT(v.time.value() == frame);
                TLRENDER_ASSERT(0 == memcmp(
                    v.image->getData(),
                    images[2]->getData(),
                    images[2]->getDataByteCount()));

                // And it says so, so that a stand-in can be shown as one
                // rather than passing for the frame that was asked for.
                TLRENDER_ASSERT(v.missing);
                TLRENDER_ASSERT(v.heldFrom.has_value());
                TLRENDER_ASSERT(2 == v.heldFrom.value());
            }

            // A frame that is there says nothing of the sort.
            {
                const io::VideoData v = readFrame(io::MissingFrames::Hold, 2);
                TLRENDER_ASSERT(v.image);
                TLRENDER_ASSERT(!v.missing);
                TLRENDER_ASSERT(!v.heldFrom.has_value());
            }

            // Black: a blank frame of the right size and type.
            {
                const io::VideoData v = readFrame(io::MissingFrames::Black, 3);
                // Blank, so there is no frame it stands in for, but it is
                // still not the frame that was asked for.
                TLRENDER_ASSERT(v.missing);
                TLRENDER_ASSERT(!v.heldFrom.has_value());
                TLRENDER_ASSERT(v.image);
                TLRENDER_ASSERT(v.image->getSize() == size);
                std::vector<uint8_t> zero(v.image->getDataByteCount(), 0);
                TLRENDER_ASSERT(0 == memcmp(
                    v.image->getData(), zero.data(), zero.size()));
            }

            // A frame that is there but half written is missing as far as
            // this is concerned, which is what a frame still being rendered
            // looks like. Truncating 5 and 2 means holding from 5 has to walk
            // past two frames that are not there and one that will not read
            // before it reaches frame 1.
            for (int64_t frame : { 2, 5 })
            {
                const std::string fileName = seqPath.getFrame(frame, true);
                std::vector<uint8_t> bytes;
                {
                    auto fileIO = file::FileIO::create(
                        fileName, file::Mode::Read);
                    bytes.resize(fileIO->getSize() / 2);
                    fileIO->read(bytes.data(), bytes.size());
                }
                auto fileIO = file::FileIO::create(
                    fileName, file::Mode::Write);
                fileIO->write(bytes.data(), bytes.size());
            }
            {
                bool threw = false;
                try
                {
                    readFrame(io::MissingFrames::Error, 5);
                }
                catch (const std::exception&)
                {
                    threw = true;
                }
                TLRENDER_ASSERT(threw);

                const io::VideoData black = readFrame(io::MissingFrames::Black, 5);
                TLRENDER_ASSERT(black.image);

                const io::VideoData hold = readFrame(io::MissingFrames::Hold, 5);
                TLRENDER_ASSERT(hold.image);
                TLRENDER_ASSERT(0 == memcmp(
                    hold.image->getData(),
                    images[1]->getData(),
                    images[1]->getDataByteCount()));
            }

            _print("missing frames follow the policy");
        }

        void IOTest::_seqRange()
        {
            // A sequence opened over a range wider than the frames that
            // exist, which is how a render in progress is watched: the range
            // is the one the shot is meant to be, not the one reached so far.
            auto readSystem = _context->getSystem<io::ReadSystem>();
            auto writeSystem = _context->getSystem<io::WriteSystem>();
            const image::Size size(16, 16);
            const file::Path path(
                file::fromFileSystem(_getTempDir() / "IOTestWide.0001.png"));
            auto writePlugin = writeSystem->getPlugin(path);
            auto readPlugin = readSystem->getPlugin(path);
            if (!writePlugin || !readPlugin)
            {
                return;
            }
            auto decode = readPlugin->decode();
            TLRENDER_ASSERT(decode);

            io::Info writeInfo;
            writeInfo.video.push_back(writePlugin->getInfo(
                image::Info(size, image::PixelType::RGB_U8)));

            // Only frames 50 and 51 have been rendered, so the frame the path
            // names is not there.
            {
                auto write = writeSystem->write(path, writeInfo);
                for (int64_t frame : { 50, 51 })
                {
                    write->writeVideo(
                        OTIO_NS::RationalTime(static_cast<double>(frame), 24.0),
                        image::Image::create(writeInfo.video[0]));
                }
            }

            file::Path seqPath(path);
            seqPath.setFrames(math::Int64Range(1, 60));

            io::Options options;
            options["SequenceIO/DefaultSpeed"] = "24";
            options["SequenceIO/MissingFrames"] = to_string(io::MissingFrames::Black);
            auto seq = io::SeqDecode::create(seqPath, {}, decode, options);

            // Opening works, and the range is the one that was asked for
            // rather than the frames that happen to be there.
            const io::Info seqInfo = seq->getInfo();
            TLRENDER_ASSERT(!seqInfo.video.empty());
            TLRENDER_ASSERT(seqInfo.video[0].size == size);
            TLRENDER_ASSERT(60 == seqInfo.videoTime->duration().value());
            TLRENDER_ASSERT(1 == seqInfo.videoTime->start_time().value());

            // A frame that has been rendered reads; one that has not follows
            // the policy.
            const io::VideoData rendered = seq->readVideo(
                OTIO_NS::RationalTime(50.0, 24.0));
            TLRENDER_ASSERT(rendered.image);
            const io::VideoData pending = seq->readVideo(
                OTIO_NS::RationalTime(10.0, 24.0));
            TLRENDER_ASSERT(pending.image);
            TLRENDER_ASSERT(pending.image->getSize() == size);

            _print("a sequence opens over a range wider than its frames");
        }

        void IOTest::_structural()
        {
            // A structural policy is not this layer's business: the timeline
            // built over a sequence decides which frames it covers, so here the
            // sequence keeps the range it was asked for and a frame number is a
            // frame number. What does change is that a frame which is not there
            // is a failure rather than something to fill in, because structure
            // promised it would not be asked for.
            auto readSystem = _context->getSystem<io::ReadSystem>();
            auto writeSystem = _context->getSystem<io::WriteSystem>();
            const image::Size size(16, 16);
            const file::Path path(
                file::fromFileSystem(_getTempDir() / "IOTestStructural.0001.png"));
            auto writePlugin = writeSystem->getPlugin(path);
            auto readPlugin = readSystem->getPlugin(path);
            if (!writePlugin || !readPlugin)
            {
                return;
            }
            auto decode = readPlugin->decode();
            TLRENDER_ASSERT(decode);

            io::Info writeInfo;
            writeInfo.video.push_back(writePlugin->getInfo(
                image::Info(size, image::PixelType::RGB_U8)));

            // Every fifth frame, which is what a render in progress looks like.
            const std::vector<int64_t> frames = { 1, 6, 11, 16 };
            std::map<int64_t, std::shared_ptr<image::Image> > images;
            {
                auto write = writeSystem->write(path, writeInfo);
                for (int64_t frame : frames)
                {
                    auto image = image::Image::create(writeInfo.video[0]);
                    memset(
                        image->getData(),
                        static_cast<int>(17 + frame * 13),
                        image->getDataByteCount());
                    images[frame] = image;
                    write->writeVideo(
                        OTIO_NS::RationalTime(static_cast<double>(frame), 24.0),
                        image);
                }
            }

            file::Path seqPath(path);
            seqPath.setFrames(math::Int64Range(1, 20));
            for (auto missingFrames :
                     { io::MissingFrames::Skip, io::MissingFrames::Gaps })
            {
                TLRENDER_ASSERT(io::isStructural(missingFrames));
                io::Options options;
                options["SequenceIO/DefaultSpeed"] = "24";
                options["SequenceIO/MissingFrames"] = to_string(missingFrames);
                auto seq = io::SeqDecode::create(seqPath, {}, decode, options);

                // The whole range asked for, not the four frames that are
                // there: shortening it is the timeline's job.
                const io::Info info = seq->getInfo();
                TLRENDER_ASSERT(20 == info.videoTime->duration().value());
                TLRENDER_ASSERT(1 == info.videoTime->start_time().value());

                // A frame that is there reads at its own number.
                for (int64_t frame : frames)
                {
                    const io::VideoData v = seq->readVideo(
                        OTIO_NS::RationalTime(
                            static_cast<double>(frame), 24.0));
                    TLRENDER_ASSERT(v.image);
                    TLRENDER_ASSERT(0 == memcmp(
                        v.image->getData(),
                        images[frame]->getData(),
                        images[frame]->getDataByteCount()));
                }

                // One that is not is an error rather than a held or blank
                // frame, since no clip should have asked for it.
                bool caught = false;
                try
                {
                    seq->readVideo(OTIO_NS::RationalTime(2.0, 24.0));
                }
                catch (const std::exception&)
                {
                    caught = true;
                }
                TLRENDER_ASSERT(caught);
            }

            _print("a structural policy leaves the sequence its own length");
        }
    }
}
