// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/ImageOptionsTest.h>

#include <tlTimeline/ImageOptions.h>

#include <tlCore/Assert.h>
#include <tlCore/String.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        ImageOptionsTest::ImageOptionsTest(
            const std::shared_ptr<system::Context>& context) :
            ITest("timeline_tests::ImageOptionsTest", context)
        {
        }

        std::shared_ptr<ImageOptionsTest> ImageOptionsTest::create(
            const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<ImageOptionsTest>(
                new ImageOptionsTest(context));
        }

        void ImageOptionsTest::run()
        {
            {
                _enum<InputVideoLevels>(
                    "InputVideoLevels", getInputVideoLevelsEnums);
                _enum<AlphaBlend>("AlphaBlend", getAlphaBlendEnums);
                _enum<ImageFilter>("ImageFilter", getImageFilterEnums);
            }
            {
                ImageFilters v;
                v.minify = ImageFilter::Nearest;
                TLRENDER_ASSERT(v == v);
                TLRENDER_ASSERT(v != ImageFilters());
            }
            {
                ImageOptions v;
                v.videoLevels = InputVideoLevels::FullRange;
                TLRENDER_ASSERT(v == v);
                TLRENDER_ASSERT(v != ImageOptions());
            }
        }
    } // namespace timeline_tests
} // namespace tl
