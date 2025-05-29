// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/TimelineTest.h>

#include <tlTimeline/Timeline.h>
#include <tlTimeline/Util.h>

#include <tlIO/System.h>

#include <tlCore/Assert.h>
#include <tlCore/File.h>
#include <tlCore/StringFormat.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/timeline.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        TimelineTest::TimelineTest(
            const std::shared_ptr<system::Context>& context) :
            ITest("timeline_tests::TimelineTest", context)
        {
        }

        std::shared_ptr<TimelineTest>
        TimelineTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<TimelineTest>(new TimelineTest(context));
        }

        void TimelineTest::run()
        {
            _enums();
            _options();
            _util();
            _transitions();
            _videoData();
            _timeline();
            _separateAudio();
            _setTimeline();
        }

        void TimelineTest::_enums()
        {
            _enum<FileSequenceAudio>(
                "FileSequenceAudio", getFileSequenceAudioEnums);
            _enum<Transition>("Transition", getTransitionEnums);
        }

        void TimelineTest::_options()
        {
            Options a;
            a.fileSequenceAudio = FileSequenceAudio::Directory;
            TLRENDER_ASSERT(a == a);
            TLRENDER_ASSERT(a != Options());
        }

        void TimelineTest::_util() {}

        void TimelineTest::_transitions()
        {
            {
                TLRENDER_ASSERT(
                    toTransition(std::string()) == Transition::None);
                TLRENDER_ASSERT(
                    toTransition("SMPTE_Dissolve") == Transition::Dissolve);
            }
        }

        void TimelineTest::_videoData()
        {
            {
                VideoLayer a, b;
                TLRENDER_ASSERT(a == b);
                a.transition = Transition::Dissolve;
                TLRENDER_ASSERT(a != b);
            }
            {
                VideoData a, b;
                TLRENDER_ASSERT(a == b);
                a.time = otime::RationalTime(1.0, 24.0);
                TLRENDER_ASSERT(a != b);
            }
        }

        void TimelineTest::_timeline()
        {
            // Test timelines.
            const std::vector<file::Path> paths = {
                file::Path(TLRENDER_SAMPLE_DATA, "BART_2021-02-07.m4v"),
                file::Path(
                    TLRENDER_SAMPLE_DATA, "Seq/BART_2021-02-07.0001.jpg"),
                file::Path(TLRENDER_SAMPLE_DATA, "MovieAndSeq.otio"),
                file::Path(TLRENDER_SAMPLE_DATA, "TransitionGap.otio"),
                file::Path(TLRENDER_SAMPLE_DATA, "SingleClip.otioz"),
                file::Path(TLRENDER_SAMPLE_DATA, "SingleClipSeq.otioz")};
            for (const auto& path : paths)
            {
                try
                {
                    _print(string::Format("Timeline: {0}").arg(path.get()));
                    auto timeline = Timeline::create(path, _context);
                    _timeline(timeline);
                }
                catch (const std::exception& e)
                {
                    _printError(e.what());
                }
            }
            for (const auto& path : paths)
            {
                try
                {
                    _print(
                        string::Format("Memory timeline: {0}").arg(path.get()));
                    auto otioTimeline = timeline::create(path, _context);
                    toMemoryReferences(
                        otioTimeline, path.getDirectory(),
                        ToMemoryReference::Shared);
                    auto timeline =
                        timeline::Timeline::create(otioTimeline, _context);
                    _timeline(timeline);
                }
                catch (const std::exception& e)
                {
                    _printError(e.what());
                }
            }
        }

        void TimelineTest::_timeline(
            const std::shared_ptr<timeline::Timeline>& timeline)
        {
            // Get video from the timeline.
            const otime::TimeRange& timeRange = timeline->getTimeRange();
            std::vector<timeline::VideoData> videoData;
            std::vector<timeline::VideoRequest> videoRequests;
            for (size_t i = 0;
                 i < static_cast<size_t>(timeRange.duration().value()); ++i)
            {
                videoRequests.push_back(
                    timeline->getVideo(otime::RationalTime(i, 24.0)));
            }
            io::Options ioOptions;
            ioOptions["Layer"] = "1";
            for (size_t i = 0;
                 i < static_cast<size_t>(timeRange.duration().value()); ++i)
            {
                videoRequests.push_back(timeline->getVideo(
                    otime::RationalTime(i, 24.0), ioOptions));
            }
            while (videoData.size() <
                   static_cast<size_t>(timeRange.duration().value()) * 2)
            {
                auto i = videoRequests.begin();
                while (i != videoRequests.end())
                {
                    if (i->future.valid() &&
                        i->future.wait_for(std::chrono::seconds(0)) ==
                            std::future_status::ready)
                    {
                        videoData.push_back(i->future.get());
                        i = videoRequests.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
            }
            TLRENDER_ASSERT(videoRequests.empty());

            // Get audio from the timeline.
            std::vector<timeline::AudioData> audioData;
            std::vector<timeline::AudioRequest> audioRequests;
            for (size_t i = 0;
                 i < static_cast<size_t>(
                         timeRange.duration().rescaled_to(1.0).value());
                 ++i)
            {
                audioRequests.push_back(timeline->getAudio(i));
            }
            while (audioData.size() <
                   static_cast<size_t>(
                       timeRange.duration().rescaled_to(1.0).value()))
            {
                auto i = audioRequests.begin();
                while (i != audioRequests.end())
                {
                    if (i->future.valid() &&
                        i->future.wait_for(std::chrono::seconds(0)) ==
                            std::future_status::ready)
                    {
                        audioData.push_back(i->future.get());
                        i = audioRequests.erase(i);
                    }
                    else
                    {
                        ++i;
                    }
                }
            }
            TLRENDER_ASSERT(audioRequests.empty());

            // Cancel requests.
            videoData.clear();
            videoRequests.clear();
            audioData.clear();
            audioRequests.clear();
            for (size_t i = 0;
                 i < static_cast<size_t>(timeRange.duration().value()); ++i)
            {
                videoRequests.push_back(
                    timeline->getVideo(otime::RationalTime(i, 24.0)));
            }
            for (size_t i = 0;
                 i < static_cast<size_t>(timeRange.duration().value()); ++i)
            {
                videoRequests.push_back(timeline->getVideo(
                    otime::RationalTime(i, 24.0), ioOptions));
            }
            for (size_t i = 0;
                 i < static_cast<size_t>(
                         timeRange.duration().rescaled_to(1.0).value());
                 ++i)
            {
                audioRequests.push_back(timeline->getAudio(i));
            }
            std::vector<uint64_t> ids;
            for (const auto& i : videoRequests)
            {
                ids.push_back(i.id);
            }
            for (const auto& i : audioRequests)
            {
                ids.push_back(i.id);
            }
            timeline->cancelRequests(ids);
        }

        void TimelineTest::_separateAudio()
        {
#if defined(TLRENDER_FFMPEG)
            try
            {
                const file::Path path(
                    TLRENDER_SAMPLE_DATA, "Seq/BART_2021-02-07.0001.jpg");
                const file::Path audioPath(
                    TLRENDER_SAMPLE_DATA, "BART_2021-02-07.m4v");
                auto timeline =
                    Timeline::create(path.get(), audioPath.get(), _context);
            }
            catch (const std::exception& e)
            {
                _printError(e.what());
            }
            try
            {
                const file::Path path(
                    TLRENDER_SAMPLE_DATA, "Seq/BART_2021-02-07.0001.jpg");
                const file::Path audioPath(
                    TLRENDER_SAMPLE_DATA, "BART_2021-02-07.m4v");
                auto timeline = Timeline::create(path, audioPath, _context);
            }
            catch (const std::exception& e)
            {
                _printError(e.what());
            }
            try
            {
                const file::Path path(
                    TLRENDER_SAMPLE_DATA, "Seq/BART_2021-02-07.0001.jpg");
                _print(string::Format("Path: {0}").arg(path.get()));
                Options options;
                options.fileSequenceAudio = FileSequenceAudio::None;
                auto timeline = Timeline::create(path, _context, options);
                const file::Path& audioPath = timeline->getAudioPath();
                TLRENDER_ASSERT(audioPath.isEmpty());
                _print(string::Format("Audio path: {0}").arg(audioPath.get()));
            }
            catch (const std::exception& e)
            {
                _printError(e.what());
            }
            try
            {
                const file::Path path(
                    TLRENDER_SAMPLE_DATA, "Seq/BART_2021-02-07.0001.jpg");
                _print(string::Format("Path: {0}").arg(path.get()));
                Options options;
                options.fileSequenceAudio = FileSequenceAudio::BaseName;
                auto timeline = Timeline::create(path, _context, options);
                const file::Path& audioPath = timeline->getAudioPath();
                TLRENDER_ASSERT(!audioPath.isEmpty());
                _print(string::Format("Audio path: {0}").arg(audioPath.get()));
            }
            catch (const std::exception& e)
            {
                _printError(e.what());
            }
            try
            {
                const file::Path path(
                    TLRENDER_SAMPLE_DATA, "Seq/BART_2021-02-07.0001.jpg");
                _print(string::Format("Path: {0}").arg(path.get()));
                Options options;
                options.fileSequenceAudio = FileSequenceAudio::FileName;
                options.fileSequenceAudioFileName =
                    file::Path(TLRENDER_SAMPLE_DATA, "AudioToneStereo.wav")
                        .get();
                auto timeline = Timeline::create(path, _context, options);
                const file::Path& audioPath = timeline->getAudioPath();
                TLRENDER_ASSERT(!audioPath.isEmpty());
                _print(string::Format("Audio path: {0}").arg(audioPath.get()));
            }
            catch (const std::exception& e)
            {
                _printError(e.what());
            }
            try
            {
                const file::Path path(
                    TLRENDER_SAMPLE_DATA, "Seq/BART_2021-02-07.0001.jpg");
                _print(string::Format("Path: {0}").arg(path.get()));
                Options options;
                options.fileSequenceAudio = FileSequenceAudio::Directory;
                options.fileSequenceAudioDirectory = "";
                auto timeline = Timeline::create(path, _context, options);
                const file::Path& audioPath = timeline->getAudioPath();
                TLRENDER_ASSERT(!audioPath.isEmpty());
                _print(string::Format("Audio path: {0}").arg(audioPath.get()));
            }
            catch (const std::exception& e)
            {
                _printError(e.what());
            }
#endif // TLRENDER_FFMPEG
        }

        void TimelineTest::_setTimeline()
        {
            auto timeline = Timeline::create(
                file::Path(TLRENDER_SAMPLE_DATA, "SingleClip.otio"), _context);
            auto otioTimeline = timeline::create(
                file::Path(TLRENDER_SAMPLE_DATA, "SingleClipSeq.otio"),
                _context);
            timeline->setTimeline(otioTimeline);
            TLRENDER_ASSERT(
                otioTimeline.value == timeline->getTimeline().value);
        }
    } // namespace timeline_tests
} // namespace tl
