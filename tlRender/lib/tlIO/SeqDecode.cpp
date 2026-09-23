// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlIO/SeqDecode.h>

#include <tlIO/SequenceIO.h>

#include <tlCore/StringFormat.h>
#include <tlCore/Math.h>

#include <algorithm>
#include <filesystem>
#include <sstream>
#include <stdexcept>

namespace tl
{
    namespace io
    {
        void io::SeqDecode::_init(
            const file::Path& path,
            const std::vector<file::MemoryRead>& mem,
            const std::shared_ptr<io::IDecode>& decode,
            const io::Options& options)
        {
            _path = path;
            _mem = mem;
            _decode = decode;
            _options = options;

            // Where the sequence starts and ends. Memory comes from a bundle,
            // which holds exactly the frames it holds; otherwise the path carries
            // the range it was found with.
            const std::string& num = path.getNumber();
            if (!num.empty())
            {
                if (!_mem.empty())
                {
                    std::stringstream ss(num);
                    ss >> _startFrame;
                    _endFrame = _startFrame + _mem.size() - 1;
                }
                else if (path.getFrames().has_value())
                {
                    const math::Int64Range& frames = path.getFrames().value();
                    _startFrame = frames.min();
                    _endFrame = frames.max();
                }
            }

            double defaultSpeed = SeqOptions().defaultSpeed;
            if (const auto i = options.find("SeqIO/DefaultSpeed");
                i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> defaultSpeed;
            }

            _info = _probeInfo();
            const double speed = _decode->getSpeed(_info, defaultSpeed);

            // The range the sequence covers, whether or not every frame in it is
            // there. A policy that leaves frames out is settled by the timeline
            // that is built over this, which is the only thing that can shorten a
            // sequence or move its frames; here a frame number is a frame number.
            _info.videoTime = OTIO_NS::TimeRange::range_from_start_end_time_inclusive(
                OTIO_NS::RationalTime(_startFrame, speed),
                OTIO_NS::RationalTime(_endFrame, speed));
            addVideoTags(_info);
        }

        io::SeqDecode::SeqDecode()
        {}

        io::SeqDecode::~SeqDecode()
        {}

        std::shared_ptr<SeqDecode> io::SeqDecode::create(
            const file::Path& path,
            const std::vector<file::MemoryRead>& mem,
            const std::shared_ptr<io::IDecode>& decode,
            const io::Options& options)
        {
            auto out = std::shared_ptr<SeqDecode>(new io::SeqDecode);
            out->_init(path, mem, decode, options);
            return out;
        }

        const file::Path& io::SeqDecode::getPath() const
        {
            return _path;
        }

        const io::Info& io::SeqDecode::getInfo() const
        {
            return _info;
        }

        io::Info io::SeqDecode::_probeInfo() const
        {
            if (_path.getNumber().empty())
            {
                // Not a sequence, so there is one file to probe -- and it still
                // comes from the bundle when there is one.
                return _decode->getInfo(_path.getFileName(true), _memFile(0));
            }

            // The first frame that is actually there. Usually that is the frame
            // the path names, but it need not be: a bundle can be missing frames,
            // and a sequence can be opened over a range that begins before the
            // frames rendered so far.
            std::exception_ptr error;
            for (int64_t frame = _startFrame; frame <= _endFrame; ++frame)
            {
                const std::string fileName = _path.getFrame(frame, true);
                const file::MemoryRead* mem = nullptr;
                if (!_mem.empty())
                {
                    mem = _memFile(frame);
                    if (!mem)
                    {
                        continue;
                    }
                }
                else if (!std::filesystem::exists(
                             std::filesystem::u8path(fileName)))
                {
                    continue;
                }
                try
                {
                    return _decode->getInfo(fileName, mem);
                }
                catch (const std::exception&)
                {
                    // There but half written, most likely. Keep looking, and
                    // keep the first failure in case none of them read.
                    if (!error)
                    {
                        error = std::current_exception();
                    }
                }
            }
            if (error)
            {
                std::rethrow_exception(error);
            }

            // Nothing at all, so fail against the frame the path names rather
            // than inventing a message about one the caller never asked for.
            return _decode->getInfo(_path.getFileName(true), nullptr);
        }

        const file::MemoryRead* io::SeqDecode::_memFile(int64_t frame) const
        {
            const int64_t i = !_path.getNumber().empty() ?
                              (frame - _startFrame) :
                              0;
            const file::MemoryRead* out =
                i >= 0 && i < static_cast<int64_t>(_mem.size()) ?
                &_mem[i] :
                nullptr;
            return out && out->p ? out : nullptr;
        }

