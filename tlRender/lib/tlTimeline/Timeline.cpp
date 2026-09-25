// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/TimelinePrivate.h>

#include <tlTimeline/MemoryReference.h>
#include <tlTimeline/Util.h>
#include <tlTimeline/ZipPrivate.h>

#include <tlIO/SequenceIO.h>
#include <tlIO/System.h>

#include <tlCore/Assert.h>
#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>
#include <tlCore/URL.h>

#include <opentimelineio/externalReference.h>
#include <opentimelineio/gap.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/transition.h>

#include <algorithm>

namespace tl
{
    namespace timeline
    {

        namespace
        {
            std::string getKey(const file::Path& path)
            {
                std::vector<std::string> out;
                out.push_back(path.get());
                out.push_back(path.getNumber());
                return string::join(out, ';');
            }

            file::Path getAssociatedAudio(
                const std::shared_ptr<system::Context>& context,
                const file::Path& path,
                const ImageSeqAudio& imageSeqAudio,
                const std::vector<std::string>& imageSeqAudioExts,
                const std::string& imageSeqAudioFileName,
                const file::PathOptions& pathOptions)
            {
                file::Path out;
                auto ioSystem = context->getSystem<io::ReadSystem>();
                switch (imageSeqAudio)
                {
                case ImageSeqAudio::Ext:
                {
                    // Check for an audio file with the same base name.
                    std::vector<std::string> baseNames;
                    baseNames.push_back(path.getDirectory() + path.getBaseName());
                    std::string tmp = path.getBaseName();
                    if (!tmp.empty() && '.' == tmp[tmp.size() - 1])
                    {
                        tmp.pop_back();
                    }
                    baseNames.push_back(path.getDirectory() + tmp);
                    for (const auto& baseName : baseNames)
                    {
                        for (const auto& ext : imageSeqAudioExts)
                        {
                            const file::Path audioPath(baseName + ext,
                                                       pathOptions);
                            if (std::filesystem::exists(std::filesystem::u8path(audioPath.get())))
                            {
                                out = audioPath;
                                break;
                            }
                        }
                    }

                    // Or use the first audio file.
                    if (out.isEmpty())
                    {
                        file::DirListOptions listOptions;
                        listOptions.filterExt = imageSeqAudioExts;
                        const auto entries = file::dirList(path.getDirectory(), listOptions);
                        if (!entries.empty())
                        {
                            out = entries.front().path;
                        }
                    }

                    break;
                }
                case ImageSeqAudio::FileName:
                    out = file::Path(path.getDirectory() +
                                     imageSeqAudioFileName, pathOptions);
                    break;
                default: break;
                }
                return out;
            }

            //! An absolute, normalized form of a media path, used only to
            //! compare paths that name the same file in different ways.
            std::string normalMediaPath(const file::Path& path)
            {
                std::filesystem::path out = std::filesystem::u8path(path.get());
                if (!out.is_absolute())
                {
                    std::error_code ec;
                    const std::filesystem::path abs = std::filesystem::absolute(out, ec);
                    if (!ec)
                    {
                        out = abs;
                    }
                }
                return out.lexically_normal().u8string();
            }
        }

        namespace
        {
            const std::chrono::milliseconds timeout(5);

            // How often an otherwise idle timeline wakes to log itself, and so
            // how long it will wait for a request before looking again.
            const std::chrono::seconds logInterval(10);
            // How long a timeline without a thread waits for one of its own
            // requests before giving up on it.
            const std::chrono::seconds syncRequestTimeout(60);

            //! Get the OTIO spatial coordinates of a media reference. These are
            //! optional; media without them is laid out from the image size as
            //! before. The coordinates are returned as authored, in the OTIO
            //! coordinate system: unit-less and Y-up.
            std::optional<math::Box2f> getMediaReferenceBounds(
                const OTIO_NS::MediaReference* otioMediaReference)
            {
                std::optional<math::Box2f> out;
                if (otioMediaReference)
                {
                    const auto bounds = otioMediaReference->available_image_bounds();
                    if (bounds.has_value())
                    {
                        const auto& min = bounds.value().min;
                        const auto& max = bounds.value().max;
                        out = math::Box2f(
                            math::Vector2f(min.x, min.y),
                            math::Vector2f(max.x, max.y));
                    }
                }
                return out;
            }

            //! Get the OTIO spatial coordinates of a clip's active media
            //! reference.
            std::optional<math::Box2f> getClipBounds(const OTIO_NS::Clip* otioClip)
            {
                return getMediaReferenceBounds(otioClip->media_reference());
            }

            //! Look on disk for the frames a sequence has and group them into runs
            //! of consecutive numbers, one per clip.
            //!
            //! This is the one place that goes looking, and it is a snapshot:
            //! frames written after it are picked up by opening the sequence again,
            //! not while it is being watched. The other policies need no such thing
            //! because they answer for a missing frame as they meet it.
            std::vector<math::Int64Range> getRuns(
                const file::Path& path,
                const math::Int64Range& within,
                const file::PathOptions& pathOptions)
            {
                std::vector<math::Int64Range> out;
                auto frames = file::toFrames(file::findSeq(path, pathOptions));
                std::sort(frames.begin(), frames.end());
                for (int64_t frame : frames)
                {
                    if (frame < within.min() || frame > within.max())
                    {
                        // Outside the range asked for, so not this sequence's
                        // business even though it sits beside it on disk.
                        continue;
                    }
                    if (!out.empty() && out.back().max() + 1 == frame)
                    {
                        out.back() = math::Int64Range(out.back().min(), frame);
                    }
                    else
                    {
                        out.push_back(math::Int64Range(frame, frame));
                    }
                }
                return out;
            }

            //! Get the union of the OTIO spatial coordinates of every media
            //! reference on a clip. The canvas is built from this rather than from
            //! the active reference, so that changing the active media reference
            //! leaves the canvas unchanged.
            std::optional<math::Box2f> getClipBoundsUnion(const OTIO_NS::Clip* otioClip)
            {
                std::optional<math::Box2f> out;
                for (const auto& i : otioClip->media_references())
                {
                    if (const auto bounds = getMediaReferenceBounds(i.second))
                    {
                        out = out.has_value() ?
                              math::expand(out.value(), bounds.value()) :
                              bounds.value();
                    }
                }
                return out;
            }

            //! Convert OTIO spatial coordinates into image space.
            //!
            //! The OTIO coordinates are unit-less, so they are scaled by the
            //! pixels per unit established from the first clip that has them;
            //! bounds of "0, 0, 1920, 1080" and "0, 0, 16, 9" describe the same
            //! area and must give the same result. The Y axis is also flipped,
            //! since OTIO is Y-up and image space is Y-down.
            std::optional<math::Box2f> toImageSpace(
                const std::optional<math::Box2f>& bounds,
                double scale)
            {
                std::optional<math::Box2f> out;
                if (bounds.has_value())
                {
                    const auto& min = bounds.value().min;
                    const auto& max = bounds.value().max;
                    out = math::Box2f(
                        math::Vector2f(min.x * scale, -max.y * scale),
                        math::Vector2f(max.x * scale, -min.y * scale));
                }
                return out;
            }

            //! Resolve which media reference a clip should be read from.
            //!
            //! A key set for the clip alone takes precedence over the timeline
            //! wide key. Clips that do not have the requested key fall back to
            //! the default media key, and then to the media reference OTIO has
            //! active.
            OTIO_NS::MediaReference* resolveMediaReference(
                const OTIO_NS::Clip* otioClip,
                const std::string& key,
                const std::map<const OTIO_NS::Clip*, std::string>& clipKeys)
            {
                std::string clipKey = key;
                const auto i = clipKeys.find(otioClip);
                if (i != clipKeys.end() && !i->second.empty())
                {
                    clipKey = i->second;
                }
                if (clipKey.empty())
                {
                    // The common case, where no key has been set. Return early so
                    // that the media reference map is not copied.
                    return otioClip->media_reference();
                }
                const auto mediaReferences = otioClip->media_references();
                auto j = mediaReferences.find(clipKey);
                if (j == mediaReferences.end())
                {
                    j = mediaReferences.find(OTIO_NS::Clip::default_media_key);
                }
                return j != mediaReferences.end() ?
                    j->second :
                    otioClip->media_reference();
            }

            //! Get a clip's box in image space, before it is placed on the canvas.
            //!
            //! With Spatial::Normalize a clip that has no spatial coordinates is
            //! given the reference size, so that clips of differing resolutions
            //! are displayed at the same size. This covers timelines that were not
            //! authored with spatial coordinates at all.
            std::optional<math::Box2f> getSpatialBounds(
                const std::optional<math::Box2f>& clipBounds,
                Spatial spatial,
                const math::Size2i& normalizeSize,
                double scale)
            {
                std::optional<math::Box2f> out;
                if (Spatial::kNone == spatial)
                {
                    return out;
                }
                out = toImageSpace(clipBounds, scale);
                if (!out.has_value() &&
                    Spatial::Normalize == spatial &&
                    normalizeSize.isValid())
                {
                    out = math::Box2f(
                        math::Vector2f(0.F, -static_cast<float>(normalizeSize.h)),
                        math::Vector2f(static_cast<float>(normalizeSize.w), 0.F));
                }
                return out;
            }

            //! Get a clip's box within the timeline canvas.
            std::optional<math::Box2f> getCanvasBox(
                const std::optional<math::Box2f>& clipBounds,
                Spatial spatial,
                const math::Size2i& normalizeSize,
                double scale,
                const math::Vector2f& offset)
            {
                std::optional<math::Box2f> out;
                if (const auto bounds = getSpatialBounds(
                        clipBounds,
                        spatial,
                        normalizeSize,
                        scale))
                {
                    out = bounds.value() + offset;
                }
                return out;
            }

        }  // namespace

