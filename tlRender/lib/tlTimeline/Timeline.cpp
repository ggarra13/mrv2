// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/TimelinePrivate.h>

#include <tlTimeline/MemoryReference.h>
#include <tlTimeline/Util.h>
#include <tlTimeline/ZipPrivate.h>

#include <tlIO/System.h>

#include <tlCore/Assert.h>
#include <tlCore/Error.h>
#include <tlCore/File.h>
#include <tlCore/FileInfo.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>
#include <tlCore/URL.h>

#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/serializableCollection.h>


namespace tl
{
    namespace timeline
    {
        namespace
        {
            const size_t readCacheMax = 10;
        }

        TLRENDER_ENUM_IMPL(
            FileSequenceAudio, "None", "BaseName", "FileName", "Directory");
        TLRENDER_ENUM_SERIALIZE_IMPL(FileSequenceAudio);

        TLRENDER_ENUM_IMPL(
            Spatial,
            "None",
            "Coordinates",
            "Normalize");


        namespace
        {
            file::Path getPathToAudio(
                const file::Path& path,
                const FileSequenceAudio& fileSequenceAudio,
                const std::string& fileSequenceAudioFileName,
                const std::string& fileSequenceAudioDirectory,
                const file::PathOptions& pathOptions,
                const std::shared_ptr<system::Context>& context)
            {
                file::Path out;
                auto ioSystem = context->getSystem<io::System>();
                const auto audioExtensions = ioSystem->getExtensions(
                    static_cast<int>(io::FileType::Audio));
                switch (fileSequenceAudio)
                {
                case FileSequenceAudio::BaseName:
                {
                    std::vector<std::string> names;
                    names.push_back(path.getDirectory() + path.getBaseName());
                    std::string tmp = path.getBaseName();
                    if (!tmp.empty() && '.' == tmp[tmp.size() - 1])
                    {
                        tmp.pop_back();
                    }
                    names.push_back(path.getDirectory() + tmp);
                    for (const auto& name : names)
                    {
                        for (const auto& extension : audioExtensions)
                        {
                            const file::Path audioPath(
                                name + extension, pathOptions);
                            if (file::exists(audioPath.get()))
                            {
                                out = audioPath;
                                break;
                            }
                        }
                    }
                    break;
                }
                case FileSequenceAudio::FileName:
                    out = file::Path(
                        path.getDirectory() + fileSequenceAudioFileName,
                        pathOptions);
                    break;
                case FileSequenceAudio::Directory:
                {
                    const file::Path directoryPath(
                        path.getDirectory(), fileSequenceAudioDirectory,
                        pathOptions);
                    file::ListOptions listOptions;
                    listOptions.maxNumberDigits = pathOptions.seqMaxDigits;
                    std::vector<file::FileInfo> list;
                    file::list(directoryPath.get(), list, listOptions);
                    for (const auto& fileInfo : list)
                    {
                        if (file::Type::File == fileInfo.getType())
                        {
                            for (const auto& extension : audioExtensions)
                            {
                                if (extension ==
                                    fileInfo.getPath().getExtension())
                                {
                                    out = fileInfo.getPath();
                                    break;
                                }
                            }
                        }
                    }
                    break;
                }
                default:
                    break;
                }
                return out;
            }

            std::string getKey(const file::Path& path)
            {
                std::vector<std::string> out;
                out.push_back(path.get());
                out.push_back(path.getNumber());
                return string::join(out, ';');
            }


            float transitionValue(double frame, double in, double out)
            {
                return (frame - in) / (out - in);
            }

        } // namespace

