// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlTimelineTest/CompareOptionsTest.h>

#include <tlTimeline/CompareOptions.h>

#include <tlCore/Assert.h>
#include <tlCore/StringFormat.h>
#include <tlCore/String.h>

namespace tl
{
    namespace timeline_tests
    {
        CompareOptionsTest::CompareOptionsTest(const std::shared_ptr<system::Context>& context) :
            ITest(context, "timeline_tests::CompareOptionsTest")
        {}

        std::shared_ptr<CompareOptionsTest> CompareOptionsTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<CompareOptionsTest>(new CompareOptionsTest(context));
        }

        void CompareOptionsTest::run()
        {
            // \@todo:
            // {
            //     FTK_TEST_ENUM(Compare);
            //     FTK_TEST_ENUM(CompareTime);
            // }
            {
                CompareOptions options;
                options.compare = Compare::B;
                TLRENDER_ASSERT(options == options);
                TLRENDER_ASSERT(options != CompareOptions());
            }
            {
                // Every field survives being written and read back.
                CompareOptions options;
                options.compare = Compare::Difference;
                options.wipeCenter = math::Vector2f(.25F, .75F);
                options.wipeRotation = 90.F;
                options.overlay = .25F;
                options.differenceGain = 8.F;
                options.sameSize = false;
                nlohmann::json json;
                to_json(json, options);
                CompareOptions options2;
                from_json(json, options2);
                TLRENDER_ASSERT(options == options2);

                // A file written before a field existed keeps the default
                // for it rather than throwing and losing the rest.
                json.erase("DifferenceGain");
                CompareOptions options3;
                from_json(json, options3);
                TLRENDER_ASSERT(CompareOptions().differenceGain == options3.differenceGain);
                TLRENDER_ASSERT(options.overlay == options3.overlay);
            }
            {
                const std::vector<image::Info> infos =
                {
                    image::Info(1920, 1080, image::ImageType::RGBA_U8),
                    image::Info(1920 / 2, 1080 / 2, image::ImageType::RGBA_U8),
                    image::Info(1920 / 2, 1080 / 2, image::ImageType::RGBA_U8),
                    image::Info(1920 / 2, 1080 / 2, image::ImageType::RGBA_U8)
                };

                for (auto compare :
                    {
                        Compare::None,
                        Compare::B,
                        Compare::Wipe,
                        Compare::Overlay,
                        Compare::Difference
                    })
                {
                    auto boxes = getBoxes({ compare }, AspectRatioOptions(), infos);
                    TLRENDER_ASSERT(!boxes.empty());
                    TLRENDER_ASSERT(math::Box2i(0, 0, 1920, 1080) == boxes[0]);
                    TLRENDER_ASSERT(math::Box2i(0, 0, 1920, 1080) == boxes[1]);
                    auto renderSize = getRenderSize({ compare }, AspectRatioOptions(), infos);
                    TLRENDER_ASSERT(math::Size2i(1920, 1080) == renderSize);
                }

                // The boxes exist for every source; isShown() says which
                // of them the mode draws.
                TLRENDER_ASSERT(isShown(Compare::None, 0));
                TLRENDER_ASSERT(!isShown(Compare::None, 1));
                TLRENDER_ASSERT(!isShown(Compare::B, 0));
                TLRENDER_ASSERT(isShown(Compare::B, 1));
                TLRENDER_ASSERT(!isShown(Compare::B, 2));
                for (auto compare :
                    {
                        Compare::Wipe,
                        Compare::Overlay,
                        Compare::Difference,
                        Compare::Horizontal,
                        Compare::Vertical,
                        Compare::Tile
                    })
                {
                    TLRENDER_ASSERT(isShown(compare, 0));
                    TLRENDER_ASSERT(isShown(compare, 1));
                    TLRENDER_ASSERT(isShown(compare, 2));
                }

                auto boxes = getBoxes({ Compare::Horizontal }, AspectRatioOptions(), infos);
                TLRENDER_ASSERT(2 == boxes.size());
                TLRENDER_ASSERT(math::Box2i(0, 0, 1920, 1080) == boxes[0]);
                TLRENDER_ASSERT(math::Box2i(1920, 0, 1920, 1080) == boxes[1]);
                auto renderSize = getRenderSize({ Compare::Horizontal }, AspectRatioOptions(), infos);
                TLRENDER_ASSERT(math::Size2i(1920 * 2, 1080) == renderSize);

                boxes = getBoxes({ Compare::Vertical }, AspectRatioOptions(), infos);
                TLRENDER_ASSERT(2 == boxes.size());
                TLRENDER_ASSERT(math::Box2i(0, 0, 1920, 1080) == boxes[0]);
                TLRENDER_ASSERT(math::Box2i(0, 1080, 1920, 1080) == boxes[1]);
                renderSize = getRenderSize({ Compare::Vertical }, AspectRatioOptions(), infos);
                TLRENDER_ASSERT(math::Size2i(1920, 1080 * 2) == renderSize);

                boxes = getBoxes({ Compare::Tile }, AspectRatioOptions(), infos);
                TLRENDER_ASSERT(4 == boxes.size());
                TLRENDER_ASSERT(math::Box2i(0, 0, 1920, 1080) == boxes[0]);
                TLRENDER_ASSERT(math::Box2i(1920, 0, 1920, 1080) == boxes[1]);
                TLRENDER_ASSERT(math::Box2i(0, 1080, 1920, 1080) == boxes[2]);
                TLRENDER_ASSERT(math::Box2i(1920, 1080, 1920, 1080) == boxes[3]);
                renderSize = getRenderSize({ Compare::Tile }, AspectRatioOptions(), infos);
                TLRENDER_ASSERT(math::Size2i(1920 * 2, 1080 * 2) == renderSize);
            }
            {
                // Side by side, the second is as tall as the first, and one
                // above the other it is as wide, whichever of them is wider:
                // an anamorphic picture beside a spherical one, both ways
                // around, and with the pixels taken as square.
                image::Info anamorphic(64, 36, image::ImageType::RGBA_U8);
                anamorphic.pixelAspectRatio = 2.F;
                const image::Info spherical(1920, 1080, image::ImageType::RGBA_U8);

                auto boxes = getBoxes(
                    { Compare::Horizontal },
                    AspectRatioOptions(),
                    { anamorphic, spherical });
                TLRENDER_ASSERT(math::Box2i(0, 0, 128, 36) == boxes[0]);
                TLRENDER_ASSERT(math::Box2i(128, 0, 64, 36) == boxes[1]);

                boxes = getBoxes(
                    { Compare::Horizontal },
                    AspectRatioOptions(),
                    { spherical, anamorphic });
                TLRENDER_ASSERT(math::Box2i(0, 0, 1920, 1080) == boxes[0]);
                TLRENDER_ASSERT(math::Box2i(1920, 0, 3840, 1080) == boxes[1]);

                const AspectRatioOptions square(AspectRatio(1.F), AspectRatioType::Pixel);
                boxes = getBoxes(
                    { Compare::Horizontal },
                    square,
                    { anamorphic, spherical });
                TLRENDER_ASSERT(math::Box2i(0, 0, 64, 36) == boxes[0]);
                TLRENDER_ASSERT(math::Box2i(64, 0, 64, 36) == boxes[1]);

                boxes = getBoxes(
                    { Compare::Vertical },
                    AspectRatioOptions(),
                    { spherical, anamorphic });
                TLRENDER_ASSERT(math::Box2i(0, 0, 1920, 1080) == boxes[0]);
                TLRENDER_ASSERT(math::Box2i(0, 1080, 1920, 540) == boxes[1]);

                boxes = getBoxes(
                    { Compare::Vertical },
                    AspectRatioOptions(),
                    { anamorphic, spherical });
                TLRENDER_ASSERT(math::Box2i(0, 0, 128, 36) == boxes[0]);
                TLRENDER_ASSERT(math::Box2i(0, 36, 128, 72) == boxes[1]);
            }
            {
                const auto time = getCompareTime(
                    OTIO_NS::RationalTime(0.0, 24.0),
                    OTIO_NS::TimeRange(
                        OTIO_NS::RationalTime(0.0, 24.0),
                        OTIO_NS::RationalTime(24.0, 24.0)),
                    OTIO_NS::TimeRange(
                        OTIO_NS::RationalTime(0.0, 24.0),
                        OTIO_NS::RationalTime(24.0, 24.0)),
                    CompareTime::Absolute);
                TLRENDER_ASSERT(time == OTIO_NS::RationalTime(0.0, 24.0));
            }
            {
                const auto time = getCompareTime(
                    OTIO_NS::RationalTime(0.0, 24.0),
                    OTIO_NS::TimeRange(
                        OTIO_NS::RationalTime(0.0, 24.0),
                        OTIO_NS::RationalTime(24.0, 24.0)),
                    OTIO_NS::TimeRange(
                        OTIO_NS::RationalTime(24.0, 24.0),
                        OTIO_NS::RationalTime(24.0, 24.0)),
                    CompareTime::Relative);
                TLRENDER_ASSERT(time == OTIO_NS::RationalTime(24.0, 24.0));
            }
        }
    }
}