        void Timeline::_init(
            const std::shared_ptr<system::Context>& context,
            file::Path& inputPath,
            file::Path& inputAudioPath,
            const Options& options)
        {
            TLRENDER_P();

            auto logSystem = context->getLogSystem();
            logSystem->print(
                "tl::Timeline::_init",
                string::Format(
                    "\n"
                    "    Path: {0}\n"
                    "    Audio path: {1}").
                arg(inputPath.get()).
                arg(inputAudioPath.get()));

            OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> otioTimeline;

            // Is the input a sequence?
            const std::vector<std::string> seqExts = getExtensions(
                context,
                static_cast<int>(io::FileType::Sequence));
            const bool hasSeqExt = std::find(
                seqExts.begin(),
                seqExts.end(),
                string::toLower(inputPath.getExtension())) != seqExts.end();
            if (hasSeqExt && options.seqExpand)
            {
                inputPath = file::expandSeq(inputPath, options.pathOptions);
            }
            if (hasSeqExt && inputPath.isSequence())
            {
                if (inputAudioPath.isEmpty())
                {
                    // Check for an associated audio file.
                    inputAudioPath = getAssociatedAudio(
                        context,
                        inputPath,
                        options.imageSeqAudio,
                        options.imageSeqAudioExts,
                        options.imageSeqAudioFileName,
                        options.pathOptions);
                }
            }

            // Read the file. A sequence is read by a decoder, which holds no
            // thread; only a format that has to be read statefully still needs
            // a reader here.
            auto ioSystem = context->getSystem<io::ReadSystem>();
            io::Info info;
            bool infoValid = false;
            if (auto plugin = ioSystem->getPlugin(inputPath))
            {
                if (auto decode = plugin->decode(options.ioOptions))
                {
                    info = io::SeqDecode::create(
                        inputPath, {}, decode, options.ioOptions)->getInfo();
                    infoValid = true;
                }
            }
            if (!infoValid)
            {
                // Which tracks this file gets depends on both halves, so both
                // are read and merged here. The readers are temporary: what
                // the timeline goes on to read is decided by the tracks below.
                auto videoRead = ioSystem->videoRead(inputPath,
                                                     options.ioOptions);
                auto audioRead = ioSystem->audioRead(inputPath,
                                                     options.ioOptions);
                std::future<io::Info> videoFuture;
                std::future<io::Info> audioFuture;
                if (videoRead)
                {
                    videoFuture = videoRead->getInfo();
                }
                if (audioRead)
                {
                    audioFuture = audioRead->getInfo();
                }
                io::Info videoInfo;
                if (videoFuture.valid())
                {
                    videoInfo = videoFuture.get();
                    infoValid = true;
                }
                io::Info audioInfo;
                if (audioFuture.valid())
                {
                    audioInfo = audioFuture.get();
                    infoValid = true;
                }
                if (infoValid)
                {
                    info = merge(videoInfo, audioInfo);
                }
            }
            if (infoValid)
            {
                std::optional<OTIO_NS::RationalTime> startTime;
                OTIO_NS::Track* videoTrack = nullptr;
                OTIO_NS::Track* audioTrack = nullptr;

                // Read the video.
                if (!info.video.empty())
                {
                    startTime = info.videoTime->start_time();
                    const double rate = info.videoTime->duration().rate();
                    const io::MissingFrames missingFrames =
                        io::getMissingFrames(options.ioOptions);

                    // Every clip names the whole sequence over the whole range
                    // it covers, whatever the clip itself takes out of it.
                    // They are separate objects because a clip owns its
                    // reference, but they describe the same file, so the reads
                    // behind them share one decoder.
                    const auto makeClip =
                        [&](const OTIO_NS::TimeRange& sourceRange)
                            {
                                auto out = new OTIO_NS::Clip;
                                out->set_source_range(sourceRange);
                                if (inputPath.isSequence())
                                {
                                    auto mediaReference =
                                        new OTIO_NS::ImageSequenceReference(
                                            "",
                                            inputPath.getBaseName(),
                                            inputPath.getExtension(),
                                            info.videoTime->start_time().value(),
                                            1,
                                            rate,
                                            inputPath.getPadding(),
                                            // A file opened directly has no reference
                                            // to say what to do about frames it is
                                            // missing, so it takes the options the
                                            // timeline was opened with.
                                            toOTIO(missingFrames));
                                    mediaReference->set_available_range(*info.videoTime);
                                    out->set_media_reference(mediaReference);
                                }
                                else
                                {
                                    out->set_media_reference(
                                        new OTIO_NS::ExternalReference(
                                            inputPath.getFileName(),
                                            info.videoTime));
                                }
                                return out;
                            };

                    // A structural policy is answered here rather than by the
                    // reads: the frames that are there are found once, and a clip
                    // is laid over each run of them. Skip puts the runs end to
                    // end, so the timeline is only as long as the frames it has;
                    // Gaps leaves the holes in, so every frame keeps the time it
                    // had. Either way no read asks for a frame that is not there.
                    std::vector<math::Int64Range> runs;
                    if (inputPath.isSequence() &&
                        io::isStructural(missingFrames) &&
                        inputPath.getFrames().has_value())
                    {
                        runs = getRuns(
                            inputPath,
                            inputPath.getFrames().value(),
                            options.pathOptions);
                    }

                    videoTrack = new OTIO_NS::Track(
                        "Video", std::nullopt, OTIO_NS::Track::Kind::video);
                    if (runs.size() < 2 && io::MissingFrames::Gaps != missingFrames)
                    {
                        // Nothing to take out, so this is the same single clip a
                        // complete sequence gets. A lone run still covers only
                        // itself, which is what Skip means when the frames that
                        // are there are consecutive.
                        videoTrack->append_child(makeClip(
                                                     runs.empty() ?
                                                     *info.videoTime :
                                                     OTIO_NS::TimeRange(
                                                         OTIO_NS::RationalTime(runs.front().min(), rate),
                                                         OTIO_NS::RationalTime(
                                                             runs.front().max() - runs.front().min() + 1,
                                                             rate))));
                    }
                    else
                    {
                        const math::Int64Range& frames =
                            inputPath.getFrames().value();
                        int64_t at = frames.min();
                        for (const auto& run : runs)
                        {
                            if (io::MissingFrames::Gaps == missingFrames &&
                                run.min() > at)
                            {
                                videoTrack->append_child(new OTIO_NS::Gap(
                                                             OTIO_NS::RationalTime(run.min() - at, rate)));
                            }
                            videoTrack->append_child(makeClip(OTIO_NS::TimeRange(
                                                                  OTIO_NS::RationalTime(run.min(), rate),
                                                                  OTIO_NS::RationalTime(
                                                                      run.max() - run.min() + 1, rate))));
                            at = run.max() + 1;
                        }
                        if (io::MissingFrames::Gaps == missingFrames &&
                            at <= frames.max())
                        {
                            // The tail of a render that has not got there yet, kept
                            // so the range asked for is the range shown.
                            videoTrack->append_child(new OTIO_NS::Gap(
                                                         OTIO_NS::RationalTime(frames.max() - at + 1, rate)));
                        }
                    }
                }

                // Read the separate audio if provided.
                if (!inputAudioPath.isEmpty())
                {
                    if (auto audioRead = ioSystem->audioRead(inputAudioPath, options.ioOptions))
                    {
                        const auto audioInfo = audioRead->getInfo().get();

                        auto audioClip = new OTIO_NS::Clip;
                        audioClip->set_source_range(*audioInfo.audioTime);
                        audioClip->set_media_reference(new OTIO_NS::ExternalReference(
                                                           inputAudioPath.getFileName(),
                                                           audioInfo.audioTime));

                        audioTrack = new OTIO_NS::Track("Audio", std::nullopt, OTIO_NS::Track::Kind::audio);
                        audioTrack->append_child(audioClip);
                    }
                }
                else if (info.audio.isValid())
                {
                    if (!startTime.has_value())
                    {
                        startTime = info.audioTime->start_time();
                    }

                    auto audioClip = new OTIO_NS::Clip;
                    audioClip->set_source_range(*info.audioTime);
                    audioClip->set_media_reference(new OTIO_NS::ExternalReference(
                                                       inputPath.getFileName(),
                                                       info.audioTime));

                    audioTrack = new OTIO_NS::Track("Audio", std::nullopt, OTIO_NS::Track::Kind::audio);
                    audioTrack->append_child(audioClip);
                }

                // Create the stack.
                auto otioStack = new OTIO_NS::Stack;
                if (videoTrack)
                {
                    otioStack->append_child(videoTrack);
                }
                if (audioTrack)
                {
                    otioStack->append_child(audioTrack);
                }

                // Create the timeline.
                otioTimeline = new OTIO_NS::Timeline(inputPath.get());
                otioTimeline->set_tracks(otioStack);
                if (startTime.has_value())
                {
                    otioTimeline->set_global_start_time(startTime);
                }
            }

            // Is the input an OTIO file?
            if (!otioTimeline)
            {
                const std::string fileName = inputPath.get();
                const std::string ext = string::toLower(inputPath.getExtension());
                OTIO_NS::ErrorStatus otioError;
                if (".otio" == ext)
                {
                    otioTimeline = dynamic_cast<OTIO_NS::Timeline*>(
                        OTIO_NS::Timeline::from_json_file(fileName, &otioError));
                    if (!otioTimeline)
                    {
                        throw std::runtime_error(
                            string::Format("Cannot read timeline: \"{0}\"").
                            arg(inputPath.get()));
                    }
                    else if (OTIO_NS::is_error(otioError))
                    {
                        throw std::runtime_error(
                            string::Format("Cannot read timeline: \"{0}\": {1}").
                            arg(inputPath.get()).
                            arg(otioError.details));
                    }
                }
                else if (".otioz" == ext)
                {
                    // Read as scattered ranges rather than start to finish:
                    // opening reads a local header per media file, and those
                    // are spread across the whole bundle, one before each
                    // file's data. Asking for sequential read ahead makes the
                    // operating system fetch around every one of them and
                    // then throw it away.
                    p.fileIO = file::FileIO::create(
                        fileName,
                        file::Mode::Read,
                        file::Read::MemoryMapped,
                        file::Access::Random);

                    p.zipReader = std::make_shared<ZipReader>(logSystem);
                    auto& zipReader = *p.zipReader;
                    zipReader.open(fileName, p.fileIO->getSize());

                    std::string json = zipReader.readText("content.otio");
                    otioTimeline = dynamic_cast<OTIO_NS::Timeline*>(
                        OTIO_NS::Timeline::from_json_string(json, &otioError));
                    if (!otioTimeline)
                    {
                        throw std::runtime_error(
                            string::Format("Cannot read timeline: \"{0}\"").
                            arg(inputPath.get()));
                    }
                    else if (OTIO_NS::is_error(otioError))
                    {
                        throw std::runtime_error(
                            string::Format("Cannot read timeline: \"{0}\": {1}").
                            arg(inputPath.get()).
                            arg(otioError.details));
                    }

                    // Map a media reference to the memory it occupies within the
                    // bundle.
                    //
                    // The bundle is missing the media it is playing if the active
                    // reference is not there, so that is an error. An alternate
                    // that is missing only costs the ability to switch to it, so
                    // the timeline is still opened and the reference is recorded
                    // as unavailable. Either way the media is never read from its
                    // path: a bundle is meant to be self contained, and quietly
                    // reading a file from somewhere else would be misleading.
                    // Record which references the bundle holds, and check the
                    // first file of each so that a bundle missing its media is
                    // still reported at open. Working out every frame's byte
                    // range waits until the reference is read: for a bundle of
                    // 25,000 frames that was seconds of URL decoding and path
                    // parsing before anything appeared.
                    const auto mapMediaReference = [&](
                        OTIO_NS::MediaReference* mediaReference,
                        bool active)
                        {
                            if (!mediaReference ||
                                p.bundleMediaReferences.find(mediaReference) !=
                                p.bundleMediaReferences.end())
                            {
                                return;
                            }

                            std::string first;
                            if (auto externalReference =
                                dynamic_cast<OTIO_NS::ExternalReference*>(mediaReference))
                            {
                                first = file::Path(
                                    url::decode(externalReference->target_url())).get();
                            }
                            else if (auto imageSeqReference =
                                     dynamic_cast<OTIO_NS::ImageSequenceReference*>(mediaReference))
                            {
                                if (imageSeqReference->number_of_images_in_sequence() <= 0)
                                {
                                    return;
                                }
                                first = file::Path(url::decode(
                                                       imageSeqReference->target_url_for_image_number(0))).get();
                            }
                            else
                            {
                                return;
                            }

                            if (!zipReader.find(first).has_value())
                            {
                                if (active)
                                {
                                    throw std::runtime_error(string::Format(
                                                                 "Cannot find zip entry: \"{0}\"").arg(first));
                                }
                                logSystem->print(
                                    "tl::Timeline",
                                    string::Format(
                                        "Cannot find zip entry: \"{0}\"; this media "
                                        "reference cannot be used").
                                    arg(first),
                                    log::Type::Warning);
                                p.unavailableMediaReferences.insert(mediaReference);
                                return;
                            }
                            p.bundleMediaReferences.insert(mediaReference);
                        };

                    // Map every media reference, not only the active one, so that
                    // the active reference can be changed without re-reading the
                    // bundle.
                    for (auto clip : otioTimeline->find_children<OTIO_NS::Clip>())
                    {
                        const auto* activeReference = clip->media_reference();
                        for (const auto& i : clip->media_references())
                        {
                            mapMediaReference(i.second, i.second == activeReference);
                        }
                    }
                }
            }

            if (!otioTimeline)
            {
                // Nothing claimed the file and it is not a timeline document.
                // Whether that is because the format is not supported at all or
                // because a supported file could not be read is the difference
                // between "try another application" and "this file is damaged",
                // so say which.
                throw std::runtime_error(
                    ioSystem->getPlugin(inputPath) ?
                    string::Format("Cannot read the file: \"{0}\"").
                    arg(inputPath.get()) :
                    string::Format("Unsupported file format: \"{0}\"").
                    arg(inputPath.get()));
            }

            OTIO_NS::AnyDictionary dict;
            dict["path"] = inputPath.get();
            dict["audioPath"] = inputAudioPath.get();
            otioTimeline->metadata()["tlRender"] = dict;

            _init(context, otioTimeline, options);
        }