        int64_t io::SeqDecode::_holdFrame(int64_t frame) const
        {
            for (int64_t i = frame - 1; i >= _startFrame; --i)
            {
                const bool exists = !_mem.empty() ?
                                    _memFile(i) != nullptr :
                                    // Only reached once a frame has already failed to read, so
                                    // this costs nothing while a sequence is complete, and while
                                    // it is not it costs one test per frame of the gap. Asking
                                    // the file system each time rather than caching what was
                                    // there at open is deliberate: a render in progress gains
                                    // frames while it is being watched.
                                    std::filesystem::exists(
                                        std::filesystem::u8path(_path.getFrame(i, true)));
                if (exists)
                {
                    return i;
                }
            }
            return frame;
        }

        VideoData io::SeqDecode::_missingVideo(
            const OTIO_NS::RationalTime& time,
            io::MissingFrames missingFrames) const
        {
            std::shared_ptr<image::Image> image;
            if (io::MissingFrames::Black == missingFrames && !_info.video.empty())
            {
                image = image::Image::create(_info.video[0]);
                image->zero();
            }
            VideoData out(time, 0, image);
            out.missing = true;
            return out;
        }

        VideoData io::SeqDecode::readVideo(
            const OTIO_NS::RationalTime& time,
            const io::Options& options) const
        {
            const io::Options merged = io::merge(options, _options);
            // Taken per read rather than held from when this was created, so that
            // changing the policy takes effect on what is already open. The
            // options this was created with are the base of the merge, so a
            // caller that says nothing still gets what the media asked for.
            io::MissingFrames missingFrames = getMissingFrames(merged);
            if (io::isStructural(missingFrames))
            {
                // Structure decided those, and it laid clips only over frames that
                // are there, so one that will not read here is a real failure
                // rather than something to fill in.
                missingFrames = io::MissingFrames::Error;
            }
            const bool seq = !_path.getNumber().empty();
            const int64_t frame = static_cast<int64_t>(time.value());

            if (!_mem.empty())
            {
                // Memory is indexed by frame number, and a frame the bundle does
                // not hold leaves an empty entry. Reading that frame from its path
                // would be reading a different file than the bundle describes, so
                // it is missing whatever is on disk.
                int64_t readFrame = frame;
                const file::MemoryRead* mem = _memFile(frame);
                if (!mem && seq && io::MissingFrames::Hold == missingFrames)
                {
                    readFrame = _holdFrame(frame);
                    mem = _memFile(readFrame);
                }
                if (!mem)
                {
                    if (io::MissingFrames::Error == missingFrames)
                    {
                        // The same as a frame that will not read from disk, so
                        // that the policy means one thing everywhere.
                        throw std::runtime_error(
                            string::Format("Frame not in the bundle: \"{0}\"").
                            arg(_path.getFrame(frame, true)));
                    }
                    return _missingVideo(time, missingFrames);
                }

                // Faulting a mapped range in a page at a time costs several
                // times what asking for the whole range costs, and the faults
                // contend when several frames are read at once.
                file::prefetch(mem->p, mem->size);

                VideoData out = _decode->readVideo(
                    seq ? _path.getFrame(readFrame, true) : _path.getFileName(true),
                    mem,
                    time,
                    merged);
                if (readFrame != frame)
                {
                    out.missing = true;
                    out.heldFrom = readFrame;
                }
                return out;
            }

            if (!seq)
            {
                return _decode->readVideo(
                    _path.getFileName(true), nullptr, time, merged);
            }

            try
            {
                return _decode->readVideo(
                    _path.getFrame(frame, true), nullptr, time, merged);
            }
            catch (const std::exception&)
            {
                // The frame did not read. Which frames a sequence has is only
                // discovered by trying, so that a complete sequence pays nothing
                // for this. A frame that is there but half written counts as
                // missing too, which is the case while it is being rendered.
                switch (missingFrames)
                {
                case io::MissingFrames::Hold:
                {
                    // Walk back a frame at a time rather than holding whichever
                    // one is nearest: with several frames being written at once,
                    // more than one of them can fail to read.
                    for (int64_t i = frame; ; )
                    {
                        const int64_t prev = _holdFrame(i);
                        if (prev == i)
                        {
                            break;
                        }
                        try
                        {
                            VideoData out = _decode->readVideo(
                                _path.getFrame(prev, true), nullptr, time, merged);
                            // Which frame is being looked at rather than the one
                            // asked for, so it can be said rather than guessed.
                            out.missing = true;
                            out.heldFrom = prev;
                            return out;
                        }
                        catch (const std::exception&)
                        {}
                        i = prev;
                    }
                    return _missingVideo(time, missingFrames);
                }
                case io::MissingFrames::Black:
                    return _missingVideo(time, missingFrames);
                default:
                    throw;
                }
            }
        }
    }
}
