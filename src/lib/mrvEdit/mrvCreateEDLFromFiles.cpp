// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include <tlIO/System.h>

#include <tlCore/File.h>
#include <tlCore/FileInfo.h>
#include <tlCore/Path.h>
#include <tlCore/StringFormat.h>

#include <tlTimeline/Timeline.h>
#include <tlTimeline/Util.h>

#include "mrvOS/mrvI8N.h"
#include "mrvCore/mrvFile.h"

#include "mrvFl/mrvIO.h"

#include "mrvEdit/mrvCreateEDLFromFiles.h"
#include "mrvEdit/mrvEditUtil.h"

#include "mrvApp/mrvApp.h"

#include <opentimelineio/clip.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/gap.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/timeline.h>

namespace
{
    const char* kModule = "edl";
}


namespace mrv
{
    using namespace tl;

    using OTIO_NS::RationalTime;
    using OTIO_NS::TimeRange;

    using OTIO_NS::Clip;
    using OTIO_NS::Composition;
    using OTIO_NS::Gap;
    using OTIO_NS::Timeline;
    using OTIO_NS::Track;

    bool createEDLFromFiles(std::string& otioFile,
                            const std::vector<std::string> files)
    {
        OTIO_NS::ErrorStatus errorStatus;

        const timeline::Options options;

        OTIO_NS::SerializableObject::Retainer<OTIO_NS::Timeline> otioTimeline =
            new OTIO_NS::Timeline("EDL");

        auto videoTrack =
            new OTIO_NS::Track("Video", std::nullopt, OTIO_NS::Track::Kind::video);

        auto audioTrack =
            new OTIO_NS::Track("Audio", std::nullopt, OTIO_NS::Track::Kind::audio);


        auto context = App::app->getContext();
        auto ioSystem = context->getSystem<io::ReadSystem>();

        std::optional<OTIO_NS::RationalTime> startTime;

        for (const auto& file : files)
        {
            tl::file::Path path(file);
            if (!path.isAbsolute())
            {
                path = tl::file::Path(tl::file::getCWD() + file);
            }

            if (mrv::file::isOTIO(path))
            {
                LOG_ERROR(_("Cannot currently concatenate an .otio file."));
                return false;
            }

            if (file::isDirectory(file))
            {
                LOG_ERROR(_("Cannot currently concatenate a directory."));
                return false;
            }

            if (!mrv::file::isReadable(file))
            {
                /* xgettext:c-format */
                const std::string err =
                    string::Format(
                        _("Filename '{0}' does not exist or does not "
                          "have read permissions."))
                        .arg(file);
                LOG_ERROR(err);
                continue;
            }

            // Is the input a sequence?
            const bool isSequence = mrv::file::isSequence(path);
            if (isSequence)
            {
                path = tl::file::expandSeq(path, options.pathOptions);
            }

            // Read the file. A sequence is read by a decoder, which holds no
            // thread; only a format that has to be read statefully still needs
            // a reader here.
            auto ioSystem = context->getSystem<io::ReadSystem>();
            io::Info info;
            bool infoValid = false;
            // What a reader said went wrong, when it said it somewhere other
            // than by throwing.
            std::string readError;
            if (auto plugin = ioSystem->getPlugin(path))
            {
                if (auto decode = plugin->decode(options.ioOptions))
                {
                    info = io::SeqDecode::create(
                        path, {}, decode, options.ioOptions)->getInfo();
                    infoValid = true;
                }
            }
            if (!infoValid)
            {
                // Which tracks this file gets depends on both halves, so both are
                // read and merged here. The readers are temporary: what the
                // timeline goes on to read is decided by the tracks below.
                auto videoRead = ioSystem->videoRead(path, options.ioOptions);
                auto audioRead = ioSystem->audioRead(path, options.ioOptions);
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
                }
                io::Info audioInfo;
                if (audioFuture.valid())
                {
                    audioInfo = audioFuture.get();
                }
                info = io::merge(videoInfo, audioInfo);

                // Whether anything was read, rather than whether there was a
                // reader to ask. A reader that reports its failure on its own
                // thread -- FFmpeg does -- answers the information request with
                // nothing instead of throwing, and taking that for a readable
                // file would build a timeline with no tracks in it: opening a
                // movie that is not there would give an empty tab rather than
                // an error.
                infoValid = !info.video.empty() || info.audio.isValid();
                if (!infoValid)
                {
                    // The reader's own account of it, so that a file that is
                    // missing says so rather than being reported as unreadable
                    // for reasons unknown.
                    const std::string error = videoRead ?
                        videoRead->getError() :
                        std::string();
                    readError = !error.empty() ?
                        error :
                        (audioRead ? audioRead->getError() : std::string());
                    LOG_ERROR(readError);
                    continue;
                }
            }

            if (infoValid)
            {
                // Read the video.
                if (!info.video.empty())
                {
                    startTime = info.videoTime->start_time();
                    const double rate = info.videoTime->duration().rate();
                    const io::MissingFrames missingFrames =
                        io::getMissingFrames(options.ioOptions);

                    // Every clip names the whole sequence over the whole range it
                    // covers, whatever the clip itself takes out of it. They are
                    // separate objects because a clip owns its reference, but they
                    // describe the same file, so the reads behind them share one
                    // decoder.
                    const auto makeClip =
                        [&](const OTIO_NS::TimeRange& sourceRange)
                            {
                                auto out = new OTIO_NS::Clip;
                                out->set_source_range(sourceRange);
                                if (path.isSequence())
                                {
                                    auto mediaReference =
                                        new OTIO_NS::ImageSequenceReference(
                                            path.getDirectory(),
                                            path.getBaseName(),
                                            path.getExtension(),
                                            info.videoTime->start_time().value(),
                                            1,
                                            rate,
                                            path.getPadding(),
                                            // A file opened directly has no reference
                                            // to say what to do about frames it is
                                            // missing, so it takes the options the
                                            // timeline was opened with.
                                            timeline::toOTIO(missingFrames));
                                    mediaReference->set_available_range(*info.videoTime);
                                    out->set_media_reference(mediaReference);
                                }
                                else
                                {
                                    const bool listdir = true;
                                    out->set_media_reference(
                                        new OTIO_NS::ExternalReference(
                                            path.getFileName(listdir),
                                            *info.videoTime));
                                }
                                return out;
                            };

                    // Nothing to take out, so this is the same single clip a
                    // complete sequence gets. A lone run still covers only
                    // itself, which is what Skip means when the frames that
                    // are there are consecutive.
                    videoTrack->append_child(makeClip(*info.videoTime));
                }
                else if (info.audioTime.has_value())
                {
                    auto gap = new OTIO_NS::Gap();
                    gap->set_source_range(*info.audioTime);
                    videoTrack->append_child(gap);
                }

                // Read the separate audio if provided.
                if (info.audio.isValid())
                {
                    auto audioClip = new OTIO_NS::Clip;
                    audioClip->set_source_range(*info.audioTime);
                    audioClip->set_media_reference(
                        new OTIO_NS::ExternalReference(
                            path.getFileName(true),
                            *info.audioTime));
                    audioTrack->append_child(audioClip);

                    // Add a gap if audio is shorter than the video.
                    if (info.videoTime.has_value())
                    {
                        auto audioDuration = info.audioTime->duration();
                        auto videoDuration = info.videoTime->duration().rescaled_to(audioDuration.rate());
                        auto diff = videoDuration - audioDuration;
                        if (diff.value() > 0.F)
                        {
                            auto gap = new OTIO_NS::Gap();
                            auto duration = OTIO_NS::TimeRange(
                                OTIO_NS::RationalTime(0.F,
                                                      audioDuration.rate()),
                                diff);
                            gap->set_source_range(duration);
                            audioTrack->append_child(gap);
                        }
                    }
                }
                else if (info.videoTime.has_value())
                {
                    auto gap = new OTIO_NS::Gap();
                    gap->set_source_range(*info.videoTime);
                    audioTrack->append_child(gap);
                }
            }
        }

        auto stack = new OTIO_NS::Stack;
        stack->append_child(videoTrack, &errorStatus);
        if (OTIO_NS::is_error(errorStatus))
        {
            /* xgettext:c++-format */
            std::string error =
                string::Format(_("Could not append video track {0}"))
                .arg(errorStatus.full_description);
            throw std::runtime_error(error);
        }

        stack->append_child(audioTrack, &errorStatus);
        if (OTIO_NS::is_error(errorStatus))
        {
            /* xgettext:c++-format */
            std::string error =
                string::Format(_("Could not append audio track {0}"))
                .arg(errorStatus.full_description);
            throw std::runtime_error(error);
        }

        otioTimeline->set_tracks(stack);

        otioFile = otioFilename(App::ui);

        otioTimeline->to_json_file(otioFile, &errorStatus);

        if (OTIO_NS::is_error(errorStatus))
        {
            std::string error =
                string::Format(_("Could not save .otio file: {0}"))
                .arg(errorStatus.full_description);
            throw std::runtime_error(error);
        }

        return true;
    }
}