        void Timeline::_init(
            const std::shared_ptr<system::Context>& context,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>&
            otioTimeline,
            const Options& options)
        {
            TLRENDER_P();

            p.context = context;
            auto logSystem = context->getLogSystem();
            p.logSystem = logSystem;
            {
                std::vector<std::string> lines;
                lines.push_back(std::string());
                lines.push_back(string::Format("    * Image sequence audio: {0}")
                                .arg(options.imageSeqAudio));
                lines.push_back(
                    string::Format("    * Image sequence audio file name: {0}")
                    .arg(options.imageSeqAudioFileName));
                lines.push_back(string::Format("    * Compatability: {0}").
                                arg(options.compat));
                lines.push_back(string::Format("    Video request count: {0}")
                                .arg(options.videoRequestMax));
                lines.push_back(string::Format("    Audio request count: {0}")
                                .arg(options.audioRequestMax));
                lines.push_back(string::Format("    Request timeout: {0}ms")
                                .arg(options.requestTimeout.count()));
                for (const auto& i : options.ioOptions)
                {
                    lines.push_back(string::Format("    AV I/O {0}: {1}")
                                    .arg(i.first)
                                    .arg(i.second));
                }
                lines.push_back(
                    string::Format("    Path max number digits: {0}")
                    .arg(options.pathOptions.seqMaxDigits));
                logSystem->print(
                    string::Format("tl::timeline::Timeline {0}").arg(this),
                    string::join(lines, "\n"));
            }

            p.context = context;
            p.otioTimeline = otioTimeline;
            p.frameCache = io::Cache::create();
            p.timelineChanges = observer::Value<bool>::create(false);
            const auto i = otioTimeline->metadata().find("tlRender");
            if (i != otioTimeline->metadata().end())
            {
                try
                {
                    const auto dict =
                        std::any_cast<OTIO_NS::AnyDictionary>(i->second);
                    auto j = dict.find("path");
                    if (j != dict.end())
                    {
                        p.path =
                            file::Path(std::any_cast<std::string>(j->second));
                    }
                    j = dict.find("audioPath");
                    if (j != dict.end())
                    {
                        p.audioPath =
                            file::Path(std::any_cast<std::string>(j->second));
                    }
                }
                catch (const std::exception&)
                {
                }
            }
            p.options = options;
            p.videoReadCache.setMax(p.options.readCacheMax);
            p.audioReadCache.setMax(p.options.readCacheMax);
            p.seqCache.setMax(p.options.seqCacheMax);

            // Get information about the timeline. A timeline whose tracks have
            // no duration is zero length rather than unset, so that everything
            // downstream has a range to work in.
            p.indexTimeline();
            for (const auto& i : p.otioTimeline.value->tracks()->children())
            {
                if (auto otioTrack = dynamic_cast<const OTIO_NS::Track*>(i.value))
                {
                    if (OTIO_NS::Track::Kind::audio == otioTrack->kind())
                    {
                        if (_getAudioInfo(otioTrack))
                        {
                            auto j = p.options.ioOptions.find(
                                "FFmpeg/AudioChannelCount");
                            if (j == p.options.ioOptions.end())
                            {
                                p.options
                                    .ioOptions["FFmpeg/AudioChannelCount"] =
                                    string::Format("{0}").arg(
                                        p.ioInfo.audio.channelCount);
                            }
                            j = p.options.ioOptions.find(
                                "FFmpeg/AudioType");
                            if (j == p.options.ioOptions.end())
                            {
                                p.options.ioOptions["FFmpeg/AudioType"] =
                                    string::Format("{0}").arg(
                                        p.ioInfo.audio.dataType);
                            }
                            j = p.options.ioOptions.find(
                                "FFmpeg/AudioSampleRate");
                            if (j == p.options.ioOptions.end())
                            {
                                p.options.ioOptions["FFmpeg/AudioSampleRate"] =
                                    string::Format("{0}").arg(
                                        p.ioInfo.audio.sampleRate);
                            }
                            break;
                        }
                    }
                }
            }
            _timelineUpdate();

            logSystem->print(
                string::Format("tl::timeline::Timeline {0}").arg(this),
                string::Format("\n"
                               "    Time range: {0}\n"
                               "    Video: {1} {2}\n"
                               "    Audio: {3} {4} {5}")
                .arg(p.timeRange)
                .arg(
                    !p.ioInfo.video.empty() ? p.ioInfo.video[0].size
                    : image::Size())
                .arg(
                    !p.ioInfo.video.empty() ? p.ioInfo.video[0].pixelType
                    : image::PixelType::kNone)
                .arg(p.ioInfo.audio.channelCount)
                .arg(p.ioInfo.audio.dataType)
                .arg(p.ioInfo.audio.sampleRate));

            // Create a new thread.
            p.mutex.otioTimeline = p.otioTimeline;
            p.thread.running = true;
            p.thread.logTimer = std::chrono::steady_clock::now();
            if (p.options.threaded)
            {
                p.startReadPool(p.options.readThreadCount);
                p.thread.thread = std::thread(
                    [this]
                        {
                            TLRENDER_P();
                            while (p.thread.running)
                            {
                                _tick();
                            }
                            _finishRequests();
                        });
            }
        }


        namespace
        {
            std::atomic<size_t> objectCount = 0;
        }

        float Timeline::_transitionValue(double frame, double in, double out) const
        {
            return (frame - in) / (out - in);
        }

        Timeline::Timeline() :
            _p(new Private)
        {
            ++objectCount;
        }

        Timeline::~Timeline()
        {
            TLRENDER_P();

            if (auto logSystem = p.logSystem.lock())
            {
                logSystem->print(
                    string::Format("tl::~Timeline {0}").arg(this),
                    p.path.get());
            }

            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.thread.running = false;
            }
            p.thread.cv.notify_one();
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
            p.stopReadPool();

            --objectCount;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const std::shared_ptr<system::Context>& context,
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& timeline,
            const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            out->_init(context, timeline, options);
            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const std::shared_ptr<system::Context>& context,
            file::Path& path,
            const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            file::Path empty;
            out->_init(context, path, empty, options);
            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const std::shared_ptr<system::Context>& context,
            file::Path& path,
            file::Path& audioPath,
            const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            out->_init(context, path, audioPath, options);
            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const std::shared_ptr<system::Context>& context,
            const std::string& fileName,
            const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            file::Path path(fileName, options.pathOptions);
            file::Path empty;
            out->_init(
                context,
                path,
                empty,
                options);
            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const std::shared_ptr<system::Context>& context,
            const std::string& fileName,
            const std::string& audioFileName,
            const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            file::Path path(fileName, options.pathOptions);
            file::Path audioPath(audioFileName, options.pathOptions);
            out->_init(
                context,
                path,
                audioPath,
                options);
            return out;
        }

        const std::weak_ptr<system::Context>& Timeline::getContext() const
        {
            return _p->context;
        }

        const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>&
        Timeline::getTimeline() const
        {
            return _p->otioTimeline;
        }

        std::shared_ptr<observer::IValue<bool> >
        Timeline::observeTimelineChanges() const
        {
            return _p->timelineChanges;
        }

        void Timeline::setTimeline(
            const OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline>& value)
        {
            TLRENDER_P();

            // Only one of these at a time: the sequence below stops and
            // restarts the request thread and read pool, which is not safe
            // to do from two threads at once.
            std::unique_lock<std::mutex> setTimelineLock(p.setTimelineMutex);

            // Stop the request thread and read pool before touching
            // anything they read without a lock -- otioTimeline itself, the
            // indexes indexTimeline() rebuilds below, and the time range,
            // caches and canvas/video info _timelineUpdate() recomputes.
            // Previously this function swapped otioTimeline and called
            // _timelineUpdate() while _requests() and the read pool were
            // still running against the old timeline, racing with them.
            // This mirrors the stop sequence in ~Timeline(), except the
            // thread and pool are started back up afterwards instead of
            // staying down; any getVideo()/getAudio() request that arrives
            // in the meantime comes back with an empty frame immediately,
            // the same as it would after the timeline is destroyed.
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.thread.running = false;
            }
            p.thread.cv.notify_one();
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
            p.stopReadPool();

            // The request thread and read pool are stopped, so it is now
            // safe to swap in the new timeline and rebuild everything
            // derived from it -- including trimmedRangeInParent, trackItems,
            // mediaByPath and mediaByNormalPath, which used to be built only
            // once, in _init(), and were left pointing at the composables
            // and media references of whichever timeline was previously set.
            // ABA hazard: a media reference in the *new* timeline could be
            // allocated at the same address as a freed one from the old
            // timeline and be wrongly treated as already resolved, already
            // known-unavailable, or already read into memFiles.
            //
            // The zip reader and its memory mapped file are only good for
            // resolving byte ranges for those same, now-cleared, bundle
            // media references -- nothing maps the new timeline's media
            // references into the bundle the way _init() does when it first
            // opens an .otioz -- so they are dropped here too rather than
            // left open for no reason.
            {
                std::unique_lock<std::mutex> lock(p.memFilesMutex);
                p.memFiles.clear();
                p.bundleMediaReferences.clear();
                p.unavailableMediaReferences.clear();
            }
            p.zipReader.reset();
            p.fileIO.reset();

            // The request thread and read pool are stopped, so it is now
            // safe to swap in the new timeline and rebuild everything
            // derived from it -- including trimmedRangeInParent, trackItems,
            // mediaByPath and mediaByNormalPath, which used to be built only
            // once, in _init(), and were left pointing at the composables
            // and media references of whichever timeline was previously set.
            p.otioTimeline = value;
            p.indexTimeline();
            if (p.otioTimeline.value)
            {
                _timelineUpdate();
            }

