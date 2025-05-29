// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/CompareOptionsTest.h>

#include <tlTimeline/CompareOptions.h>

#include <tlCore/Assert.h>
#include <tlCore/String.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        CompareOptionsTest::CompareOptionsTest(
            const std::shared_ptr<system::Context>& context) :
            ITest("timeline_tests::CompareOptionsTest", context)
        {
        }

        std::shared_ptr<CompareOptionsTest> CompareOptionsTest::create(
            const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<CompareOptionsTest>(
                new CompareOptionsTest(context));
        }

        void CompareOptionsTest::run()
        {
            {
                _enum<CompareMode>("CompareMode", getCompareModeEnums);
                _enum<CompareTimeMode>(
                    "CompareTimeMode", getCompareTimeModeEnums);
            }
            {
                CompareOptions options;
                options.mode = CompareMode::B;
                TLRENDER_ASSERT(options == options);
                TLRENDER_ASSERT(options != CompareOptions());
            }
            {
                const std::vector<image::Size> sizes = {
                    image::Size(1920, 1080), image::Size(1920 / 2, 1080 / 2),
                    image::Size(1920 / 2, 1080 / 2),
                    image::Size(1920 / 2, 1080 / 2)};

                for (auto mode :
                     {CompareMode::A, CompareMode::B, CompareMode::Wipe,
                      CompareMode::Overlay, CompareMode::Difference})
                {
                    auto boxes = getBoxes(mode, sizes);
                    TLRENDER_ASSERT(2 == boxes.size());
                    TLRENDER_ASSERT(math::Box2i(0, 0, 1920, 1080) == boxes[0]);
                    TLRENDER_ASSERT(math::Box2i(0, 0, 1920, 1080) == boxes[1]);
                    auto renderSize = getRenderSize(mode, sizes);
                    TLRENDER_ASSERT(math::Size2i(1920, 1080) == renderSize);
                }

                auto boxes = getBoxes(CompareMode::Horizontal, sizes);
                TLRENDER_ASSERT(2 == boxes.size());
                TLRENDER_ASSERT(math::Box2i(0, 0, 1920, 1080) == boxes[0]);
                TLRENDER_ASSERT(math::Box2i(1920, 0, 1920, 1080) == boxes[1]);
                auto renderSize = getRenderSize(CompareMode::Horizontal, sizes);
                TLRENDER_ASSERT(math::Size2i(1920 * 2, 1080) == renderSize);

                boxes = getBoxes(CompareMode::Vertical, sizes);
                TLRENDER_ASSERT(2 == boxes.size());
                TLRENDER_ASSERT(math::Box2i(0, 0, 1920, 1080) == boxes[0]);
                TLRENDER_ASSERT(math::Box2i(0, 1080, 1920, 1080) == boxes[1]);
                renderSize = getRenderSize(CompareMode::Vertical, sizes);
                TLRENDER_ASSERT(math::Size2i(1920, 1080 * 2) == renderSize);

                boxes = getBoxes(CompareMode::Tile, sizes);
                TLRENDER_ASSERT(4 == boxes.size());
                TLRENDER_ASSERT(math::Box2i(0, 0, 1920, 1080) == boxes[0]);
                TLRENDER_ASSERT(math::Box2i(1920, 0, 1920, 1080) == boxes[1]);
                TLRENDER_ASSERT(math::Box2i(0, 1080, 1920, 1080) == boxes[2]);
                TLRENDER_ASSERT(
                    math::Box2i(1920, 1080, 1920, 1080) == boxes[3]);
                renderSize = getRenderSize(CompareMode::Tile, sizes);
                TLRENDER_ASSERT(math::Size2i(1920 * 2, 1080 * 2) == renderSize);
            }
            {
                const auto time = getCompareTime(
                    otime::RationalTime(0.0, 24.0),
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0)),
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0)),
                    CompareTimeMode::Absolute);
                TLRENDER_ASSERT(time == otime::RationalTime(0.0, 24.0));
            }
            {
                const auto time = getCompareTime(
                    otime::RationalTime(0.0, 24.0),
                    otime::TimeRange(
                        otime::RationalTime(0.0, 24.0),
                        otime::RationalTime(24.0, 24.0)),
                    otime::TimeRange(
                        otime::RationalTime(24.0, 24.0),
                        otime::RationalTime(24.0, 24.0)),
                    CompareTimeMode::Relative);
                TLRENDER_ASSERT(time == otime::RationalTime(24.0, 24.0));
            }
        }
    } // namespace timeline_tests
} // namespace tl
