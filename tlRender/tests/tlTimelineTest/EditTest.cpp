// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/EditTest.h>

#include <tlTimeline/Edit.h>
#include <tlTimeline/MemoryReference.h>
#include <tlTimeline/Util.h>

#include <tlCore/Assert.h>
#include <tlCore/Path.h>
#include <tlCore/StringFormat.h>

#include <opentimelineio/clip.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        EditTest::EditTest(const std::shared_ptr<system::Context>& context) :
            ITest("timeline_tests::EditTest", context)
        {
        }

        std::shared_ptr<EditTest>
        EditTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<EditTest>(new EditTest(context));
        }

        namespace
        {
            otio::SerializableObject::Retainer<otio::Composable> getChild(
                const otio::SerializableObject::Retainer<otio::Timeline>&
                    otioTimeline,
                int track, int index)
            {
                auto otioTrack = otio::dynamic_retainer_cast<otio::Track>(
                    otioTimeline->tracks()->children()[track]);
                return otioTrack->children()[index];
            }

            otio::SerializableObject::Retainer<otio::Clip> getClip(
                const otio::SerializableObject::Retainer<otio::Timeline>&
                    otioTimeline,
                int track, int index)
            {
                auto otioTrack = otio::dynamic_retainer_cast<otio::Track>(
                    otioTimeline->tracks()->children()[track]);
                return otio::dynamic_retainer_cast<otio::Clip>(
                    otioTrack->children()[index]);
            }
        } // namespace

        void EditTest::run()
        {
            _move();
        }

        void EditTest::_move()
        {
            {
                otio::SerializableObject::Retainer<otio::Timeline> otioTimeline(
                    new otio::Timeline);
                auto otioTrack = new otio::Track(
                    "Video", std::nullopt, otio::Track::Kind::video);
                otioTimeline->tracks()->append_child(otioTrack);
                otioTrack->append_child(new otio::Clip(
                    "Video 0", nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0))));

                MoveData moveData;
                moveData.fromTrack = 0;
                moveData.fromIndex = 0;
                moveData.fromOtioIndex = 0;
                moveData.toTrack = 0;
                moveData.toIndex = 0;
                moveData.toOtioIndex = 0;
                auto otioTimeline2 = move(otioTimeline, {moveData});
                TLRENDER_ASSERT(
                    "Video 0" == getChild(otioTimeline2, 0, 0)->name());

                moveData.fromTrack = 0;
                moveData.fromIndex = 0;
                moveData.fromOtioIndex = 0;
                moveData.toTrack = 0;
                moveData.toIndex = 1;
                moveData.toOtioIndex = 1;
                auto otioTimeline3 = move(otioTimeline2, {moveData});
                TLRENDER_ASSERT(
                    "Video 0" == getChild(otioTimeline3, 0, 0)->name());
            }
            {
                otio::SerializableObject::Retainer<otio::Timeline> otioTimeline(
                    new otio::Timeline);
                auto otioTrack = new otio::Track(
                    "Video", std::nullopt, otio::Track::Kind::video);
                otioTimeline->tracks()->append_child(otioTrack);
                otioTrack->append_child(new otio::Clip(
                    "Video 0", nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0))));
                otioTrack->append_child(new otio::Clip(
                    "Video 1", nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0))));

                MoveData moveData;
                moveData.fromTrack = 0;
                moveData.fromIndex = 0;
                moveData.fromOtioIndex = 0;
                moveData.toTrack = 0;
                moveData.toIndex = 2;
                moveData.toOtioIndex = 2;
                auto otioTimeline2 = move(otioTimeline, {moveData});
                TLRENDER_ASSERT(
                    "Video 1" == getChild(otioTimeline2, 0, 0)->name());
                TLRENDER_ASSERT(
                    "Video 0" == getChild(otioTimeline2, 0, 1)->name());

                moveData.fromTrack = 0;
                moveData.fromIndex = 1;
                moveData.toTrack = 0;
                moveData.toIndex = 0;
                auto otioTimeline3 = move(otioTimeline2, {moveData});
                TLRENDER_ASSERT(
                    "Video 0" == getChild(otioTimeline3, 0, 0)->name());
                TLRENDER_ASSERT(
                    "Video 1" == getChild(otioTimeline3, 0, 1)->name());
            }
            {
                otio::SerializableObject::Retainer<otio::Timeline> otioTimeline(
                    new otio::Timeline);
                auto otioTrack = new otio::Track(
                    "Video", std::nullopt, otio::Track::Kind::video);
                otioTimeline->tracks()->append_child(otioTrack);
                otioTrack->append_child(new otio::Clip(
                    "Video 0", nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0))));
                otioTrack->append_child(new otio::Clip(
                    "Video 1", nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0))));
                otioTrack->append_child(new otio::Clip(
                    "Video 2", nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0))));

                MoveData moveData;
                moveData.fromTrack = 0;
                moveData.fromIndex = 2;
                moveData.fromOtioIndex = 2;
                moveData.toTrack = 0;
                moveData.toIndex = 0;
                moveData.toOtioIndex = 0;
                auto otioTimeline2 = move(otioTimeline, {moveData});
                TLRENDER_ASSERT(
                    "Video 2" == getChild(otioTimeline2, 0, 0)->name());
                TLRENDER_ASSERT(
                    "Video 0" == getChild(otioTimeline2, 0, 1)->name());
                TLRENDER_ASSERT(
                    "Video 1" == getChild(otioTimeline2, 0, 2)->name());

                moveData.fromTrack = 0;
                moveData.fromIndex = 1;
                moveData.fromOtioIndex = 1;
                moveData.toTrack = 0;
                moveData.toIndex = 3;
                moveData.toOtioIndex = 3;
                auto otioTimeline3 = move(otioTimeline2, {moveData});
                TLRENDER_ASSERT(
                    "Video 2" == getChild(otioTimeline3, 0, 0)->name());
                TLRENDER_ASSERT(
                    "Video 1" == getChild(otioTimeline3, 0, 1)->name());
                TLRENDER_ASSERT(
                    "Video 0" == getChild(otioTimeline3, 0, 2)->name());
            }
            {
                otio::SerializableObject::Retainer<otio::Timeline> otioTimeline(
                    new otio::Timeline);
                auto otioTrack = new otio::Track(
                    "Video", std::nullopt, otio::Track::Kind::video);
                otioTimeline->tracks()->append_child(otioTrack);
                otioTrack->append_child(new otio::Clip(
                    "Video 0", nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0))));
                otioTrack->append_child(new otio::Clip(
                    "Video 1", nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0))));
                otioTrack->append_child(new otio::Clip(
                    "Video 2", nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0))));
                otioTrack = new otio::Track(
                    "Audio", std::nullopt, otio::Track::Kind::audio);
                otioTimeline->tracks()->append_child(otioTrack);
                otioTrack->append_child(new otio::Clip(
                    "Audio 0", nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 48000.0),
                        otime::RationalTime(48000.0, 48000.0))));
                otioTrack->append_child(new otio::Clip(
                    "Audio 1", nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 48000.0),
                        otime::RationalTime(48000.0, 48000.0))));
                otioTrack->append_child(new otio::Clip(
                    "Audio 2", nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 48000.0),
                        otime::RationalTime(48000.0, 48000.0))));

                std::vector<MoveData> moveData;
                moveData.push_back({0, 2, 2, 0, 0, 0});
                moveData.push_back({1, 2, 2, 1, 0, 0});
                auto otioTimeline2 = move(otioTimeline, moveData);
                TLRENDER_ASSERT(
                    "Video 2" == getChild(otioTimeline2, 0, 0)->name());
                TLRENDER_ASSERT(
                    "Video 0" == getChild(otioTimeline2, 0, 1)->name());
                TLRENDER_ASSERT(
                    "Video 1" == getChild(otioTimeline2, 0, 2)->name());
                TLRENDER_ASSERT(
                    "Audio 2" == getChild(otioTimeline2, 1, 0)->name());
                TLRENDER_ASSERT(
                    "Audio 0" == getChild(otioTimeline2, 1, 1)->name());
                TLRENDER_ASSERT(
                    "Audio 1" == getChild(otioTimeline2, 1, 2)->name());

                moveData.clear();
                moveData.push_back({0, 1, 1, 0, 3, 3});
                moveData.push_back({1, 1, 1, 1, 3, 3});
                auto otioTimeline3 = move(otioTimeline2, moveData);
                TLRENDER_ASSERT(
                    "Video 2" == getChild(otioTimeline3, 0, 0)->name());
                TLRENDER_ASSERT(
                    "Video 1" == getChild(otioTimeline3, 0, 1)->name());
                TLRENDER_ASSERT(
                    "Video 0" == getChild(otioTimeline3, 0, 2)->name());
                TLRENDER_ASSERT(
                    "Audio 2" == getChild(otioTimeline3, 1, 0)->name());
                TLRENDER_ASSERT(
                    "Audio 1" == getChild(otioTimeline3, 1, 1)->name());
                TLRENDER_ASSERT(
                    "Audio 0" == getChild(otioTimeline3, 1, 2)->name());
            }
            {
                otio::SerializableObject::Retainer<otio::Timeline> otioTimeline(
                    new otio::Timeline);
                auto otioTrack = new otio::Track(
                    "Video", std::nullopt, otio::Track::Kind::video);
                otioTimeline->tracks()->append_child(otioTrack);
                otioTrack->append_child(new otio::Clip(
                    "Video 0", nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0))));
                otioTrack = new otio::Track(
                    "Video", std::nullopt, otio::Track::Kind::video);
                otioTimeline->tracks()->append_child(otioTrack);

                MoveData moveData;
                moveData.fromTrack = 0;
                moveData.fromIndex = 0;
                moveData.fromOtioIndex = 0;
                moveData.toTrack = 1;
                moveData.toIndex = 0;
                moveData.toOtioIndex = 0;
                auto otioTimeline2 = move(otioTimeline, {moveData});
                TLRENDER_ASSERT(
                    "Video 0" == getChild(otioTimeline2, 1, 0)->name());

                moveData.fromTrack = 1;
                moveData.fromIndex = 0;
                moveData.fromOtioIndex = 0;
                moveData.toTrack = 0;
                moveData.toIndex = 0;
                moveData.toOtioIndex = 0;
                auto otioTimeline3 = move(otioTimeline2, {moveData});
                TLRENDER_ASSERT(
                    "Video 0" == getChild(otioTimeline3, 0, 0)->name());
            }
            for (const auto otio : {"SingleClip.otio", "SingleClipSeq.otio"})
            {
                for (const auto toMemoryReference :
                     {ToMemoryReference::Shared, ToMemoryReference::Raw})
                {
                    auto otioTimeline = timeline::create(
                        file::Path(TLRENDER_SAMPLE_DATA, otio), _context);
                    auto track = dynamic_cast<otio::Track*>(
                        otioTimeline->tracks()->children()[0].value);
                    track->append_child(new otio::Clip(
                        "Video", nullptr,
                        otime::TimeRange(
                            otime::RationalTime(0.0, 30.0),
                            otime::RationalTime(30.0, 30.0))));
                    toMemoryReferences(
                        otioTimeline.value, TLRENDER_SAMPLE_DATA,
                        toMemoryReference);

                    const std::string video0 =
                        getChild(otioTimeline, 0, 0)->name();
                    const std::string video1 =
                        getChild(otioTimeline, 0, 1)->name();

                    MoveData moveData;
                    moveData.fromTrack = 0;
                    moveData.fromIndex = 0;
                    moveData.fromOtioIndex = 0;
                    moveData.toTrack = 0;
                    moveData.toIndex = 2;
                    moveData.toOtioIndex = 2;
                    auto otioTimeline2 = move(otioTimeline, {moveData});
                    TLRENDER_ASSERT(
                        video1 == getChild(otioTimeline2, 0, 0)->name());
                    TLRENDER_ASSERT(
                        video0 == getChild(otioTimeline2, 0, 1)->name());

                    moveData.fromTrack = 0;
                    moveData.fromIndex = 1;
                    moveData.fromOtioIndex = 1;
                    moveData.toTrack = 0;
                    moveData.toIndex = 0;
                    moveData.toOtioIndex = 0;
                    auto otioTimeline3 = move(otioTimeline2, {moveData});
                    TLRENDER_ASSERT(
                        video0 == getChild(otioTimeline3, 0, 0)->name());
                    TLRENDER_ASSERT(
                        video1 == getChild(otioTimeline3, 0, 1)->name());

                    if (ToMemoryReference::Raw == toMemoryReference)
                    {
                        for (const auto clip : otioTimeline->find_clips())
                        {
                            if (const auto ref =
                                    dynamic_cast<RawMemoryReference*>(
                                        clip->media_reference()))
                            {
                                delete[] ref->memory();
                                ref->set_memory(nullptr, 0);
                            }
                            else if (
                                const auto ref =
                                    dynamic_cast<RawMemorySequenceReference*>(
                                        clip->media_reference()))
                            {
                                for (const auto& memory : ref->memory())
                                {
                                    delete[] memory;
                                }
                                ref->set_memory({}, {});
                            }
                        }
                    }
                }
            }
            for (const auto otioz : {"SingleClip.otioz", "SingleClipSeq.otioz"})
            {
                auto otioTimeline = timeline::create(
                    file::Path(TLRENDER_SAMPLE_DATA, otioz), _context);
                auto track = dynamic_cast<otio::Track*>(
                    otioTimeline->tracks()->children()[0].value);
                track->append_child(new otio::Clip(
                    "Video", nullptr,
                    otime::TimeRange(
                        otime::RationalTime(0.0, 30.0),
                        otime::RationalTime(30.0, 30.0))));

                const std::string video0 = getChild(otioTimeline, 0, 0)->name();
                const std::string video1 = getChild(otioTimeline, 0, 1)->name();

                MoveData moveData;
                moveData.fromTrack = 0;
                moveData.fromIndex = 0;
                moveData.fromOtioIndex = 0;
                moveData.toTrack = 0;
                moveData.toIndex = 2;
                moveData.toOtioIndex = 2;
                auto otioTimeline2 = move(otioTimeline, {moveData});
                TLRENDER_ASSERT(
                    video1 == getChild(otioTimeline2, 0, 0)->name());
                TLRENDER_ASSERT(
                    video0 == getChild(otioTimeline2, 0, 1)->name());

                moveData.fromTrack = 0;
                moveData.fromIndex = 1;
                moveData.fromOtioIndex = 1;
                moveData.toTrack = 0;
                moveData.toIndex = 0;
                moveData.toOtioIndex = 0;
                auto otioTimeline3 = move(otioTimeline2, {moveData});
                TLRENDER_ASSERT(
                    video0 == getChild(otioTimeline3, 0, 0)->name());
                TLRENDER_ASSERT(
                    video1 == getChild(otioTimeline3, 0, 1)->name());
            }
        }
    } // namespace timeline_tests
} // namespace tl