            // Start the request thread and read pool back up, mirroring the
            // end of _init(). mutex.stopped has to be cleared explicitly:
            // _finishRequests(), run by the thread just joined above, set it
            // when that thread exited.
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                p.mutex.stopped = false;
                p.mutex.otioTimeline = p.otioTimeline;
            }
            p.thread.running = true;
            p.thread.logTimer = std::chrono::steady_clock::now();
            if (p.options.threaded)
            {
                p.startReadPool(p.options.readThreadCount);
                p.thread.thread = std::thread(
                    [this]
                        {
                            TLRENDER_P();
                            while (p.thread.running)
                            {
                                _tick();
                            }
                            _finishRequests();
                        });
            }
        }

        const file::Path& Timeline::getPath() const
        {
            return _p->path;
        }

        const file::Path& Timeline::getAudioPath() const
        {
            return _p->audioPath;
        }

        const Options& Timeline::getOptions() const
        {
            return _p->options;
        }

        std::future<io::VideoData> Timeline::readMedia(
            const file::Path& path,
            const OTIO_NS::RationalTime& time,
            const io::Options& options)
        {
            TLRENDER_P();
            std::future<io::VideoData> out;
            const io::Options optionsMerged = io::merge(options, p.options.ioOptions);
            if (auto mediaReference = _findMedia(path))
            {
                if (auto seq = _getSeqDecode(mediaReference, optionsMerged))
                {
                    out = p.submitRead(
                        [seq, time, optionsMerged]
                            {
                                return seq->readVideo(time, optionsMerged);
                            });
                }
                else if (auto videoRead = _getVideoRead(mediaReference, optionsMerged))
                {
                    out = videoRead->readVideo(time, optionsMerged);
                }
            }
            return out;
        }

        std::future<io::AudioData> Timeline::readMediaAudio(
            const file::Path& path,
            const OTIO_NS::TimeRange& timeRange,
            const io::Options& options)
        {
            TLRENDER_P();
            std::future<io::AudioData> out;
            const io::Options optionsMerged = io::merge(options, p.options.ioOptions);
            if (auto mediaReference = _findMedia(path))
            {
                // Audio is never a sequence of stateless files.
                if (auto audioRead = _getAudioRead(mediaReference, optionsMerged))
                {
                    out = audioRead->readAudio(timeRange, optionsMerged);
                }
            }
            return out;
        }

        std::vector<file::MemoryRead>
        Timeline::getMem(const OTIO_NS::MediaReference* otioRef)
        {
            TLRENDER_P();
            return *p.getMem(otioRef);
        }

        std::shared_ptr<std::vector<file::MemoryRead> >
        Timeline::Private::getMem(const OTIO_NS::MediaReference* otioRef)
        {
            std::unique_lock<std::mutex> lock(memFilesMutex);
            if (const auto i = memFiles.find(otioRef); i != memFiles.end())
            {
                return i->second;
            }
            if (bundleMediaReferences.find(otioRef) ==
                bundleMediaReferences.end())
            {
                // Not in a bundle: read from its path.
                return std::make_shared<std::vector<file::MemoryRead> >();
            }

            // First use of this reference: work out where each of its files
            // lives inside the bundle.
            //
            // A sequence member is placed at its offset from the first frame
            // rather than packed against the previous one, so that the result
            // is indexed by frame number. Packing them would misplace every
            // frame/ after a gap, and every frame at all when the step is
            // greater than one.
            auto out = std::make_shared<std::vector<file::MemoryRead> >();
            std::vector<std::pair<size_t, std::string> > mediaFileNames;
            if (auto externalReference = dynamic_cast<const OTIO_NS::ExternalReference*>(otioRef))
            {
                mediaFileNames.push_back(std::make_pair(
                                             size_t(0),
                                             file::Path(url::decode(externalReference->target_url())).get()));
            }
            else if (auto imageSeqReference =
                     dynamic_cast<const OTIO_NS::ImageSequenceReference*>(otioRef))
            {
                const int count = imageSeqReference->number_of_images_in_sequence();
                const size_t step = std::max(imageSeqReference->frame_step(), 1);
                mediaFileNames.reserve(count);
                for (int number = 0; number < count; ++number)
                {
                    mediaFileNames.push_back(std::make_pair(
                                                 number * step,
                                                 file::Path(url::decode(
                                                               imageSeqReference->target_url_for_image_number(number))).get()));
                }
            }
            if (!mediaFileNames.empty())
            {
                out->resize(mediaFileNames.back().first + 1);
            }
            size_t found = 0;
            std::string missing;
            size_t missingCount = 0;
            for (const auto& mediaFileName : mediaFileNames)
            {
                const auto entry = zipReader->find(mediaFileName.second);
                if (!entry.has_value())
                {
                    // A sequence member the bundle does not hold is a missing
                    // frame, which the sequence decoder deals with. Leave its slot
                    // empty and carry on.
                    ++missingCount;
                    if (missing.empty())
                    {
                        missing = mediaFileName.second;
                    }
                    continue;
                }
                (*out)[mediaFileName.first] = file::MemoryRead(
                    fileIO,
                    fileIO->getMemoryStart() + entry->offset,
                    entry->size);
                ++found;
            }
            if (0 == found)
            {
                // The bundle holds none of this media. Mark the reference
                // unavailable rather than returning nothing: an empty result reads
                // as "not in a bundle", and the caller would go on to read the
                // media from its path, which is a different file than the bundle
                // describes.
                if (auto log = logSystem.lock())
                {
                    log->print(
                        "tl::Timeline",
                        string::Format(
                            "Cannot find zip entry: \"{0}\"; this media "
                            "reference cannot be used").arg(missing),
                        log::Type::Error);
                }
                unavailableMediaReferences.insert(otioRef);
                out->clear();
            }
            else if (missingCount > 0)
            {
                if (auto log = logSystem.lock())
                {
                    log->print(
                        "tl::Timeline",
                        string::Format(
                            "Bundle is missing {0} of {1} sequence frames, "
                            "starting with \"{2}\"").
                        arg(missingCount).
                        arg(mediaFileNames.size()).
                        arg(missing),
                        log::Type::Warning);
                }
            }
            memFiles[otioRef] = out;
            return out;
        }

        OTIO_NS::MediaReference* Timeline::Private::mediaReference(
            const OTIO_NS::Clip* otioClip) const
        {
            return resolveMediaReference(
                otioClip,
                thread.mediaReferenceKey,
                thread.clipMediaReferenceKeys);
        }

        std::optional<OTIO_NS::TimeRange>
        Timeline::Private::getTrimmedRangeInParent(
            const OTIO_NS::Composable* otioComposable) const
        {
            if (const auto i = trimmedRangeInParent.find(otioComposable);
                i != trimmedRangeInParent.end())
            {
                return i->second;
            }
            if (auto otioItem = dynamic_cast<const OTIO_NS::Item*>(otioComposable))
            {
                return otioItem->trimmed_range_in_parent();
            }
            return std::nullopt;
        }

        std::vector<OTIO_NS::Composable*> Timeline::Private::getTrackChildrenAt(
            const OTIO_NS::Track* otioTrack,
            const OTIO_NS::RationalTime& time) const
        {
            std::vector<OTIO_NS::Composable*> out;
            const auto i = trackItems.find(otioTrack);
            if (i == trackItems.end())
            {
                for (const auto& otioChild : otioTrack->children())
                {
                    out.push_back(otioChild.value);
                }
                return out;
            }
            const auto& items = i->second;
            auto j = std::upper_bound(
                items.begin(),
                items.end(),
                time,
                [](const OTIO_NS::RationalTime& value, const TrackItem& item)
                    {
                        return value < item.range.start_time();
                    });
            if (j != items.begin())
            {
                --j;
                if (j->range.contains(time))
                {
                    out.push_back(j->item);
                }
            }
            return out;
        }

        std::vector<std::string> Timeline::getMediaReferenceKeys() const
        {
            TLRENDER_P();

            std::vector<std::string> out;
            if (!p.otioTimeline.value)
                return out;

            std::set<std::string> keys;
            for (const auto& otioClip :
                     p.otioTimeline.value->find_children<OTIO_NS::Clip>())
            {
                for (const auto& i : otioClip->media_references())
                {
                    keys.insert(i.first);
                }
            }
            out = std::vector<std::string>(keys.begin(), keys.end());
            return out;
        }

        std::string Timeline::getMediaReferenceKey() const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            return p.mutex.mediaReferenceKey;
        }

        void Timeline::setMediaReferenceKey(const std::string& value)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            if (value != p.mutex.mediaReferenceKey)
            {
                p.mutex.mediaReferenceKey = value;
                p.mutex.mediaReferenceKeysChanged = true;
            }
        }

        std::string Timeline::getMediaReferenceKey(
            const OTIO_NS::Clip* otioClip) const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            const auto i = p.mutex.clipMediaReferenceKeys.find(otioClip);
            return i != p.mutex.clipMediaReferenceKeys.end() ?
                i->second :
                std::string();
        }

        void Timeline::setMediaReferenceKey(
            const OTIO_NS::Clip* otioClip,
            const std::string& value)
        {
            TLRENDER_P();

            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            if (value.empty())
            {
                if (p.mutex.clipMediaReferenceKeys.erase(otioClip) > 0)
                {
                    p.mutex.mediaReferenceKeysChanged = true;
                }
            }
            else
            {
                auto& key = p.mutex.clipMediaReferenceKeys[otioClip];
                if (value != key)
                {
                    key = value;
                    p.mutex.mediaReferenceKeysChanged = true;
                }
            }
        }

        OTIO_NS::MediaReference* Timeline::getMediaReference(
            const OTIO_NS::Clip* otioClip) const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            return resolveMediaReference(
                otioClip,
                p.mutex.mediaReferenceKey,
                p.mutex.clipMediaReferenceKeys);
        }

        const OTIO_NS::TimeRange& Timeline::getTimeRange() const
        {
            return _p->timeRange;
        }

        const io::Info& Timeline::getIOInfo() const
        {
            TLRENDER_P();
            // Follow the media reference being read, so that the information
            // describes the media on screen rather than the media that
            // happened to be active when the timeline was read. The entries
            // are fixed once the timeline has been read, so returning a
            // reference to one is safe.
            if (p.videoInfoClip)
            {
                const auto i = p.videoInfoByReference.find(
                    getMediaReference(p.videoInfoClip));
                if (i != p.videoInfoByReference.end())
                {
                    return i->second;
                }
            }
            return p.ioInfo;
        }

        VideoRequest Timeline::getVideo(
            const OTIO_NS::RationalTime& time, const io::Options& options)
        {
            TLRENDER_P();
            (p.requestId)++;
            auto request = std::make_shared<Private::PendingVideoRequest>();
            request->id = p.requestId;
            request->time = time;
            request->options = options;
            VideoRequest out;
            out.id = p.requestId;
            out.future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.videoRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(VideoFrame());
            }
            return out;
        }

        AudioRequest
        Timeline::getAudio(double seconds, const io::Options& options)
        {
            TLRENDER_P();
            (p.requestId)++;
            auto request = std::make_shared<Private::PendingAudioRequest>();
            request->id = p.requestId;
            request->seconds = seconds;
            request->options = options;
            AudioRequest out;
            out.id = p.requestId;
            out.future = request->promise.get_future();
            bool valid = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                if (!p.mutex.stopped)
                {
                    valid = true;
                    p.mutex.audioRequests.push_back(request);
                }
            }
            if (valid)
            {
                p.thread.cv.notify_one();
            }
            else
            {
                request->promise.set_value(AudioFrame());
            }
            return out;
        }

        void Timeline::cancelRequests(const std::vector<uint64_t>& ids)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            {
                auto i = p.mutex.videoRequests.begin();
                while (i != p.mutex.videoRequests.end())
                {
                    const auto j = std::find(ids.begin(), ids.end(), (*i)->id);
                    if (j != ids.end())
                    {
                        i = p.mutex.videoRequests.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
            }
            {
                auto i = p.mutex.audioRequests.begin();
                while (i != p.mutex.audioRequests.end())
                {
                    const auto j = std::find(ids.begin(), ids.end(), (*i)->id);
                    if (j != ids.end())
                    {
                        i = p.mutex.audioRequests.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
            }
        }

        void Timeline::tick()
        {
            TLRENDER_P();
            bool otioTimelineChanged = false;
            {
                std::unique_lock<std::mutex> lock(p.mutex.mutex);
                otioTimelineChanged = p.mutex.otioTimelineChanged;
                p.mutex.otioTimelineChanged = false;
            }
            if (otioTimelineChanged)
            {
                p.timelineChanges->setAlways(true);
            }
        }

        void Timeline::_tick()
        {
            TLRENDER_P();

            const auto t0 = std::chrono::steady_clock::now();

            _requests();

            // Logging.
            auto t1 = std::chrono::steady_clock::now();
            const std::chrono::duration<float> diff = t1 - p.thread.logTimer;
            if (diff > logInterval)
            {
                p.thread.logTimer = t1;
                if (auto logSystem = p.logSystem.lock())
                {
                    size_t videoRequestsSize = 0;
                    size_t audioRequestsSize = 0;
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        videoRequestsSize = p.mutex.videoRequests.size();
                        audioRequestsSize = p.mutex.audioRequests.size();
                    }
                    logSystem->print(
                        string::Format("tl::Timeline {0}").arg(this),
                        string::Format(
                            "\n"
                            "    * Path: {0}\n"
                            "    * Video requests: {1}, {2} in-progress, {3} max\n"
                            "    * Audio requests: {4}, {5} in-progress, {6} max").
                        arg(p.path.get()).
                        arg(videoRequestsSize).
                         arg(p.thread.videoRequestsInProgress.size()).
                         arg(getVideoRequestMax()).
                         arg(audioRequestsSize).
                         arg(p.thread.audioRequestsInProgress.size()).
                         arg(p.options.audioRequestMax));
                 }
                 t1 = std::chrono::steady_clock::now();
             }

             // Sleep for a bit, unless the caller is driving this itself and is
             // waiting on the very request that just finished.
             if (p.options.threaded)
             {
                 //file_wrong::sleep(timeout, t0, t1);
                 time::sleep(timeout, t0, t1);
             }
         }

         void Timeline::_requests()
         {
             TLRENDER_P();

             // Gather requests.
             std::list<std::shared_ptr<Private::PendingVideoRequest> > newVideoRequests;
             std::list<std::shared_ptr<Private::PendingAudioRequest> > newAudioRequests;
             {
                 std::unique_lock<std::mutex> lock(p.mutex.mutex);
                 p.thread.cv.wait_for(
                     lock, p.options.requestTimeout,
                     [this]
                         {
                             TLRENDER_P();

                             return p.mutex.otioTimeline.value ||
                                 !p.mutex.videoRequests.empty() ||
                                 !p.thread.videoRequestsInProgress.empty() ||
                                 !p.mutex.audioRequests.empty() ||
                                 !p.thread.audioRequestsInProgress.empty();
                         });
                 if (p.mutex.otioTimeline.value)
                 {
                     p.thread.otioTimeline = p.mutex.otioTimeline;
                     p.mutex.otioTimeline = nullptr;
                     p.mutex.otioTimelineChanged = true;
                 }
                 while (!p.mutex.videoRequests.empty() &&
                        (p.thread.videoRequestsInProgress.size() +
                         newVideoRequests.size()) < p.options.videoRequestMax)
                {
                    newVideoRequests.push_back(p.mutex.videoRequests.front());
                    p.mutex.videoRequests.pop_front();
                }
                while (!p.mutex.audioRequests.empty() &&
                       (p.thread.audioRequestsInProgress.size() +
                        newAudioRequests.size()) < p.options.audioRequestMax)
                {
                    newAudioRequests.push_back(p.mutex.audioRequests.front());
                    p.mutex.audioRequests.pop_front();
                }
                // Take a copy of the media reference keys so that the rest of
                // the traversal can resolve media references without locking.
                if (p.mutex.mediaReferenceKeysChanged)
                {
                    p.thread.mediaReferenceKey = p.mutex.mediaReferenceKey;
                    p.thread.clipMediaReferenceKeys = p.mutex.clipMediaReferenceKeys;
                    p.mutex.mediaReferenceKeysChanged = false;
                }
            }

            // Traverse the timeline for new video requests.
            for (auto& request : newVideoRequests)
            {
                try
                {
                    for (const auto& otioTrack :
                             p.thread.otioTimeline->video_tracks())
                    {
                        if (!otioTrack->enabled())
                            continue;
                        for (const auto& otioChild : otioTrack->children())
                        {
                            if (auto otioItem =
                                dynamic_cast<OTIO_NS::Item*>(otioChild.value))
                            {
                                const auto requestTime =
                                    request->time - p.timeRange.start_time();
                                OTIO_NS::ErrorStatus errorStatus;
                                const auto range =
                                    otioItem->trimmed_range_in_parent(
                                        &errorStatus);
                                if (range.has_value() &&
                                    range.value().contains(requestTime))
                                {
                                    Private::VideoLayerData videoLayerData;
                                    if (auto otioClip =
                                        dynamic_cast<const OTIO_NS::Clip*>(
                                            otioItem))
                                    {
                                        videoLayerData.image = _readVideo(
                                            otioClip, requestTime,
                                            request->options);
                                        videoLayerData.bounds = getCanvasBox(
                                            getMediaReferenceBounds(p.mediaReference(otioClip)),
                                            p.options.spatial,
                                            p.normalizeSize,
                                            p.boundsScale,
                                            p.canvasOffset);
                                    }
                                    const auto neighbors =
                                        otioTrack->neighbors_of(
                                            otioItem, &errorStatus);
                                    if (auto otioTransition =
                                        dynamic_cast<OTIO_NS::Transition*>(
                                            neighbors.second.value))
                                    {
                                        if (requestTime >
                                            range.value().end_time_inclusive() -
                                            otioTransition->in_offset())
                                        {
                                            videoLayerData.transition = toTransition(
                                                otioTransition
                                                ->transition_type());
                                            videoLayerData.transitionValue =
                                                _transitionValue(
                                                    requestTime.value(),
                                                    range.value()
                                                    .end_time_inclusive()
                                                    .value() -
                                                    otioTransition
                                                    ->in_offset()
                                                    .value(),
                                                    range.value()
                                                    .end_time_inclusive()
                                                    .value() +
                                                    otioTransition
                                                    ->out_offset()
                                                    .value() +
                                                    1.0);
                                            const auto transitionNeighbors =
                                                otioTrack->neighbors_of(
                                                    otioTransition,
                                                    &errorStatus);
                                            if (const auto otioClipB =
                                                dynamic_cast<OTIO_NS::Clip*>(
                                                    transitionNeighbors
                                                    .second.value))
                                            {
                                                videoLayerData.imageB = _readVideo(
                                                    otioClipB, requestTime,
                                                    request->options);
                                                videoLayerData.boundsB = getCanvasBox(
                                                    getMediaReferenceBounds(p.mediaReference(otioClipB)),
                                                    p.options.spatial,
                                                    p.normalizeSize,
                                                    p.boundsScale,
                                                    p.canvasOffset);
                                            }
                                        }
                                    }
                                    if (auto otioTransition =
                                        dynamic_cast<OTIO_NS::Transition*>(
                                            neighbors.first.value))
                                    {
                                        if (requestTime <
                                            range.value().start_time() +
                                            otioTransition->out_offset())
                                        {
                                            std::swap(
                                                videoLayerData.image,
                                                videoLayerData.imageB);
                                            videoLayerData.transition = toTransition(
                                                otioTransition
                                                ->transition_type());
                                            videoLayerData.transitionValue =
                                                _transitionValue(
                                                    requestTime.value(),
                                                    range.value()
                                                    .start_time()
                                                    .value() -
                                                    otioTransition
                                                    ->in_offset()
                                                    .value() -
                                                    1.0,
                                                    range.value()
                                                    .start_time()
                                                    .value() +
                                                    otioTransition
                                                    ->out_offset()
                                                    .value());
                                            const auto transitionNeighbors =
                                                otioTrack->neighbors_of(
                                                    otioTransition,
                                                    &errorStatus);
                                            if (const auto otioClipB =
                                                dynamic_cast<OTIO_NS::Clip*>(
                                                    transitionNeighbors
                                                    .first.value))
                                            {
                                                videoLayerData.image = _readVideo(
                                                    otioClipB, requestTime,
                                                    request->options);
                                                videoLayerData.bounds = getCanvasBox(
                                                    getMediaReferenceBounds(p.mediaReference(otioClipB)),
                                                    p.options.spatial,
                                                    p.normalizeSize,
                                                    p.boundsScale,
                                                    p.canvasOffset);
                                            }
                                        }
                                    }
                                    request->layerData.push_back(
                                        std::move(videoLayerData));
                                }
                            }
                        }
                    }
                }
                catch (const std::exception&)
                {
                    //! \todo How should this be handled?
                }

                p.thread.videoRequestsInProgress.push_back(request);
            }

            // Traverse the timeline for new audio requests.
            for (auto& request : newAudioRequests)
            {
                try
                {
                    for (const auto& otioTrack :
                             p.thread.otioTimeline->audio_tracks())
                    {
                        for (const auto& otioChild : otioTrack->children())
                        {
                            if (auto otioItem =
                                dynamic_cast<OTIO_NS::Item*>(otioChild.value))
                            {
                                const auto rangeOptional =
                                    otioItem->trimmed_range_in_parent();
                                if (rangeOptional.has_value())
                                {
                                    const OTIO_NS::TimeRange clipTimeRange(
                                        rangeOptional.value()
                                        .start_time()
                                        .rescaled_to(1.0),
                                        rangeOptional.value()
                                        .duration()
                                        .rescaled_to(1.0));
                                    const double start = request->seconds -
                                                         p.timeRange.start_time()
                                                         .rescaled_to(1.0)
                                                         .value();
                                    const OTIO_NS::TimeRange requestTimeRange =
                                        OTIO_NS::TimeRange(
                                            OTIO_NS::RationalTime(start, 1.0),
                                            OTIO_NS::RationalTime(1.0, 1.0));
                                    OTIO_NS::TimeRange transitionRange =
                                        clipTimeRange;

                                    OTIO_NS::ErrorStatus errorStatus;
                                    const auto neighbors =
                                        otioTrack->neighbors_of(
                                            otioItem, &errorStatus);
                                    if (auto otioTransition =
                                        dynamic_cast<OTIO_NS::Transition*>(
                                            neighbors.first.value))
                                    {
                                        const auto inOffset =
                                            otioTransition->in_offset()
                                            .rescaled_to(1.0);
                                        transitionRange = OTIO_NS::TimeRange(
                                            transitionRange.start_time() -
                                            inOffset,
                                            transitionRange.duration() +
                                            inOffset);
                                    }

                                    if (auto otioTransition =
                                        dynamic_cast<OTIO_NS::Transition*>(
                                            neighbors.second.value))
                                    {
                                        const auto outOffset =
                                            otioTransition->out_offset()
                                            .rescaled_to(1.0);
                                        transitionRange = OTIO_NS::TimeRange(
                                            transitionRange.start_time(),
                                            transitionRange.duration() +
                                            outOffset);
                                    }

                                    if (requestTimeRange.intersects(
                                            transitionRange))
                                    {
                                        Private::AudioLayerData audioData;
                                        audioData.seconds = request->seconds;
                                        //! \bug Why is
                                        //! OTIO_NS::TimeRange::clamped() not
                                        //! giving us the result we expect?
                                        // audioData.timeRange =
                                        // requestTimeRange.clamped(clipTimeRange);
                                        const double start = std::max(
                                            transitionRange.start_time()
                                            .value(),
                                            requestTimeRange.start_time()
                                            .value());
                                        const double end = std::min(
                                            transitionRange.start_time()
                                            .value() +
                                            transitionRange.duration()
                                            .value(),
                                            requestTimeRange.start_time()
                                            .value() +
                                            requestTimeRange.duration()
                                            .value());
                                        audioData.timeRange = OTIO_NS::TimeRange(
                                            OTIO_NS::RationalTime(start, 1.0),
                                            OTIO_NS::RationalTime(
                                                end - start, 1.0));

                                        if (auto otioClip =
                                            dynamic_cast<OTIO_NS::Clip*>(
                                                otioItem))
                                        {
                                            audioData.audio = _readAudio(
                                                otioClip, audioData.timeRange,
                                                request->options);
                                        }

                                        if (auto otioTransition =
                                            dynamic_cast<OTIO_NS::Transition*>(
                                                neighbors.second.value))
                                        {
                                            const auto pad =
                                                OTIO_NS::RationalTime(1.0, 1.0);
                                            const auto inOffset =
                                                otioTransition->in_offset()
                                                .rescaled_to(1.0);
                                            const auto outOffset =
                                                otioTransition->out_offset()
                                                .rescaled_to(1.0);
                                            auto transitionRange =
                                                OTIO_NS::TimeRange(
                                                    clipTimeRange
                                                    .end_time_inclusive() -
                                                    inOffset,
                                                    inOffset + outOffset + pad);
                                            if (audioData.timeRange.intersects(
                                                    transitionRange))
                                            {
                                                audioData.clipTimeRange =
                                                    clipTimeRange;
                                                audioData.outTransition =
                                                    otioTransition;
                                            }
                                        }

                                        if (auto otioTransition =
                                            dynamic_cast<OTIO_NS::Transition*>(
                                                neighbors.first.value))
                                        {
                                            const auto outOffset =
                                                otioTransition->out_offset()
                                                .rescaled_to(1.0);
                                            const auto inOffset =
                                                otioTransition->in_offset()
                                                .rescaled_to(1.0);
                                            auto transitionRange =
                                                OTIO_NS::TimeRange(
                                                    clipTimeRange.start_time() -
                                                    inOffset,
                                                    outOffset + inOffset);
                                            if (audioData.timeRange.intersects(
                                                    transitionRange))
                                            {
                                                audioData.clipTimeRange =
                                                    clipTimeRange;
                                                audioData.inTransition =
                                                    otioTransition;
                                            }
                                        }
                                        request->layerData.push_back(
                                            std::move(audioData));
                                    }
                                }
                            }
                        }
                    }
                }
                catch (const std::exception&)
                {
                    //! \todo How should this be handled?
                }

                p.thread.audioRequestsInProgress.push_back(request);
            }

            // Check for finished video requests.
            auto videoRequestIt = p.thread.videoRequestsInProgress.begin();
            while (videoRequestIt != p.thread.videoRequestsInProgress.end())
            {
                bool valid = true;
                for (auto& i : (*videoRequestIt)->layerData)
                {
                    if (i.image.valid())
                    {
                        valid &= i.image.wait_for(std::chrono::seconds(0)) ==
                                 std::future_status::ready;
                    }
                    if (i.imageB.valid())
                    {
                        valid &= i.imageB.wait_for(std::chrono::seconds(0)) ==
                                 std::future_status::ready;
                    }
                }
                if (valid)
                {
                    const auto frame = p.videoFrame(**videoRequestIt);
                    (*videoRequestIt)->promise.set_value(frame);
                    videoRequestIt = p.thread.videoRequestsInProgress.erase(videoRequestIt);
                    continue;
                }
                ++videoRequestIt;
            }

            // Check for finished audio requests.
            auto audioRequestIt = p.thread.audioRequestsInProgress.begin();
            while (audioRequestIt != p.thread.audioRequestsInProgress.end())
            {
                bool valid = true;
                for (auto& i : (*audioRequestIt)->layerData)
                {
                    if (i.audio.valid())
                    {
                        valid &= i.audio.wait_for(std::chrono::seconds(0)) ==
                                 std::future_status::ready;
                    }
                }
                if (valid)
                {
                    const auto frame = p.audioFrame(**audioRequestIt);
                    (*audioRequestIt)->promise.set_value(frame);
                    audioRequestIt =
                        p.thread.audioRequestsInProgress.erase(audioRequestIt);
                    continue;
                }
                ++audioRequestIt;
            }
        }

        std::future<io::VideoData> Timeline::_readVideo(
            const OTIO_NS::Clip* clip, const OTIO_NS::RationalTime& time,
            const io::Options& options)
        {
            TLRENDER_P();

            std::future<io::VideoData> out;
            io::Options optionsMerged = io::merge(options, p.options.ioOptions);
            optionsMerged["USD/cameraName"] = clip->name();

            const auto mediaReference = p.mediaReference(clip);
            // A sequence is decoded on the timeline's pool; anything that has to
            // be read statefully keeps its own reader.
            auto seq = _getSeqDecode(mediaReference, optionsMerged);
            auto read = seq ? nullptr : _getVideoRead(mediaReference, optionsMerged);
            const auto timeRangeOpt = p.getTrimmedRangeInParent(clip);
            if ((seq || read) && timeRangeOpt.has_value())
            {
                const io::Info& ioInfo = seq ? seq->getInfo() : read->getInfo().get();
                if (!ioInfo.videoTime.has_value())
                {
                    // No video in the media, so there is no frame to read and no
                    // rate to convert the time with.
                    return out;
                }
                OTIO_NS::TimeRange availableRange = clip->available_range();
                OTIO_NS::TimeRange trimmedRange = clip->trimmed_range();
                if (p.options.compat &&
                    availableRange.start_time() > ioInfo.videoTime->start_time())
                {
                    //! \bug If the available range is greater than the media
                    //! time, assume the media time is wrong and compensate
                    //! for it.
                    trimmedRange = OTIO_NS::TimeRange(
                        trimmedRange.start_time() - availableRange.start_time(),
                        trimmedRange.duration());
                }
                const auto mediaTime = toVideoMediaTime(
                    time,
                    timeRangeOpt.value(),
                    trimmedRange,
                    ioInfo.videoTime->duration().rate());
                out = seq ?
                      p.submitRead(
                          [seq, mediaTime, optionsMerged]
                              {
                                  return seq->readVideo(mediaTime, optionsMerged);
                              }) :
                      read->readVideo(mediaTime, optionsMerged);
            }
            return out;
        }

        std::future<io::AudioData> Timeline::_readAudio(
            const OTIO_NS::Clip* clip, const OTIO_NS::TimeRange& timeRange,
            const io::Options& options)
        {
            TLRENDER_P();

            std::future<io::AudioData> out;
            io::Options optionsMerged =
                io::merge(options, p.options.ioOptions);
            auto read = _getAudioRead(clip, optionsMerged);
            const auto timeRangeOpt = clip->trimmed_range_in_parent();
            if (read && timeRangeOpt.has_value())
            {
                const io::Info& ioInfo = read->getInfo().get();
                if (ioInfo.audioTime.has_value())
                {
                    OTIO_NS::TimeRange trimmedRange = clip->trimmed_range();
                    if (p.options.compat &&
                        trimmedRange.start_time() < ioInfo.audioTime->start_time())
                    {
                        //! \bug If the trimmed range is less than the media time,
                        //! assume the media time is wrong (e.g., ALab trailer) and
                        //! compensate for it.
                        trimmedRange = OTIO_NS::TimeRange(
                            ioInfo.audioTime->start_time() + trimmedRange.start_time(),
                            trimmedRange.duration());
                    }
                    const auto mediaRange = timeline::toAudioMediaTime(
                        timeRange, timeRangeOpt.value(), trimmedRange,
                        ioInfo.audio.sampleRate);
                    out = read->readAudio(mediaRange, optionsMerged);
                }
            }
            return out;
        }

        bool Timeline::_getVideoInfo(const OTIO_NS::Composable* composable)
        {
            TLRENDER_P();
            if (auto clip = dynamic_cast<const OTIO_NS::Clip*>(composable))
            {
                if (auto context = p.context.lock())
                {
                    // The first video clip defines the video information for the timeline.
                    io::Info ioInfo;
                    if (_getVideoIOInfo(
                            p.mediaReference(clip), p.options.ioOptions,
                            ioInfo))
                    {
                        p.ioInfo.video = ioInfo.video;
                        p.ioInfo.videoTime = ioInfo.videoTime;
                        p.ioInfo.tags.insert(ioInfo.tags.begin(), ioInfo.tags.end());

                        // Find the largest resolution among the clip's media
                        // references, so that the canvas can hold the highest
                        // resolution one rather than only the reference that is
                        // active now. The readers opened here stay in the read
                        // cache, which also makes the first switch faster.
                        //
                        // The information reported by getIOInfo() is left as that
                        // of the active reference, since that is the media being
                        // played.
                        p.maxVideoSize = math::Size2i();
                        if (!p.ioInfo.video.empty())
                        {
                            p.maxVideoSize.w = p.ioInfo.video[0].size.w;
                            p.maxVideoSize.h = p.ioInfo.video[0].size.h;
                        }
                        p.videoInfoClip = clip;
                        for (const auto& i : clip->media_references())
                        {
                            io::Info mediaReferenceInfo;
                            if (_getVideoIOInfo(
                                    i.second, p.options.ioOptions, mediaReferenceInfo))
                            {
                                // Kept so that getIOInfo() can report the media
                                // that is actually being read; completed with
                                // the timeline level information once it is
                                // known.
                                p.videoInfoByReference[i.second] = mediaReferenceInfo;
                                if (!mediaReferenceInfo.video.empty())
                                {
                                    const math::Size2i size(
                                        mediaReferenceInfo.video[0].size.w,
                                        mediaReferenceInfo.video[0].size.h);
                                    if (size.w * size.h >
                                        p.maxVideoSize.w * p.maxVideoSize.h)
                                    {
                                        p.maxVideoSize = size;
                                    }
                                }
                            }
                        }
                        return true;
                    }
                }
            }
            if (auto composition = dynamic_cast<const OTIO_NS::Composition*>(composable))
            {
                for (const auto& child : composition->children())
                {
                    if (_getVideoInfo(child))
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        void Timeline::_finishRequests()
        {
            TLRENDER_P();

            {
                std::list<std::shared_ptr<Private::PendingVideoRequest> >
                    videoRequests;
                std::list<std::shared_ptr<Private::PendingAudioRequest> >
                    audioRequests;
                {
                    std::unique_lock<std::mutex> lock(p.mutex.mutex);
                    p.mutex.stopped = true;
                    videoRequests = std::move(p.mutex.videoRequests);
                    audioRequests = std::move(p.mutex.audioRequests);
                }
                videoRequests.insert(
                    videoRequests.begin(),
                    p.thread.videoRequestsInProgress.begin(),
                    p.thread.videoRequestsInProgress.end());
                p.thread.videoRequestsInProgress.clear();
                audioRequests.insert(
                    audioRequests.begin(),
                    p.thread.audioRequestsInProgress.begin(),
                    p.thread.audioRequestsInProgress.end());
                p.thread.audioRequestsInProgress.clear();
                for (auto& request : videoRequests)
                {
                    const auto frame = p.videoFrame(*request);
                    request->promise.set_value(frame);
                }
                for (auto& request : audioRequests)
                {
                    const auto frame = p.audioFrame(*request);
                    request->promise.set_value(frame);
                }
            }
        }

        void Timeline::Private::startReadPool(size_t threadCount)
        {
            readPool.stopped = false;
            for (size_t i = 0; i < std::max(threadCount, size_t(1)); ++i)
            {
                readPool.threads.push_back(
                    std::thread(
                        [this]
                            {
                                while (true)
                                {
                                    ReadPool::Task task;
                                    {
                                        std::unique_lock<std::mutex> lock(readPool.mutex);
                                        readPool.cv.wait(
                                            lock,
                                            [this]
                                                {
                                                    return readPool.stopped || !readPool.tasks.empty();
                                                });
                                        if (readPool.tasks.empty())
                                        {
                                            // Stopped and drained.
                                            return;
                                        }
                                        task = std::move(readPool.tasks.front());
                                        readPool.tasks.pop_front();
                                    }
                                    try
                                    {
                                        task.promise.set_value(task.f());
                                    }
                                    catch (const std::exception&)
                                    {
                                        // Passed on rather than delivered empty: the
                                        // frame still comes out blank, since videoFrame()
                                        // catches this and carries on, but it is counted
                                        // and logged instead of going by in silence.
                                        task.promise.set_exception(std::current_exception());
                                    }
                                }
                            }));
            }
        }

        void Timeline::Private::stopReadPool()
        {
            std::list<ReadPool::Task> dropped;
            {
                std::unique_lock<std::mutex> lock(readPool.mutex);
                readPool.stopped = true;
                // Whatever has not started decoding is not going to be looked at,
                // so give the frames back empty rather than making the close wait
                // for a queue of them.
                dropped = std::move(readPool.tasks);
                readPool.tasks.clear();
            }
            for (auto& task : dropped)
            {
                task.promise.set_value(io::VideoData());
            }
            readPool.cv.notify_all();
            for (auto& thread : readPool.threads)
            {
                if (thread.joinable())
                {
                    thread.join();
                }
            }
            readPool.threads.clear();
        }

        std::future<io::VideoData> Timeline::Private::submitRead(
            std::function<io::VideoData()> f)
        {
            ReadPool::Task task;
            task.f = std::move(f);
            auto out = task.promise.get_future();
            if (readPool.threads.empty())
            {
                // No pool: the caller is the worker.
                try
                {
                    task.promise.set_value(task.f());
                }
                catch (const std::exception&)
                {
                    task.promise.set_exception(std::current_exception());
                }
                return out;
            }
            bool queued = false;
            {
                std::unique_lock<std::mutex> lock(readPool.mutex);
                if (!readPool.stopped)
                {
                    readPool.tasks.push_back(std::move(task));
                    queued = true;
                }
            }
            if (queued)
            {
                readPool.cv.notify_one();
            }
            else
            {
                task.promise.set_value(io::VideoData());
            }
            return out;
        }

        bool Timeline::_getIOInfo(
            const OTIO_NS::MediaReference* mediaReference,
            const io::Options& ioOptions,
            io::Info& out)
        {
            if (auto seq = _getSeqDecode(mediaReference, ioOptions))
            {
                out = seq->getInfo();
                return true;
            }
            // Both requests go out before either is waited on, so that the two
            // readers open the file at the same time.
            auto videoRead = _getVideoRead(mediaReference, ioOptions);
            auto audioRead = _getAudioRead(mediaReference, ioOptions);
            std::future<io::Info> videoFuture;
            std::future<io::Info> audioFuture;
            if (videoRead)
            {
                videoFuture = videoRead->getInfo();
            }
            if (audioRead)
            {
                audioFuture = audioRead->getInfo();
            }
            if (!videoFuture.valid() && !audioFuture.valid())
            {
                return false;
            }
            io::Info videoInfo;
            if (videoFuture.valid())
            {
                videoInfo = videoFuture.get();
            }
            io::Info audioInfo;
            if (audioFuture.valid())
            {
                audioInfo = audioFuture.get();
            }
            out = merge(videoInfo, audioInfo);
            return true;
        }

        std::shared_ptr<io::SeqDecode> Timeline::_getSeqDecode(
            const OTIO_NS::MediaReference* mediaReference,
            const io::Options& ioOptions)
        {
            TLRENDER_P();
            return p.getCached<io::SeqDecode>(
                p.seqCache,
                mediaReference,
                ioOptions,
                [](const std::shared_ptr<system::Context>& context,
                   const file::Path& path,
                   const std::vector<file::MemoryRead>& mem,
                   const io::Options& options)
                    {
                        std::shared_ptr<io::SeqDecode> out;
                        const auto readSystem = context->getSystem<io::ReadSystem>();
                        if (const auto plugin = readSystem->getPlugin(path))
                        {
                            // Null for a format that has to be read statefully,
                            // which leaves the caller to fall back to a reader.
                            if (const auto decode = plugin->decode(options))
                            {
                                out = io::SeqDecode::create(path, mem, decode, options);
                            }
                        }
                        return out;
                    });
        }

        std::shared_ptr<io::IVideoRead> Timeline::_getVideoRead(
            const OTIO_NS::Clip* clip,
            const io::Options& ioOptions)
        {
            TLRENDER_P();
            return _getVideoRead(p.mediaReference(clip), ioOptions);
        }

        std::shared_ptr<io::IAudioRead> Timeline::_getAudioRead(
            const OTIO_NS::Clip* clip,
            const io::Options& ioOptions)
        {
            TLRENDER_P();
            return _getAudioRead(p.mediaReference(clip), ioOptions);
        }

        std::shared_ptr<io::IAudioRead> Timeline::_getAudioRead(
            const OTIO_NS::MediaReference* mediaReference,
            const io::Options& ioOptions)
        {
            TLRENDER_P();
            const file::Path mediaPath = timeline::getPath(
                mediaReference, p.path.getDirectory(), p.options.pathOptions);
            // auto frameCache = p.getFrameCache(mediaPath);
            auto frameCache = p.frameCache;
            return p.getCached<io::IAudioRead>(
                p.audioReadCache,
                mediaReference,
                ioOptions,
                [frameCache](const std::shared_ptr<system::Context>& context,
                   const file::Path& path,
                   const std::vector<file::MemoryRead>& mem,
                   const io::Options& options)
                    {
                        auto read = context->getSystem<io::ReadSystem>()->audioRead(
                            path, mem, options);

                        if (read)
                            read->setCache(frameCache);  // no-op for non-FFmpeg readers
                        return read;
                    });
        }

        bool Timeline::_getVideoIOInfo(
            const OTIO_NS::MediaReference* mediaReference,
            const io::Options& ioOptions,
            io::Info& out)
        {
            if (auto seq = _getSeqDecode(mediaReference, ioOptions))
            {
                out = seq->getInfo();
                return true;
            }
            if (auto videoRead = _getVideoRead(mediaReference, ioOptions))
            {
                out = videoRead->getInfo().get();
                return true;
            }
            return false;
        }

        bool Timeline::_getAudioIOInfo(
            const OTIO_NS::MediaReference* mediaReference,
            const io::Options& ioOptions,
            io::Info& out)
        {
            // Audio is never a sequence of stateless files.
            if (auto audioRead = _getAudioRead(mediaReference, ioOptions))
            {
                out = audioRead->getInfo().get();
                return true;
            }
            return false;
        }

        bool Timeline::_getAudioInfo(const OTIO_NS::Composable* composable)
        {
            TLRENDER_P();
            if (auto clip = dynamic_cast<const OTIO_NS::Clip*>(composable))
            {
                if (auto context = p.context.lock())
                {
                    // The first audio clip defines the audio information for
                    // the timeline.
                    if (auto read = _getAudioRead(clip, p.options.ioOptions))
                    {
                        const io::Info& ioInfo = read->getInfo().get();
                        p.ioInfo.audio = ioInfo.audio;
                        p.ioInfo.audioTime = ioInfo.audioTime;
                        p.ioInfo.tags.insert(ioInfo.tags.begin(),
                                             ioInfo.tags.end());
                        return true;
                    }
                }
            }
            if (auto composition = dynamic_cast<const OTIO_NS::Composition*>(composable))
            {
                for (const auto& child : composition->children())
                {
                    if (_getAudioInfo(child))
                    {
                        return true;
                    }
                }
            }
            return false;
        }

        void Timeline::_getCanvas()
        {
            TLRENDER_P();
            // The OTIO spatial coordinates describe a single canvas shared by
            // the whole timeline, so the extent is taken from every clip
            // rather than from the clips visible at one time. This keeps the
            // render size stable as playback moves between clips.
            p.normalizeSize = p.maxVideoSize;
            const math::Size2i& normalizeSize = p.normalizeSize;
            const auto otioClips = p.otioTimeline.value->find_children<OTIO_NS::Clip>();

            // The coordinates are unit-less, so a reference is needed to map
            // them onto a pixel size. Take it from the first clip that has
            // coordinates, which is not necessarily the first clip in the
            // timeline, together with the resolution the timeline is
            // working at.
            //
            // This uses the active media reference rather than the union of
            // all of them, unlike the canvas below. The coordinates of a clip's
            // references describe the same area, so any of them gives the same
            // scale; taking the union here would only matter for a clip whose
            // references were authored inconsistently, where the active one is
            // the better guide.
            if (normalizeSize.isValid())
            {
                for (const auto& otioClip : otioClips)
                {
                    if (const auto bounds = getClipBounds(otioClip))
                    {
                        const float w = bounds.value().getSize().w;
                        if (w > 0.F)
                        {
                            p.boundsScale = normalizeSize.w / w;
                            break;
                        }
                    }
                }
            }

            std::optional<math::Box2f> canvas;
            for (const auto& otioClip : otioClips)
            {
                // Report the coordinates as they were authored, so the numbers
                // in the file can be seen alongside the canvas derived from
                // them.
                if (const auto authored = getClipBounds(otioClip))
                {
                    p.ioInfo.tags[string::Format("OTIO Image Bounds {0}").
                                  arg(otioClip->name())] =
                        string::Format("{0}, {1}, {2}, {3}").
                        arg(authored.value().min.x).
                        arg(authored.value().min.y).
                        arg(authored.value().max.x).
                        arg(authored.value().max.y);
                }
                // Cover every media reference, not just the active one, so that
                // changing the active media reference cannot place a clip
                // outside the canvas.
                if (const auto bounds = getSpatialBounds(
                        getClipBoundsUnion(otioClip),
                        p.options.spatial,
                        normalizeSize,
                        p.boundsScale))
                {
                    canvas = canvas.has_value() ?
                             math::expand(canvas.value(), bounds.value()) :
                             bounds.value();
                }
            }
            if (canvas.has_value())
            {
                const math::Size2f size = canvas.value().getSize();
                if (size.w > 0.F && size.h > 0.F)
                {
                    p.canvasOffset = -canvas.value().min;
                    p.canvasSize = math::Size2i(
                        static_cast<int>(std::round(size.w)),
                        static_cast<int>(std::round(size.h)));
                    p.ioInfo.tags["OTIO Canvas"] =
                        string::Format("{0}").arg(p.canvasSize);
                    p.ioInfo.tags["OTIO Pixels Per Unit"] =
                        string::Format("{0}").arg(p.boundsScale);
                }
            }
            else if (p.maxVideoSize.isValid())
            {
                // No clip in this timeline carries OTIO spatial coordinates,
                // so there is no authored canvas to derive from. Fall back to
                // the largest native resolution found across every clip in the
                // timeline (see _getMaxVideoSize()), so that
                // VideoFrame::canvasSize is still valid
                // and getInfos() (CompareOptions.cpp) substitutes it for the
                // raw per-frame image size on every frame -- giving
                // getRenderSize() one stable value for the whole timeline
                // instead of tracking whichever clip happens to be playing.
                p.canvasOffset = math::Vector2f();
                p.canvasSize = p.maxVideoSize;
            }
        }

        void Timeline::_getMaxVideoSize()
        {
            TLRENDER_P();
            // Scan every clip in the timeline (not just the first, and not just
            // its alternate media references) so that transitions between clips
            // of different native resolution/aspect ratio are measured against
            // the true timeline-wide maximum, not whichever clip happened to be
            // first. Readers opened here stay in the read cache.
            for (const auto& otioClip :
                     p.otioTimeline.value->find_children<OTIO_NS::Clip>())
            {
                for (const auto& i : otioClip->media_references())
                {
                    if (auto read = _getVideoRead(i.second, p.options.ioOptions))
                    {
                        const io::Info& info = read->getInfo().get();
                        if (!info.video.empty())
                        {
                            const math::Size2i size(
                                info.video[0].size.w,
                                info.video[0].size.h);
                            if (size.w * size.h >
                                p.maxVideoSize.w * p.maxVideoSize.h)
                            {
                                p.maxVideoSize = size;
                            }
                        }
                    }
                }
            }
        }

        void Timeline::Private::indexTimeline()
        {
            // Built into fresh maps and swapped in at the end, rather than
            // cleared and refilled in place: these key on raw pointers into
            // otioTimeline, and setTimeline() can replace it with an
            // unrelated tree, which would otherwise leave entries pointing
            // at composables and media references that have already been
            // freed. Building into fresh maps first also means a
            // readMedia()/readMediaAudio() call on another thread never
            // finds mediaByPath/mediaByNormalPath briefly empty mid-rebuild;
            // it sees either the old timeline's entries or the new one's.
            std::map<const OTIO_NS::Composable*, OTIO_NS::TimeRange>
                newTrimmedRangeInParent;
            std::map<const OTIO_NS::Track*, std::vector<TrackItem> >
                newTrackItems;
            std::map<std::string, OTIO_NS::MediaReference*> newMediaByPath;
            std::map<std::string, OTIO_NS::MediaReference*> newMediaByNormalPath;
            if (otioTimeline.value)
            {
                for (const auto& otioTrack :
                         otioTimeline.value->find_children<OTIO_NS::Track>())
                {
                    OTIO_NS::ErrorStatus errorStatus;
                    const auto ranges =
                        otioTrack->range_of_all_children(&errorStatus);
                    if (OTIO_NS::is_error(errorStatus))
                    {
                        continue;
                    }
                    auto& items = newTrackItems[otioTrack];
                    for (const auto& i : ranges)
                    {
                        if (const auto trimmed =
                                otioTrack->trim_child_range(i.second))
                        {
                            newTrimmedRangeInParent[i.first] = trimmed.value();
                            if (auto otioItem =
                                    dynamic_cast<OTIO_NS::Item*>(i.first))
                            {
                                items.push_back({ otioItem, trimmed.value() });
                            }
                        }
                    }
                    std::sort(
                        items.begin(),
                        items.end(),
                        [](const TrackItem& a, const TrackItem& b)
                            {
                                return a.range.start_time() <
                                       b.range.start_time();
                            });
                }
                for (const auto& otioClip :
                         otioTimeline.value->find_children<OTIO_NS::Clip>())
                {
                    for (const auto& i : otioClip->media_references())
                    {
                        if (i.second)
                        {
                            const file::Path mediaPath = timeline::getPath(
                                i.second,
                                path.getDirectory(),
                                options.pathOptions);
                            newMediaByPath[mediaPath.get()] = i.second;
                            newMediaByNormalPath[normalMediaPath(mediaPath)] =
                                i.second;
                        }
                    }
                }
            }
            // trimmedRangeInParent/trackItems are only read without locking
            // by the request thread, which indexTimeline()'s caller has
            // already stopped, so no lock is needed here to replace them.
            trimmedRangeInParent = std::move(newTrimmedRangeInParent);
            trackItems = std::move(newTrackItems);
            {
                std::unique_lock<std::mutex> lock(readCacheMutex);
                mediaByPath = std::move(newMediaByPath);
                mediaByNormalPath = std::move(newMediaByNormalPath);
            }
        }

        void Timeline::_timelineUpdate()
        {
            TLRENDER_P();

            p.timeRange = timeline::getTimeRange(p.otioTimeline.value);
            // The old videoInfoClip/videoInfoByReference pointers belonged
            // to the tree we just released above and are now dangling --
            // they must be rebuilt against the new tree, not reused.
            p.videoInfoClip = nullptr;
            p.videoInfoByReference.clear();
            {
                // Guarded because readMedia()/readMediaAudio() can be
                // reading these caches from another thread regardless of
                // whether the request thread is running; see readCacheMutex.
                std::unique_lock<std::mutex> lock(p.readCacheMutex);
                p.videoReadCache.clear();
                p.audioReadCache.clear();
                p.seqCache.clear();
            }
            p.maxVideoSize = math::Size2i();
            p.canvasSize = math::Size2i();
            p.canvasOffset = math::Vector2f();
            p.normalizeSize = math::Size2i();
            p.boundsScale = 1.0;

            bool videoFound = false;
            bool audioFound = false;

            for (const auto& i : p.otioTimeline.value->tracks()->children())
            {
                if (auto otioTrack = dynamic_cast<const OTIO_NS::Track*>(i.value))
                {
                    if (!videoFound && OTIO_NS::Track::Kind::video == otioTrack->kind())
                    {
                        videoFound = _getVideoInfo(otioTrack);
                    }
                    else if (!audioFound && OTIO_NS::Track::Kind::audio == otioTrack->kind())
                    {
                        audioFound = _getAudioInfo(otioTrack);
                    }

                    // Break early if we've successfully found both
                    if (videoFound && audioFound)
                    {
                        break;
                    }
                }
            }
            _getMaxVideoSize();
            _getCanvas();

            for (auto& i : p.videoInfoByReference)
            {
                io::Info ioInfo = p.ioInfo;
                ioInfo.video = i.second.video;
                ioInfo.videoTime = i.second.videoTime;
                for (const auto& tag : i.second.tags)
                {
                    ioInfo.tags[tag.first] = tag.second;
                }
                i.second = ioInfo;
            }
        }

        size_t Timeline::getVideoRequestMax() const
        {
            // At least one, whatever the options say. Zero here would not mean
            // "no limit", it would mean no request is ever picked up, and a
            // timeline without a thread would wait for one that never came.
            return std::max(_p->options.readThreadCount, size_t(1)) * 2;
        }

        size_t Timeline::getReadThreadCount() const
        {
            return _p->readPool.threads.size();
        }

        std::shared_ptr<io::Cache> Timeline::Private::getFrameCache(
            const file::Path& mediaPath)
        {
            const std::string key = getKey(mediaPath);
            std::unique_lock<std::mutex> lock(readCacheMutex);
            std::shared_ptr<io::Cache> out;
            if (!pathFrameCache.get(key, out))
            {
                out = io::Cache::create();
                out->setMax(4 * memory::gigabyte);
                pathFrameCache.add(key, out);
            }
            return out;
        }

        std::shared_ptr<io::IVideoRead> Timeline::_getVideoRead(
            const OTIO_NS::MediaReference* mediaReference,
            const io::Options& ioOptions)
        {
            TLRENDER_P();
            const file::Path mediaPath = timeline::getPath(
                mediaReference, p.path.getDirectory(), p.options.pathOptions);
            auto frameCache = p.frameCache; //p.getFrameCache(mediaPath);
            return p.getCached<io::IVideoRead>(
                p.videoReadCache,
                mediaReference,
                ioOptions,
                [frameCache](const std::shared_ptr<system::Context>& context,
                             const file::Path& path,
                             const std::vector<file::MemoryRead>& mem,
                             const io::Options& options)
                    {
                        auto read = context->getSystem<io::ReadSystem>()->videoRead(
                            path, mem, options);
                        if (read)
                            read->setCache(frameCache);
                        return read;
                    });
        }

        std::vector<file::Path> Timeline::getMediaPaths() const
        {
            TLRENDER_P();
            std::vector<file::Path> out;
            std::unique_lock<std::mutex> lock(p.readCacheMutex);
            for (const auto& i : p.mediaByPath)
            {
                out.push_back(file::Path(i.first));
            }
            return out;
        }

        OTIO_NS::MediaReference* Timeline::_findMedia(const file::Path& path)
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.readCacheMutex);
            const auto i = p.mediaByPath.find(path.get());
            if (i != p.mediaByPath.end())
            {
                return i->second;
            }
            // The media references are resolved when the timeline is read, so they
            // are usually absolute, while a caller asks with the path it was given.
            // Opening a file by a relative path otherwise found none of its own
            // media.
            const auto j = p.mediaByNormalPath.find(normalMediaPath(path));
            return j != p.mediaByNormalPath.end() ? j->second : nullptr;
        }

        bool Timeline::getMediaInfo(
            const file::Path& path,
            io::Info& out,
            const io::Options& options)
        {
            TLRENDER_P();
            if (auto mediaReference = _findMedia(path))
            {
                return _getIOInfo(
                    mediaReference, io::merge(options, p.options.ioOptions), out);
            }
            return false;
        }

        std::optional<Timeline::MediaAt> Timeline::_mediaAt(
            const OTIO_NS::RationalTime& time)
        {
            TLRENDER_P();
            std::optional<MediaAt> out;
            if (!p.otioTimeline)
            {
                return out;
            }
            // The same lookup the request thread makes: the timeline's own start is
            // taken off, and the first enabled video track holding that time wins.
            // Bisected rather than walked, because this is asked for the playhead
            // and for every ruler label that is drawn, and a sequence built out of
            // the runs of frames it has can be in a great many pieces.
            const OTIO_NS::RationalTime trackTime = time - p.timeRange.start_time();
            for (const auto& otioTrack : p.otioTimeline->video_tracks())
            {
                if (!otioTrack->enabled())
                {
                    continue;
                }
                for (const auto& otioChild :
                         p.getTrackChildrenAt(otioTrack, trackTime))
                {
                    auto otioClip = dynamic_cast<const OTIO_NS::Clip*>(otioChild);
                    if (!otioClip)
                    {
                        continue;
                    }
                    const auto rangeInParent = p.getTrimmedRangeInParent(otioClip);
                    if (!rangeInParent.has_value() ||
                        !rangeInParent.value().contains(trackTime))
                    {
                        continue;
                    }

                    out = _mediaFrom(otioClip, rangeInParent.value());
                    if (out)
                    {
                        return out;
                    }
                }
            }
            return out;
        }

        std::optional<Timeline::MediaAt> Timeline::_mediaFrom(
            const OTIO_NS::Clip* otioClip,
            const OTIO_NS::TimeRange& rangeInParent)
        {
            TLRENDER_P();
            std::optional<MediaAt> out;
            const io::Options optionsMerged = p.options.ioOptions;
            auto mediaReference = p.mediaReference(otioClip);
            io::Info ioInfo;
            MediaAt mediaAt;

            mediaAt.seq = _getSeqDecode(mediaReference, optionsMerged);
            if (mediaAt.seq)
            {
                ioInfo = mediaAt.seq->getInfo();
            }
            else if (auto read = _getVideoRead(mediaReference, optionsMerged))
            {
                ioInfo = read->getInfo().get();
            }
            else
            {
                return out;
            }

            if (!ioInfo.videoTime.has_value())
            {
                // No video in the media, so there is no rate to convert times
                // with and nothing to say where the clip sits.
                return out;
            }
            OTIO_NS::TimeRange trimmedRange = otioClip->trimmed_range();
            const OTIO_NS::TimeRange availableRange = otioClip->available_range();
            if (p.options.compat &&
                availableRange.start_time() > ioInfo.videoTime->start_time())
            {
                // The same compensation _readVideo() makes, so that both agree on
                // which media time a timeline time means.
                trimmedRange = OTIO_NS::TimeRange(
                    trimmedRange.start_time() - availableRange.start_time(),
                    trimmedRange.duration());
            }
            mediaAt.rangeInParent = rangeInParent;
            mediaAt.trimmedRange = trimmedRange;
            mediaAt.rate = ioInfo.videoTime->duration().rate();
            out = mediaAt;
            return out;
        }

        size_t Timeline::getObjectCount()
        {
            return objectCount;
        }

        template<typename T>
        std::shared_ptr<T> Timeline::Private::getCached(
            memory::LRUCache<std::string, std::shared_ptr<T> >& cache,
            const OTIO_NS::MediaReference* mediaReference,
            const io::Options& ioOptions,
            const std::function<std::shared_ptr<T>(
                   const std::shared_ptr<system::Context>&,
                   const file::Path&,
                   const std::vector<file::MemoryRead>&,
                   const io::Options&)>& create)
        {
            std::shared_ptr<T> out;
            if (mediaUnavailable(mediaReference))
            {
                // Named by the bundle but not inside it. Reading it from its
                // path would be reading a different file than the bundle
                // describes.
                return out;
            }
            const auto mediaPath = timeline::getPath(
                mediaReference,
                path.getDirectory(),
                options.pathOptions);
            const std::string key = getKey(mediaPath);
            std::unique_lock<std::mutex> lock(readCacheMutex);
            if (!cache.get(key, out))
            {
                auto context = this->context.lock();
                if (!context)
                {
                    return out;
                }
                try
                {
                    const auto mem = getMem(mediaReference);
                    if (mediaUnavailable(mediaReference))
                    {
                        // Resolving its byte ranges said the bundle does not
                        // hold it; reading it from its path is not the same
                        // file.
                        return out;
                    }
                    io::Options readOptions = ioOptions;
                    readOptions["SequenceIO/DefaultSpeed"] =
                        string::Format("{0}").arg(timeRange.duration().rate());
                    if (auto imageSeqReference =
                        dynamic_cast<const OTIO_NS::ImageSequenceReference*>(mediaReference))
                    {
                        // The reference says what to do about frames it does
                        // not have, and it is more specific than the options
                        // the timeline was opened with. A reference this
                        // timeline built for a file opened directly carries
                        // those options already.
                        // readOptions["SequenceIO/MissingFrames"] = to_string(
                        //     fromOTIO(imageSeqReference->missing_frame_policy()));
                    }
                    out = create(context, mediaPath, *mem, readOptions);
                }
                catch (const std::exception& e)
                {
                    if (auto log = context->getLogSystem())
                    {
                        log->print(
                            "tl::Timeline",
                            string::Format("Cannot read \"{0}\": {1}").
                            arg(mediaPath.get()).arg(e.what()),
                            log::Type::Error);
                    }
                    return std::shared_ptr<T>();
                }
                if (out)
                {
                    cache.add(key, out);
                }
            }
            return out;
        }

        bool Timeline::Private::mediaUnavailable(
            const OTIO_NS::MediaReference* mediaReference)
        {
            std::unique_lock<std::mutex> lock(memFilesMutex);
            return unavailableMediaReferences.find(mediaReference) !=
                unavailableMediaReferences.end();
        }

        void Timeline::setCacheOptions(const PlayerCacheOptions& options)
        {
            TLRENDER_P();

            double sum = (options.videoGB + options.audioGB);
            p.frameCache->setMax(sum * memory::gigabyte);
        }

        file::Path Timeline::getMediaPath(OTIO_NS::RationalTime& mediaTime)
        {
            file::Path path = getPath();
            file::Path out = path;
            const OTIO_NS::RationalTime time = mediaTime;

            const auto extension = path.getExtension();
            if (extension == ".otio" || extension == ".otioz")
            {
                if (auto otioTimeline = getTimeline())
                {
                    for (const auto& child : otioTimeline->tracks()->children())
                    {
                        auto track = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Track>(child);
                        if (!track || track->kind() != OTIO_NS::Track::Kind::video)
                            continue;

                        OTIO_NS::ErrorStatus errorStatus;
                        for (const auto& trackChild : track->children())
                        {
                            auto clip = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Clip>(trackChild);
                            if (!clip)
                                continue;

                            const auto range = track->range_of_child(clip, &errorStatus);
                            if (OTIO_NS::is_error(errorStatus))
                                continue;

                            if (range.start_time() <= time && time < range.end_time_exclusive())
                            {
                                if (auto ref = clip->media_reference())
                                {
                                    // The directory the .otio (or any nested .otio, if you ever
                                    // support nested references) lives in — target_url()s inside
                                    // it are relative to this, the same way Timeline::create()
                                    // resolves them internally.
                                    const std::string baseDir = path.getDirectory();

                                    if (auto ext = dynamic_cast<OTIO_NS::ExternalReference*>(ref))
                                    {
                                        out = file::Path(url::decode(ext->target_url()));
                                        if (!out.isAbsolute())
                                            out = file::Path(baseDir + out.get());

                                        io::Info info;

                                        double fileRate = time.rate();  // fallback if getMediaInfo fails
                                        if (getMediaInfo(out, info) &&
                                            info.videoTime.has_value())
                                            fileRate = info.videoTime->duration().rate();

                                        const OTIO_NS::TimeRange trimmedRange = clip->trimmed_range();
                                        const OTIO_NS::RationalTime offset =
                                            (time - range.start_time()).rescaled_to(trimmedRange.start_time().rate());
                                        mediaTime = (trimmedRange.start_time() + offset).rescaled_to(fileRate);
                                    }
                                    else if (auto seq = dynamic_cast<OTIO_NS::ImageSequenceReference*>(ref))
                                    {
                                        std::string dir = seq->target_url_base();
                                        if (!dir.empty() && dir.back() != '/' && dir.back() != '\\')
                                            dir += '/';

                                        // Build a path pointing at the sequence's first frame — same
                                        // shape file::Path expects when you open a sequence file
                                        // directly, which is why plain-sequence files already work.
                                        std::stringstream number;
                                        number << std::setfill('0')
                                               << std::setw(seq->frame_zero_padding())
                                               << seq->start_frame();

                                        std::string url = dir + seq->name_prefix() + number.str() +
                                                          seq->name_suffix();

                                        out = file::Path(url::decode(url));
                                        if (!out.isAbsolute())
                                            out = file::Path(baseDir + out.get());

                                        io::Info info;

                                        double fileRate = time.rate();  // fallback if getMediaInfo fails
                                        if (getMediaInfo(out, info) &&
                                            info.videoTime.has_value())
                                            fileRate = info.videoTime->duration().rate();

                                        const OTIO_NS::TimeRange trimmedRange = clip->trimmed_range();
                                        const OTIO_NS::RationalTime offset =
                                            (time - range.start_time()).rescaled_to(trimmedRange.start_time().rate());
                                        mediaTime = (trimmedRange.start_time() + offset).rescaled_to(fileRate);
                                        // mediaTime = track->transformed_time(time, clip, &errorStatus);
                                    }
                                    else
                                    {
                                        // GeneratorReference, MissingReference, etc. — nothing we can
                                        // resolve to a readable path; leave mediaPath == path so it
                                        // falls into the existing "picture of the timeline" fallback
                                        // rather than silently doing nothing.
                                    }
                                }
                            }
                        }
                        break; // first video track is enough for a thumbnail
                    }
                }
            }
            return out;
        }

        VideoFrame Timeline::Private::videoFrame(PendingVideoRequest& request)
        {
            VideoFrame frame;
            frame.canvasSize = canvasSize;
            frame.size.w = maxVideoSize.w;
            frame.size.h = maxVideoSize.h;
            frame.time = request.time;
            for (auto& i : request.layerData)
            {
                VideoLayer layer;
                try
                {
                    if (i.image.valid())
                    {
                        const io::VideoData data = i.image.get();
                        layer.image = data.image;
                        layer.missing = data.missing;
                        layer.heldFrom = data.heldFrom;
                    }
                    if (i.imageB.valid())
                    {
                        layer.imageB = i.imageB.get().image;
                    }
                }
                catch (const std::exception& e)
                {
                    ++frameErrorCount;
                    if (frameError.empty())
                    {
                        frameError = e.what();
                    }
                    if (auto logSystemLocked = logSystem.lock())
                    {
                        logSystemLocked->print(
                            "tl::Timeline",
                            e.what(),
                            log::Type::Error);
                    }
                }
                layer.bounds = i.bounds;
                layer.boundsB = i.boundsB;
                layer.transition = i.transition;
                layer.transitionValue = i.transitionValue;
                frame.layers.push_back(layer);
            }
            return frame;
        }
        AudioFrame Timeline::Private::audioFrame(PendingAudioRequest& request)
        {
            AudioFrame frame;
            frame.seconds = request.seconds;
            for (auto& i : request.layerData)
            {
                AudioLayer layer;
                try
                {
                    if (i.audio.valid())
                    {
                        const auto audioData = i.audio.get();
                        if (audioData.audio)
                        {
                            layer.audio = padAudioToOneSecond(audioData.audio, i.seconds, i.timeRange);
                        }
                    }
                }
                catch (const std::exception& e)
                {
                    ++frameErrorCount;
                    if (frameError.empty())
                    {
                        frameError = e.what();
                    }
                    if (auto logSystemLocked = logSystem.lock())
                    {
                        logSystemLocked->print(
                            "tl::Timeline",
                            e.what(),
                            log::Type::Error);
                    }
                }
                frame.layers.push_back(layer);
            }
            if (frame.layers.empty())
            {
                auto audio = audio::Audio::create(ioInfo.audio,
                                                  ioInfo.audio.sampleRate);
                audio->zero();
                frame.layers.push_back({ audio });
            }
            return frame;
        }

        std::shared_ptr<audio::Audio> Timeline::Private::padAudioToOneSecond(
            const std::shared_ptr<audio::Audio>& audio,
            double seconds,
            const OTIO_NS::TimeRange& range)
        {
            std::list<std::shared_ptr<audio::Audio> > list;
            const double s = seconds - timeRange.start_time().rescaled_to(1.0).value();
            if (range.start_time().value() > s)
            {
                const OTIO_NS::RationalTime t =
                    range.start_time() - OTIO_NS::RationalTime(s, 1.0);
                const OTIO_NS::RationalTime t2 =
                    t.rescaled_to(audio->getInfo().sampleRate);
                auto silence = audio::Audio::create(audio->getInfo(), t2.value());
                silence->zero();
                list.push_back(silence);
            }
            list.push_back(audio);
            if (range.end_time_exclusive().value() < s + 1.0)
            {
                const OTIO_NS::RationalTime t =
                    OTIO_NS::RationalTime(s + 1.0, 1.0) - range.end_time_exclusive();
                const OTIO_NS::RationalTime t2 =
                    t.rescaled_to(audio->getInfo().sampleRate);
                auto silence = audio::Audio::create(audio->getInfo(), t2.value());
                silence->zero();
                list.push_back(silence);
            }
            size_t sampleCount = getSampleCount(list);
            auto out = audio::Audio::create(audio->getInfo(), sampleCount);
            audio::move(list, out->getData(), sampleCount);
            return out;
        }

        void Timeline::expandOTIOZ(const std::string& mediaPath,
                                   std::function<void(bool& aborted,
                                                      const std::string& title,
                                                      size_t done, size_t total) > progressCb)
        {
            TLRENDER_P();

            if (!p.zipReader)
                return;

            p.zipReader->saveMedia(mediaPath, progressCb);
        }
    } // namespace timeline
} // namespace tl
