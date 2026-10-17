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
        using namespace tl::timeline;

        CompareOptionsTest::CompareOptionsTest(const std::shared_ptr<system::Context>& context) :
            ITest("timeline_tests::CompareOptionsTest", context)
        {}

        std::shared_ptr<CompareOptionsTest> CompareOptionsTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<CompareOptionsTest>(new CompareOptionsTest(context));
        }

        void CompareOptionsTest::run()
        {
            {
                // FTK_TEST_ENUM(Compare);
                // FTK_TEST_ENUM(CompareTime);
            }
            {
                timeline::CompareOptions options;
                options.mode = CompareMode::B;
                TLRENDER_ASSERT(options == options);
                TLRENDER_ASSERT(options != CompareOptions());
            }
            {
                // \@bug:
                //
                // // Every field survives being written and read back.
                // timeline::CompareOptions options;
                // options.mode = CompareMode::Difference;
                // options.wipeCenter = math::Vector2f(.25F, .75F);
                // options.wipeRotation = 90.F;
                // options.overlay = .25F;
                // options.differenceGain = 8.F;
                // options.fitToA = false;
                // nlohmann::json json;
                // to_json(json, options);
                // timeline::CompareOptions options2;
                // from_json(json, options2);
                // TLRENDER_ASSERT(options == options2);

                // // A file written before a field existed keeps the default
                // // for it rather than throwing and losing the rest.
                // json.erase("DifferenceGain");
                // timeline::CompareOptions options3;
                // from_json(json, options3);
                // TLRENDER_ASSERT(CompareOptions().differenceGain == options3.differenceGain);
                // TLRENDER_ASSERT(options.overlay == options3.overlay);
            }
            {
                const std::vector<image::Info> infos =
                {
                    image::Info(1920, 1080, image::PixelType::RGBA_U8),
                    image::Info(1920 / 2, 1080 / 2, image::PixelType::RGBA_U8),
                    image::Info(1920 / 2, 1080 / 2, image::PixelType::RGBA_U8),
                    image::Info(1920 / 2, 1080 / 2, image::PixelType::RGBA_U8)
                };

                for (auto compare :
                    {
                        timeline::CompareMode::A,
                        timeline::CompareMode::B,
                        timeline::CompareMode::Wipe,
                        timeline::CompareMode::Overlay,
                        timeline::CompareMode::Difference
                    })
                {
                    auto boxes = getBoxes({ compare },
                                          AspectRatioOptions(), infos);
                    TLRENDER_ASSERT(!boxes.empty());
                    TLRENDER_ASSERT(math::Box2i(0, 0, 1920, 1080) == boxes[0]);
                    TLRENDER_ASSERT(math::Box2i(0, 0, 1920, 1080) == boxes[1]);
                    auto renderSize = getRenderSize({ compare }, AspectRatioOptions(), infos);
                    TLRENDER_ASSERT(math::Size2i(1920, 1080) == renderSize);
                }

                // The boxes exist for every source; isShown() says which
                // of them the mode draws.
                TLRENDER_ASSERT(isShown(timeline::CompareMode::A, 0));
                TLRENDER_ASSERT(!isShown(timeline::CompareMode::A, 1));
                TLRENDER_ASSERT(!isShown(timeline::CompareMode::B, 0));
                TLRENDER_ASSERT(isShown(timeline::CompareMode::B, 1));
                TLRENDER_ASSERT(!isShown(timeline::CompareMode::B, 2));
                for (auto compare :
                    {
                        timeline::CompareMode::Wipe,
                        timeline::CompareMode::Overlay,
                        timeline::CompareMode::Difference,
                        timeline::CompareMode::Horizontal,
                        timeline::CompareMode::Vertical,
                        timeline::CompareMode::Tile
                    })
                {
                    TLRENDER_ASSERT(isShown(compare, 0));
                    TLRENDER_ASSERT(isShown(compare, 1));
                    TLRENDER_ASSERT(isShown(compare, 2));
                }

                auto boxes = timeline::getBoxes({ timeline::CompareMode::Horizontal }, AspectRatioOptions(), infos);
                TLRENDER_ASSERT(2 == boxes.size());
                TLRENDER_ASSERT(math::Box2i(0, 0, 1920, 1080) == boxes[0]);
                TLRENDER_ASSERT(math::Box2i(1920, 0, 1920, 1080) == boxes[1]);
                auto renderSize = getRenderSize({ CompareMode::Horizontal }, AspectRatioOptions(), infos);
                TLRENDER_ASSERT(math::Size2i(1920 * 2, 1080) == renderSize);

                boxes = timeline::getBoxes({ timeline::CompareMode::Vertical }, AspectRatioOptions(), infos);
                TLRENDER_ASSERT(2 == boxes.size());
                TLRENDER_ASSERT(math::Box2i(0, 0, 1920, 1080) == boxes[0]);
                TLRENDER_ASSERT(math::Box2i(0, 1080, 1920, 1080) == boxes[1]);
                renderSize = getRenderSize({ timeline::CompareMode::Vertical }, AspectRatioOptions(), infos);
                TLRENDER_ASSERT(math::Size2i(1920, 1080 * 2) == renderSize);

                boxes = timeline::getBoxes({ timeline::CompareMode::Tile }, AspectRatioOptions(), infos);
                TLRENDER_ASSERT(4 == boxes.size());
                TLRENDER_ASSERT(math::Box2i(0, 0, 1920, 1080) == boxes[0]);
                TLRENDER_ASSERT(math::Box2i(1920, 0, 1920, 1080) == boxes[1]);
                TLRENDER_ASSERT(math::Box2i(0, 1080, 1920, 1080) == boxes[2]);
                TLRENDER_ASSERT(math::Box2i(1920, 1080, 1920, 1080) == boxes[3]);
                renderSize = getRenderSize({ timeline::CompareMode::Tile }, AspectRatioOptions(), infos);
                TLRENDER_ASSERT(math::Size2i(1920 * 2, 1080 * 2) == renderSize);
            }
            {
                // Side by side, the second is as tall as the first, and one
                // above the other it is as wide, whichever of them is wider:
                // an anamorphic picture beside a spherical one, both ways
                // around, and with the pixels taken as square.
                image::Info anamorphic(64, 36, image::PixelType::RGBA_U8);
                anamorphic.size.pixelAspectRatio = 2.F;
                const image::Info spherical(1920, 1080,
                                            image::PixelType::RGBA_U8);

                auto boxes = getBoxes(
                    { timeline::CompareMode::Horizontal },
                    AspectRatioOptions(),
                    { anamorphic, spherical });
                TLRENDER_ASSERT(math::Box2i(0, 0, 128, 36) == boxes[0]);
                TLRENDER_ASSERT(math::Box2i(128, 0, 64, 36) == boxes[1]);

                boxes = timeline::getBoxes(
                    { timeline::CompareMode::Horizontal },
                    AspectRatioOptions(),
                    { spherical, anamorphic });
                TLRENDER_ASSERT(math::Box2i(0, 0, 1920, 1080) == boxes[0]);
                TLRENDER_ASSERT(math::Box2i(1920, 0, 3840, 1080) == boxes[1]);

                const AspectRatioOptions square(AspectRatio(1.F), AspectRatioType::Pixel);
                boxes = timeline::getBoxes(
                    { timeline::CompareMode::Horizontal },
                    square,
                    { anamorphic, spherical });
                TLRENDER_ASSERT(math::Box2i(0, 0, 64, 36) == boxes[0]);
                TLRENDER_ASSERT(math::Box2i(64, 0, 64, 36) == boxes[1]);

                boxes = timeline::getBoxes(
                    { timeline::CompareMode::Vertical },
                    AspectRatioOptions(),
                    { spherical, anamorphic });
                TLRENDER_ASSERT(math::Box2i(0, 0, 1920, 1080) == boxes[0]);
                TLRENDER_ASSERT(math::Box2i(0, 1080, 1920, 540) == boxes[1]);

                boxes = timeline::getBoxes(
                    { timeline::CompareMode::Vertical },
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
                    timeline::CompareTimeMode::Absolute);
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
                    timeline::CompareTimeMode::Relative);
                TLRENDER_ASSERT(time == OTIO_NS::RationalTime(24.0, 24.0));
            }
        }
    }
}
