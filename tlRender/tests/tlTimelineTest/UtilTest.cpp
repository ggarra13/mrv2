// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/UtilTest.h>

#include <tlTimeline/Util.h>

#include <tlCore/Assert.h>
#include <tlCore/FileInfo.h>
#include <tlCore/StringFormat.h>

#include <opentimelineio/clip.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        UtilTest::UtilTest(const std::shared_ptr<system::Context>& context) :
            ITest("timeline_tests::UtilTest", context)
        {
        }

        std::shared_ptr<UtilTest>
        UtilTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<UtilTest>(new UtilTest(context));
        }

        void UtilTest::run()
        {
            _enums();
            _extensions();
            _ranges();
            _util();
            _otioz();
        }

        void UtilTest::_enums()
        {
            _enum<CacheDirection>("CacheDirection", getCacheDirectionEnums);
            _enum<ToMemoryReference>(
                "ToMemoryReference", getToMemoryReferenceEnums);
        }

        void UtilTest::_extensions()
        {
            for (const auto& i : getExtensions(
                     static_cast<int>(io::FileType::Movie) |
                         static_cast<int>(io::FileType::Sequence) |
                         static_cast<int>(io::FileType::Audio),
                     _context))
            {
                std::stringstream ss;
                ss << "Timeline extension: " << i;
                _print(ss.str());
            }
            for (const auto& path : getPaths(
                     file::Path(TLRENDER_SAMPLE_DATA), file::PathOptions(),
                     _context))
            {
                _print(string::Format("Path: {0}").arg(path.get()));
            }
        }

        void UtilTest::_ranges()
        {
            {
                std::vector<otime::RationalTime> f;
                auto r = toRanges(f);
                TLRENDER_ASSERT(r.empty());
            }
            {
                std::vector<otime::RationalTime> f = {
                    otime::RationalTime(0, 24)};
                auto r = toRanges(f);
                TLRENDER_ASSERT(1 == r.size());
                TLRENDER_ASSERT(
                    otime::TimeRange(
                        otime::RationalTime(0, 24),
                        otime::RationalTime(1, 24)) == r[0]);
            }
            {
                std::vector<otime::RationalTime> f = {
                    otime::RationalTime(0, 24), otime::RationalTime(1, 24)};
                auto r = toRanges(f);
                TLRENDER_ASSERT(1 == r.size());
                TLRENDER_ASSERT(
                    otime::TimeRange(
                        otime::RationalTime(0, 24),
                        otime::RationalTime(2, 24)) == r[0]);
            }
            {
                std::vector<otime::RationalTime> f = {
                    otime::RationalTime(0, 24), otime::RationalTime(1, 24),
                    otime::RationalTime(2, 24)};
                auto r = toRanges(f);
                TLRENDER_ASSERT(1 == r.size());
                TLRENDER_ASSERT(
                    otime::TimeRange(
                        otime::RationalTime(0, 24),
                        otime::RationalTime(3, 24)) == r[0]);
            }
            {
                std::vector<otime::RationalTime> f = {
                    otime::RationalTime(0, 24), otime::RationalTime(2, 24)};
                auto r = toRanges(f);
                TLRENDER_ASSERT(2 == r.size());
                TLRENDER_ASSERT(
                    otime::TimeRange(
                        otime::RationalTime(0, 24),
                        otime::RationalTime(1, 24)) == r[0]);
                TLRENDER_ASSERT(
                    otime::TimeRange(
                        otime::RationalTime(2, 24),
                        otime::RationalTime(1, 24)) == r[1]);
            }
            {
                std::vector<otime::RationalTime> f = {
                    otime::RationalTime(0, 24), otime::RationalTime(1, 24),
                    otime::RationalTime(3, 24)};
                auto r = toRanges(f);
                TLRENDER_ASSERT(2 == r.size());
                TLRENDER_ASSERT(
                    otime::TimeRange(
                        otime::RationalTime(0, 24),
                        otime::RationalTime(2, 24)) == r[0]);
                TLRENDER_ASSERT(
                    otime::TimeRange(
                        otime::RationalTime(3, 24),
                        otime::RationalTime(1, 24)) == r[1]);
            }
            {
                std::vector<otime::RationalTime> f = {
                    otime::RationalTime(0, 24), otime::RationalTime(1, 24),
                    otime::RationalTime(3, 24), otime::RationalTime(4, 24)};
                auto r = toRanges(f);
                TLRENDER_ASSERT(2 == r.size());
                TLRENDER_ASSERT(
                    otime::TimeRange(
                        otime::RationalTime(0, 24),
                        otime::RationalTime(2, 24)) == r[0]);
                TLRENDER_ASSERT(
                    otime::TimeRange(
                        otime::RationalTime(3, 24),
                        otime::RationalTime(2, 24)) == r[1]);
            }
        }

        void UtilTest::_util()
        {
            {
                auto otioClip = new otio::Clip;
                otio::ErrorStatus errorStatus;
                auto otioTrack = new otio::Track();
                otioTrack->append_child(otioClip, &errorStatus);
                if (otio::is_error(errorStatus))
                {
                    throw std::runtime_error("Cannot append child");
                }
                auto otioStack = new otio::Stack;
                otioStack->append_child(otioTrack, &errorStatus);
                if (otio::is_error(errorStatus))
                {
                    throw std::runtime_error("Cannot append child");
                }
                otio::SerializableObject::Retainer<otio::Timeline> otioTimeline(
                    new otio::Timeline);
                otioTimeline->set_tracks(otioStack);
                TLRENDER_ASSERT(otioStack == getRoot(otioClip));
                TLRENDER_ASSERT(otioStack == getParent<otio::Stack>(otioClip));
                TLRENDER_ASSERT(otioTrack == getParent<otio::Track>(otioClip));
            }
            {
                VideoData a;
                a.time = otime::RationalTime(1.0, 24.0);
                VideoData b;
                b.time = otime::RationalTime(1.0, 24.0);
                TLRENDER_ASSERT(isTimeEqual(a, b));
            }
        }

        void UtilTest::_otioz()
        {
            std::vector<file::FileInfo> list;
            file::list(TLRENDER_SAMPLE_DATA, list);
            for (const auto& entry : list)
            {
                if (".otio" == entry.getPath().getExtension())
                {
                    otio::SerializableObject::Retainer<otio::Timeline> timeline(
                        dynamic_cast<otio::Timeline*>(
                            otio::Timeline::from_json_file(
                                entry.getPath().get())));
                    file::Path outputPath = entry.getPath();
                    outputPath.setExtension(".otioz");
                    writeOTIOZ(
                        outputPath.get(-1, file::PathType::FileName), timeline,
                        TLRENDER_SAMPLE_DATA);
                }
            }
        }
    } // namespace timeline_tests
} // namespace tl