        //! Get the OTIO spatial coordinates of a clip. These are optional;
        //! clips without them are laid out from their image size as before.
        //! The coordinates are returned as authored, in the OTIO coordinate
        //! system: unit-less and Y-up.
        std::optional<math::Box2f> getClipBounds(const otio::Clip* otioClip)
        {
            std::optional<math::Box2f> out;
            otio::ErrorStatus errorStatus;
            const auto bounds = otioClip->available_image_bounds(&errorStatus);
            if (bounds.has_value() && !otio::is_error(errorStatus))
            {
                const auto& min = bounds.value().min;
                const auto& max = bounds.value().max;
                out = math::Box2f(
                    math::Vector2f(min.x, min.y),
                    math::Vector2f(max.x, max.y));
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
        //! wide key. Clips that do not have the requested key fall back to the
        //! default media key, and then to the media reference OTIO has active.
        otio::MediaReference* resolveMediaReference(
            const otio::Clip* otioClip,
            const std::string& key,
            const std::map<const otio::Clip*, std::string>& clipKeys)
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
                j = mediaReferences.find(otio::Clip::default_media_key);
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
            const otio::Clip* otioClip,
            Spatial spatial,
            const math::Size2i& normalizeSize,
            double scale)
        {
            std::optional<math::Box2f> out;
            if (Spatial::kNone == spatial)
            {
                return out;
            }
            out = toImageSpace(getClipBounds(otioClip), scale);
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
            const otio::Clip* otioClip,
            Spatial spatial,
            const math::Size2i& normalizeSize,
            double scale,
            const math::Vector2f& offset)
        {
            std::optional<math::Box2f> out;
            if (const auto bounds = getSpatialBounds(
                otioClip,
                spatial,
                normalizeSize,
                scale))
            {
                out = bounds.value() + offset;
            }
            return out;
        }

        bool Options::operator==(const Options& other) const
        {
            return fileSequenceAudio == other.fileSequenceAudio &&
                   spatial == other.spatial &&
                   fileSequenceAudioFileName ==
                       other.fileSequenceAudioFileName &&
                   fileSequenceAudioDirectory ==
                       other.fileSequenceAudioDirectory &&
                   videoRequestCount == other.videoRequestCount &&
                   audioRequestCount == other.audioRequestCount &&
                   requestTimeout == other.requestTimeout &&
                   ioOptions == other.ioOptions &&
                   pathOptions == other.pathOptions;
        }

        bool Options::operator!=(const Options& other) const
        {
            return !(*this == other);
        }

        void Timeline::_init(
            const std::shared_ptr<system::Context>& context,
            file::Path& inputPath, const file::Path& inputAudioPath,
            const Options& options)
        {
            TLRENDER_P();

            std::string error;
            file::Path path = inputPath;
            file::Path audioPath = inputAudioPath;

            otio::SerializableObject::Retainer<otio::Timeline> otioTimeline;

            try
            {
                auto ioSystem = context->getSystem<io::System>();

                // Is the input a sequence?
                const bool isSequence =
                    io::FileType::Sequence ==
                        ioSystem->getFileType(path.getExtension()) &&
                    !path.getNumber().empty();
                if (isSequence)
                {
                    if (!path.isSequence())
                    {
                        // Check for other files in the sequence.
                        std::vector<file::FileInfo> list;
                        file::ListOptions listOptions;
                        listOptions.sequenceExtensions = {path.getExtension()};
                        listOptions.maxNumberDigits =
                            options.pathOptions.seqMaxDigits;
                        file::list(path.getDirectory(), list, listOptions);
                        const auto i = std::find_if(
                            list.begin(), list.end(),
                            [path](const file::FileInfo& value)
                            { return value.getPath().sequence(path); });
                        if (i != list.end())
                        {
                            path = i->getPath();
                        }
                        inputPath = path;
                    }
                    if (audioPath.isEmpty())
                    {
                        // Check for an associated audio file.
                        audioPath = getPathToAudio(
                            path, options.fileSequenceAudio,
                            options.fileSequenceAudioFileName,
                            options.fileSequenceAudioDirectory,
                            options.pathOptions, context);
                    }
                }

                // Is the input a video or audio file?
                if (auto read = ioSystem->read(path, options.ioOptions))
                {
                    const auto info = read->getInfo().get();

                    otime::RationalTime startTime = time::invalidTime;
                    otio::Track* videoTrack = nullptr;
                    otio::Track* audioTrack = nullptr;
                    otio::ErrorStatus errorStatus;

                    // Read the video.
                    if (!info.video.empty())
                    {
                        startTime = info.videoTime->start_time();
                        auto videoClip = new otio::Clip;
                        videoClip->set_source_range(info.videoTime);
                        if (isSequence)
                        {
                            auto mediaReference =
                                new otio::ImageSequenceReference(
                                    "",  // \@bug: not path.getDirectory()?
                                    path.getBaseName(),
                                    path.getSuffix() + path.getExtension(),
                                    info.videoTime->start_time().value(), 1,
                                    info.videoTime->duration().rate(),
                                    path.getPadding());
                            mediaReference->set_available_range(info.videoTime);
                            videoClip->set_media_reference(mediaReference);
                        }
                        else
                        {
                            videoClip->set_media_reference(
                                new otio::ExternalReference(
                                    path.hasProtocol() ? path.get() :
                                    path.getFileName(),
                                    info.videoTime));
                        }
                        videoTrack = new otio::Track(
                            "Video", std::nullopt, otio::Track::Kind::video);
                        videoTrack->append_child(videoClip, &errorStatus);
                        if (otio::is_error(errorStatus))
                        {
                            throw std::runtime_error("Cannot append child");
                        }
                    }

                    // Read the separate audio if provided.
                    if (!audioPath.isEmpty())
                    {
                        if (auto audioRead =
                                ioSystem->read(audioPath, options.ioOptions))
                        {
                            bool protocol = audioPath.hasProtocol();
                            if (file::exists(audioPath.get()))
                                protocol = false;
                            const std::string cwd = file::getCWD();
                            if (file::exists(cwd + audioPath.get()))
                            {
                                audioPath = file::Path(cwd + audioPath.get());
                                protocol = false;
                            }

                            const auto audioInfo = audioRead->getInfo().get();

                            auto audioClip = new otio::Clip;
                            audioClip->set_source_range(audioInfo.audioTime);
                            audioClip->set_media_reference(
                                new otio::ExternalReference(
                                    protocol ? audioPath.get() :
                                    audioPath.getFileName(),
                                    audioInfo.audioTime));

                            audioTrack = new otio::Track(
                                "Audio", std::nullopt,
                                otio::Track::Kind::audio);
                            audioTrack->append_child(audioClip, &errorStatus);
                            if (otio::is_error(errorStatus))
                            {
                                throw std::runtime_error("Cannot append child");
                            }
                        }
                    }
                    else if (info.audio.isValid())
                    {
                        if (startTime.is_invalid_time())
                        {
                            startTime = info.audioTime->start_time();
                        }

                        auto audioClip = new otio::Clip;
                        audioClip->set_source_range(info.audioTime);
                        audioClip->set_media_reference(
                            new otio::ExternalReference(
                                path.hasProtocol() ? path.get() :
                                path.getFileName(),
                                info.audioTime));

                        audioTrack = new otio::Track(
                            "Audio", std::nullopt, otio::Track::Kind::audio);
                        audioTrack->append_child(audioClip, &errorStatus);
                        if (otio::is_error(errorStatus))
                        {
                            throw std::runtime_error("Cannot append child");
                        }
                    }

                    // Create the stack.
                    auto otioStack = new otio::Stack;
                    if (videoTrack)
                    {
                        otioStack->append_child(videoTrack, &errorStatus);
                        if (otio::is_error(errorStatus))
                        {
                            throw std::runtime_error("Cannot append child");
                        }
                    }
                    if (audioTrack)
                    {
                        otioStack->append_child(audioTrack, &errorStatus);
                        if (otio::is_error(errorStatus))
                        {
                            throw std::runtime_error("Cannot append child");
                        }
                    }

                    // Create the timeline.
                    otioTimeline = new otio::Timeline(path.get());
                    otioTimeline->set_tracks(otioStack);
                    if (time::isValid(startTime))
                    {
                        otioTimeline->set_global_start_time(startTime);
                    }
                }
            }
            catch (const std::exception& e)
            {
                error = e.what();
            }

            auto logSystem = context->getLogSystem();
            logSystem->print(
                "tl::timeline::create",
                string::Format("\n"
                               "    Create from path: {0}\n"
                               "    Audio path: {1}")
                    .arg(path.get())
                    .arg(audioPath.get()));

            // Is the input an OTIO file?
            if (!otioTimeline)
            {
                otio::ErrorStatus errorStatus;
                const std::string fileName = path.get();
                const std::string extension = string::toLower(path.getExtension());
                if (".otio" == extension)
                {
                    auto timeline =
                        otio::Timeline::from_json_file(fileName, &errorStatus);
                    otioTimeline = dynamic_cast<otio::Timeline*>(timeline);
                    if (!otioTimeline)
                    {
                        auto collection =
                            dynamic_cast<otio::SerializableCollection*>(timeline);
                        if (collection)
                        {
                            auto children =
                                collection->find_children<otio::Timeline>();
                            if (children.size() > 1)
                            {
                                throw std::runtime_error(
                                    string::Format(
                                        "{0}: Only one timeline is supported.")
                                    .arg(fileName));
                            }
                            else if (children.size() == 1)
                            {
                                otioTimeline = otio::dynamic_retainer_cast<otio::Timeline>(
                                    children[0]);
                            }
                        }
                    }
                }
                else if (".otioz" == extension)
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
                    otioTimeline = dynamic_cast<otio::Timeline*>(
                        otio::Timeline::from_json_string(json, &errorStatus));
                    if (!otioTimeline)
                    {
                        throw std::runtime_error(
                            string::Format("Cannot read timeline: \"{0}\"").
                            arg(path.get()));
                    }
                    else if (otio::is_error(errorStatus))
                    {
                        throw std::runtime_error(
                            string::Format("Cannot read timeline: \"{0}\": {1}").
                            arg(path.get()).
                            arg(errorStatus.details));
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
                        otio::MediaReference* mediaReference,
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
                                dynamic_cast<otio::ExternalReference*>(mediaReference))
                            {
                                first = file::Path(
                                    url::decode(externalReference->target_url())).get();
                            }
                            else if (auto imageSeqReference =
                                     dynamic_cast<otio::ImageSequenceReference*>(mediaReference))
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
                    for (auto clip : otioTimeline->find_children<otio::Clip>())
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
                throw std::runtime_error(error);
            }

            otio::AnyDictionary dict;
            dict["path"] = path.get();
            dict["audioPath"] = audioPath.get();
            otioTimeline->metadata()["tlRender"] = dict;

            _init(context, otioTimeline, options);
        }

        void Timeline::_init(
            const std::shared_ptr<system::Context>& context,
            const otio::SerializableObject::Retainer<otio::Timeline>&
                otioTimeline,
            const Options& options)
        {
            TLRENDER_P();

            auto logSystem = context->getLogSystem();
            {
                std::vector<std::string> lines;
                lines.push_back(std::string());
                lines.push_back(string::Format("    File sequence audio: {0}")
                                    .arg(options.fileSequenceAudio));
                lines.push_back(
                    string::Format("    File sequence audio file name: {0}")
                        .arg(options.fileSequenceAudioFileName));
                lines.push_back(
                    string::Format("    File sequence audio directory: {0}")
                        .arg(options.fileSequenceAudioDirectory));
                lines.push_back(string::Format("    Video request count: {0}")
                                    .arg(options.videoRequestCount));
                lines.push_back(string::Format("    Audio request count: {0}")
                                    .arg(options.audioRequestCount));
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
            p.timelineChanges = observer::Value<bool>::create(false);
            const auto i = otioTimeline->metadata().find("tlRender");
            if (i != otioTimeline->metadata().end())
            {
                try
                {
                    const auto dict =
                        std::any_cast<otio::AnyDictionary>(i->second);
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
            p.readCache.setMax(readCacheMax);

            // Get information about the timeline.
            for (const auto& i : p.otioTimeline.value->tracks()->children())
            {
                if (auto otioTrack = dynamic_cast<const otio::Track*>(i.value))
                {
                    if (otio::Track::Kind::audio == otioTrack->kind())
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
            p.thread.thread = std::thread(
                [this]
                    {
                        TLRENDER_P();
                        p.thread.logTimer = std::chrono::steady_clock::now();
                        while (p.thread.running)
                        {
                            _tick();
                        }
                        _finishRequests();
                    });
        }

        Timeline::Timeline() :
            _p(new Private)
        {
        }

        Timeline::~Timeline()
        {
            TLRENDER_P();
            p.thread.running = false;
            if (p.thread.thread.joinable())
            {
                p.thread.thread.join();
            }
        }

        std::shared_ptr<Timeline> Timeline::create(
            const std::shared_ptr<system::Context>& context,
            const otio::SerializableObject::Retainer<otio::Timeline>& timeline,
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

        const otio::SerializableObject::Retainer<otio::Timeline>&
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
            const otio::SerializableObject::Retainer<otio::Timeline>& value)
        {
            TLRENDER_P();
            p.otioTimeline = value;
            if (p.otioTimeline.value)
            {
                _timelineUpdate();
            }

            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            if (!p.mutex.stopped)
            {
                p.mutex.otioTimeline = value;
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

        std::optional<size_t>
        Timeline::getBundleMemoryOffset(const otio::MediaReference* otioRef) const
        {
            TLRENDER_P();

            // Verify via existing bookkeeping that this reference is part of the bundle
            if (p.bundleMediaReferences.find(otioRef) == p.bundleMediaReferences.end())
            {
                return std::nullopt;
            }

            // Determine the internal file path for the media reference
            std::string mediaFileName;
            if (auto externalReference = dynamic_cast<const otio::ExternalReference*>(otioRef))
            {
                mediaFileName = file::Path(url::decode(externalReference->target_url())).get();
            }
            else if (auto imageSeqReference = dynamic_cast<const otio::ImageSequenceReference*>(otioRef))
            {
                // For sequences, grab the offset using the first frame
                if (imageSeqReference->number_of_images_in_sequence() > 0)
                {
                    mediaFileName = file::Path(
                        url::decode(imageSeqReference->target_url_for_image_number(0))).get();
                }
            }

            // If a valid path is derived, query the ZipReader for its offset inside the memory block
            if (!mediaFileName.empty() && p.zipReader)
            {
                const auto entry = p.zipReader->find(mediaFileName);
                if (entry.has_value())
                {
                    return entry->offset;
                }
            }

            return std::nullopt;
        }

        std::vector<file::MemoryRead>
        Timeline::getMem(const otio::MediaReference* otioRef)
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
            if (auto externalReference = dynamic_cast<const otio::ExternalReference*>(otioRef))
            {
                mediaFileNames.push_back(std::make_pair(
                                             size_t(0),
                                             file::Path(url::decode(externalReference->target_url())).get()));
            }
            else if (auto imageSeqReference =
                     dynamic_cast<const otio::ImageSequenceReference*>(otioRef))
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

        otio::MediaReference* Timeline::Private::mediaReference(
            const otio::Clip* otioClip) const
        {
            return resolveMediaReference(
                otioClip,
                thread.mediaReferenceKey,
                thread.clipMediaReferenceKeys);
        }

        std::vector<std::string> Timeline::getMediaReferenceKeys() const
        {
            TLRENDER_P();

            std::set<std::string> keys;
            for (const auto& otioClip :
                     p.otioTimeline.value->find_children<otio::Clip>())
            {
                for (const auto& i : otioClip->media_references())
                {
                    keys.insert(i.first);
                }
            }
            return std::vector<std::string>(keys.begin(), keys.end());
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
            const otio::Clip* otioClip) const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            const auto i = p.mutex.clipMediaReferenceKeys.find(otioClip);
            return i != p.mutex.clipMediaReferenceKeys.end() ?
                i->second :
                std::string();
        }

        void Timeline::setMediaReferenceKey(
            const otio::Clip* otioClip,
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

        otio::MediaReference* Timeline::getMediaReference(
            const otio::Clip* otioClip) const
        {
            TLRENDER_P();
            std::unique_lock<std::mutex> lock(p.mutex.mutex);
            return resolveMediaReference(
                otioClip,
                p.mutex.mediaReferenceKey,
                p.mutex.clipMediaReferenceKeys);
        }

        const otime::TimeRange& Timeline::getTimeRange() const
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
            const otime::RationalTime& time, const io::Options& options)
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
                        newVideoRequests.size()) < p.options.videoRequestCount)
                {
                    newVideoRequests.push_back(p.mutex.videoRequests.front());
                    p.mutex.videoRequests.pop_front();
                }
                while (!p.mutex.audioRequests.empty() &&
                       (p.thread.audioRequestsInProgress.size() +
                        newAudioRequests.size()) < p.options.audioRequestCount)
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
                                dynamic_cast<otio::Item*>(otioChild.value))
                            {
                                const auto requestTime =
                                    request->time - p.timeRange.start_time();
                                otio::ErrorStatus errorStatus;
                                const auto range =
                                    otioItem->trimmed_range_in_parent(
                                        &errorStatus);
                                if (range.has_value() &&
                                    range.value().contains(requestTime))
                                {
                                    Private::VideoLayerData videoLayerData;
                                    if (auto otioClip =
                                        dynamic_cast<const otio::Clip*>(
                                            otioItem))
                                    {
                                        videoLayerData.image = _readVideo(
                                            otioClip, requestTime,
                                            request->options);
                                        videoLayerData.bounds = getCanvasBox(
                                            otioClip,
                                            p.options.spatial,
                                            p.normalizeSize,
                                            p.boundsScale,
                                            p.canvasOffset);
                                    }
                                    const auto neighbors =
                                        otioTrack->neighbors_of(
                                            otioItem, &errorStatus);
                                    if (auto otioTransition =
                                        dynamic_cast<otio::Transition*>(
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
                                                transitionValue(
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
                                                dynamic_cast<otio::Clip*>(
                                                    transitionNeighbors
                                                    .second.value))
                                            {
                                                videoLayerData.imageB = _readVideo(
                                                    otioClipB, requestTime,
                                                    request->options);
                                                videoLayerData.bounds = getCanvasBox(
                                                    otioClipB,
                                                    p.options.spatial,
                                                    p.normalizeSize,
                                                    p.boundsScale,
                                                    p.canvasOffset);
                                            }
                                        }
                                    }
                                    if (auto otioTransition =
                                        dynamic_cast<otio::Transition*>(
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
                                                transitionValue(
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
                                                dynamic_cast<otio::Clip*>(
                                                    transitionNeighbors
                                                    .first.value))
                                            {
                                                videoLayerData.image = _readVideo(
                                                    otioClipB, requestTime,
                                                    request->options);
                                                videoLayerData.bounds = getCanvasBox(
                                                    otioClipB,
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
                                dynamic_cast<otio::Item*>(otioChild.value))
                            {
                                const auto rangeOptional =
                                    otioItem->trimmed_range_in_parent();
                                if (rangeOptional.has_value())
                                {
                                    const otime::TimeRange clipTimeRange(
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
                                    const otime::TimeRange requestTimeRange =
                                        otime::TimeRange(
                                            otime::RationalTime(start, 1.0),
                                            otime::RationalTime(1.0, 1.0));
                                    otime::TimeRange transitionRange =
                                        clipTimeRange;

                                    otio::ErrorStatus errorStatus;
                                    const auto neighbors =
                                        otioTrack->neighbors_of(
                                            otioItem, &errorStatus);
                                    if (auto otioTransition =
                                        dynamic_cast<otio::Transition*>(
                                            neighbors.first.value))
                                    {
                                        const auto inOffset =
                                            otioTransition->in_offset()
                                            .rescaled_to(1.0);
                                        transitionRange = otime::TimeRange(
                                            transitionRange.start_time() -
                                            inOffset,
                                            transitionRange.duration() +
                                            inOffset);
                                    }

                                    if (auto otioTransition =
                                        dynamic_cast<otio::Transition*>(
                                            neighbors.second.value))
                                    {
                                        const auto outOffset =
                                            otioTransition->out_offset()
                                            .rescaled_to(1.0);
                                        transitionRange = otime::TimeRange(
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
                                        //! otime::TimeRange::clamped() not
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
                                        audioData.timeRange = otime::TimeRange(
                                            otime::RationalTime(start, 1.0),
                                            otime::RationalTime(
                                                end - start, 1.0));

                                        if (auto otioClip =
                                            dynamic_cast<otio::Clip*>(
                                                otioItem))
                                        {
                                            audioData.audio = _readAudio(
                                                otioClip, audioData.timeRange,
                                                request->options);
                                        }

                                        if (auto otioTransition =
                                            dynamic_cast<otio::Transition*>(
                                                neighbors.second.value))
                                        {
                                            const auto pad =
                                                otime::RationalTime(1.0, 1.0);
                                            const auto inOffset =
                                                otioTransition->in_offset()
                                                .rescaled_to(1.0);
                                            const auto outOffset =
                                                otioTransition->out_offset()
                                                .rescaled_to(1.0);
                                            auto transitionRange =
                                                otime::TimeRange(
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
                                            dynamic_cast<otio::Transition*>(
                                                neighbors.first.value))
                                        {
                                            const auto outOffset =
                                                otioTransition->out_offset()
                                                .rescaled_to(1.0);
                                            const auto inOffset =
                                                otioTransition->in_offset()
                                                .rescaled_to(1.0);
                                            auto transitionRange =
                                                otime::TimeRange(
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

        std::shared_ptr<io::IRead> Timeline::_getRead(
            const otio::Clip* clip,
            const io::Options& ioOptions)
        {
            TLRENDER_P();
            return _getRead(p.mediaReference(clip), ioOptions);
        }

        std::shared_ptr<io::IRead> Timeline::_getRead(
            const otio::MediaReference* mediaReference,
            const io::Options& ioOptions)
        {
            TLRENDER_P();
            std::shared_ptr<io::IRead> out;
            if (p.unavailableMediaReferences.find(mediaReference) !=
                p.unavailableMediaReferences.end())
            {
                // Named by the bundle but not inside it. Reading it from its path
                // would be reading a different file than the bundle describes.
                return out;
            }
            const auto path = timeline::getPath(
                mediaReference,
                p.path.getDirectory(),
                p.options.pathOptions);
            const std::string key = getKey(path);
            if (!p.readCache.get(key, out))
            {
                if (auto context = p.context.lock())
                {
                    const auto memoryRead = getMemoryRead(mediaReference);
                    io::Options options = ioOptions;
                    options["SequenceIO/DefaultSpeed"] =
                        string::Format("{0}").arg(p.timeRange.duration().rate());
                    const auto ioSystem = context->getSystem<io::System>();
                    out = ioSystem->read(path, memoryRead, options);
                    p.readCache.add(key, out);
                }
            }
            return out;
        }

        std::future<io::VideoData> Timeline::_readVideo(
            const otio::Clip* clip, const otime::RationalTime& time,
            const io::Options& options)
        {
            TLRENDER_P();

            std::future<io::VideoData> out;
            io::Options optionsMerged =
                io::merge(options, p.options.ioOptions);
            optionsMerged["USD/cameraName"] = clip->name();
            auto read = _getRead(clip, optionsMerged);
            const auto timeRangeOpt = clip->trimmed_range_in_parent();
            if (read && timeRangeOpt.has_value())
            {
                const io::Info& ioInfo = read->getInfo().get();
                OTIO_NS::TimeRange availableRange = clip->available_range();
                OTIO_NS::TimeRange trimmedRange = clip->trimmed_range();
                if (p.options.compat &&
                    availableRange.start_time() > ioInfo.videoTime->start_time())
                {
                    //! \bug If the available range is greater than the media
                    //! time, assume the media time is wrong and compensate
                    //! for it.
                    trimmedRange = otio::TimeRange(
                        trimmedRange.start_time() - availableRange.start_time(),
                        trimmedRange.duration());
                }
                const auto mediaTime = timeline::toVideoMediaTime(
                    time, timeRangeOpt.value(), clip->trimmed_range(),
                    ioInfo.videoTime->duration().rate());
                out = read->readVideo(mediaTime, optionsMerged);
            }
            return out;
        }

        std::future<io::AudioData> Timeline::_readAudio(
            const otio::Clip* clip, const otime::TimeRange& timeRange,
            const io::Options& options)
        {
            TLRENDER_P();

            std::future<io::AudioData> out;
            io::Options optionsMerged =
                io::merge(options, p.options.ioOptions);
            auto read = _getRead(clip, optionsMerged);
            const auto timeRangeOpt = clip->trimmed_range_in_parent();
            if (read && timeRangeOpt.has_value())
            {
                const io::Info& ioInfo = read->getInfo().get();
                otime::TimeRange trimmedRange = clip->trimmed_range();
                if (p.options.compat &&
                    trimmedRange.start_time() < ioInfo.audioTime->start_time())
                {
                    //! \bug If the trimmed range is less than the media time,
                    //! assume the media time is wrong (e.g., ALab trailer) and
                    //! compensate for it.
                    trimmedRange = otio::TimeRange(
                        ioInfo.audioTime->start_time() + trimmedRange.start_time(),
                        trimmedRange.duration());
                }
                const auto mediaRange = timeline::toAudioMediaTime(
                    timeRange, timeRangeOpt.value(), trimmedRange,
                    ioInfo.audio.sampleRate);
                out = read->readAudio(mediaRange, optionsMerged);
            }
            return out;
        }

        bool Timeline::_getVideoInfo(const otio::Composable* composable)
        {
            TLRENDER_P();
            if (auto clip = dynamic_cast<const otio::Clip*>(composable))
            {
                if (auto context = p.context.lock())
                {
                    // The first video clip defines the video information for the timeline.
                    if (auto read = _getRead(clip, p.options.ioOptions))
                    {
                        const io::Info& ioInfo = read->getInfo().get();
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
                            if (auto mediaReferenceRead =
                                _getRead(i.second, p.options.ioOptions))
                            {
                                const io::Info& mediaReferenceInfo =
                                    mediaReferenceRead->getInfo().get();

                                // Kept so that getIOInfo() can report the media
                                // that is actually being read; completed with the
                                // timeline level information once it is known.
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
            if (auto composition = dynamic_cast<const otio::Composition*>(composable))
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

        bool Timeline::_getAudioInfo(const otio::Composable* composable)
        {
            TLRENDER_P();
            if (auto clip = dynamic_cast<const otio::Clip*>(composable))
            {
                if (auto context = p.context.lock())
                {
                    // The first audio clip defines the audio information for
                    // the timeline.
                    if (auto read = _getRead(clip, p.options.ioOptions))
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
            if (auto composition = dynamic_cast<const otio::Composition*>(composable))
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
            const auto otioClips = p.otioTimeline.value->find_children<otio::Clip>();

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
                        otioClip,
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
                     p.otioTimeline.value->find_children<otio::Clip>())
            {
                for (const auto& i : otioClip->media_references())
                {
                    if (auto read = _getRead(i.second, p.options.ioOptions))
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

        void Timeline::_timelineUpdate()
        {
            TLRENDER_P();

            p.timeRange = timeline::getTimeRange(p.otioTimeline.value);
            // The old videoInfoClip/videoInfoByReference pointers belonged
            // to the tree we just released above and are now dangling --
            // they must be rebuilt against the new tree, not reused.
            p.videoInfoClip = nullptr;
            p.videoInfoByReference.clear();
            p.maxVideoSize = math::Size2i();
            p.canvasSize = math::Size2i();
            p.canvasOffset = math::Vector2f();
            p.normalizeSize = math::Size2i();
            p.boundsScale = 1.0;

            for (const auto& i : p.otioTimeline.value->tracks()->children())
            {
                if (auto otioTrack = dynamic_cast<const otio::Track*>(i.value))
                {
                    if (otio::Track::Kind::video == otioTrack->kind())
                    {
                        if (_getVideoInfo(otioTrack))
                        {
                            break;
                        }
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
        namespace
        {
            const std::chrono::milliseconds timeout(5);
        }

        void Timeline::_tick()
        {
            TLRENDER_P();

            const auto t0 = std::chrono::steady_clock::now();

            _requests();

            // Logging.
            auto t1 = std::chrono::steady_clock::now();
            const std::chrono::duration<float> diff = t1 - p.thread.logTimer;
            if (diff.count() > 10.F)
            {
                p.thread.logTimer = t1;
                if (auto context = p.context.lock())
                {
                    size_t videoRequestsSize = 0;
                    size_t audioRequestsSize = 0;
                    {
                        std::unique_lock<std::mutex> lock(p.mutex.mutex);
                        videoRequestsSize = p.mutex.videoRequests.size();
                        audioRequestsSize = p.mutex.audioRequests.size();
                    }
                    auto logSystem = context->getLogSystem();
                    logSystem->print(
                        string::Format("tl::timeline::Timeline {0}").arg(this),
                        string::Format(
                            "\n"
                            "    Path: {0}\n"
                            "    Video requests: {1}, {2} in-progress, {3} "
                            "max\n"
                            "    Audio requests: {4}, {5} in-progress, {6} max")
                            .arg(p.path.get())
                            .arg(videoRequestsSize)
                            .arg(p.thread.videoRequestsInProgress.size())
                            .arg(p.options.videoRequestCount)
                            .arg(audioRequestsSize)
                            .arg(p.thread.audioRequestsInProgress.size())
                            .arg(p.options.audioRequestCount));
                }
                t1 = std::chrono::steady_clock::now();
            }

            // Sleep for a bit.
            time::sleep(timeout, t0, t1);
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
                        layer.image = i.image.get().image;
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
                    if (auto context = this->context.lock())
                    {
                        auto logSystem = context->getLogSystem();
                        logSystem->print(
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
                    if (auto context = this->context.lock())
                    {
                        auto logSystem = context->getLogSystem();
                        logSystem->print(
                            "tl::Timeline",
                            e.what(),
                            log::Type::Error);
                    }
                }
                frame.layers.push_back(layer);
            }
            if (frame.layers.empty())
            {
                auto audio = audio::Audio::create(ioInfo.audio, ioInfo.audio.sampleRate);
                audio->zero();
                frame.layers.push_back({ audio });
            }
            return frame;
        }


        std::shared_ptr<audio::Audio> Timeline::Private::padAudioToOneSecond(
            const std::shared_ptr<audio::Audio>& audio, double seconds,
            const otime::TimeRange& timeRange)
        {
            std::list<std::shared_ptr<audio::Audio> > list;
            const double s =
                seconds - this->timeRange.start_time().rescaled_to(1.0).value();
            if (timeRange.start_time().value() > s)
            {
                const otime::RationalTime t =
                    timeRange.start_time() - otime::RationalTime(s, 1.0);
                const otime::RationalTime t2 =
                    t.rescaled_to(audio->getInfo().sampleRate);
                auto silence =
                    audio::Audio::create(audio->getInfo(), t2.value());
                silence->zero();
                list.push_back(silence);
            }
            list.push_back(audio);
            if (timeRange.end_time_exclusive().value() < s + 1.0)
            {
                const otime::RationalTime t =
                    otime::RationalTime(s + 1.0, 1.0) -
                    timeRange.end_time_exclusive();
                const otime::RationalTime t2 =
                    t.rescaled_to(audio->getInfo().sampleRate);
                auto silence =
                    audio::Audio::create(audio->getInfo(), t2.value());
                silence->zero();
                list.push_back(silence);
            }
            size_t sampleCount = audio::getSampleCount(list);
            auto out = audio::Audio::create(audio->getInfo(), sampleCount);
            audio::move(list, out->getData(), out->getByteCount());
            return out;
        }

    } // namespace timeline
} // namespace tl
