// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/DisplayOptionsTest.h>

#include <tlTimeline/DisplayOptions.h>

#include <tlCore/Assert.h>
#include <tlCore/String.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        DisplayOptionsTest::DisplayOptionsTest(
            const std::shared_ptr<system::Context>& context) :
            ITest("timeline_tests::DisplayOptionsTest", context)
        {
        }

        std::shared_ptr<DisplayOptionsTest> DisplayOptionsTest::create(
            const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<DisplayOptionsTest>(
                new DisplayOptionsTest(context));
        }

        void DisplayOptionsTest::run()
        {
            {
                _enum<Channels>("Channels", getChannelsEnums);
            }
            {
                Color color;
                color.enabled = true;
                TLRENDER_ASSERT(color == color);
                TLRENDER_ASSERT(color != Color());
            }
            {
                const auto mat = brightness(math::Vector3f(2.F, 1.F, 1.F));
                const auto vec = mat * math::Vector3f(1.F, 1.F, 1.F);
            }
            {
                const auto mat = contrast(math::Vector3f(2.F, 1.F, 1.F));
                const auto vec = mat * math::Vector3f(1.F, 1.F, 1.F);
            }
            {
                const auto mat = saturation(math::Vector3f(2.F, 1.F, 1.F));
                const auto vec = mat * math::Vector3f(1.F, 1.F, 1.F);
            }
            {
                const auto mat = tint(2.F);
                const auto vec = mat * math::Vector3f(1.F, 1.F, 1.F);
            }
            {
                Color color;
                color.brightness = math::Vector3f(2.F, 1.F, 1.F);
                color.contrast = math::Vector3f(2.F, 1.F, 1.F);
                color.saturation = math::Vector3f(2.F, 1.F, 1.F);
                color.tint = 2.F;
                color.invert = true;
                const auto mat = timeline::color(color);
                const auto vec = mat * math::Vector3f(1.F, 1.F, 1.F);
            }
            {
                Levels levels;
                levels.enabled = true;
                TLRENDER_ASSERT(levels == levels);
                TLRENDER_ASSERT(levels != Levels());
            }
            {
                EXRDisplay exrDisplay;
                exrDisplay.enabled = true;
                TLRENDER_ASSERT(exrDisplay == exrDisplay);
                TLRENDER_ASSERT(exrDisplay != EXRDisplay());
            }
            {
                SoftClip softClip;
                softClip.enabled = true;
                TLRENDER_ASSERT(softClip == softClip);
                TLRENDER_ASSERT(softClip != SoftClip());
            }
            {
                DisplayOptions displayOptions;
                displayOptions.channels = Channels::Red;
                TLRENDER_ASSERT(displayOptions == displayOptions);
                TLRENDER_ASSERT(displayOptions != DisplayOptions());
            }
        }
    } // namespace timeline_tests
} // namespace tl
