// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/PlayerOptionsTest.h>

#include <tlTimeline/PlayerOptions.h>

#include <tlCore/Assert.h>
#include <tlCore/String.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        PlayerOptionsTest::PlayerOptionsTest(
            const std::shared_ptr<system::Context>& context) :
            ITest("timeline_tests::PlayerOptionsTest", context)
        {
        }

        std::shared_ptr<PlayerOptionsTest> PlayerOptionsTest::create(
            const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<PlayerOptionsTest>(
                new PlayerOptionsTest(context));
        }

        void PlayerOptionsTest::run()
        {
            {
                _enum<TimerMode>("TimerMode", getTimerModeEnums);
            }
            {
                PlayerCacheOptions v;
                v.readAhead = otime::RationalTime(10.0, 1.0);
                TLRENDER_ASSERT(v == v);
                TLRENDER_ASSERT(v != PlayerCacheOptions());
            }
            {
                PlayerOptions v;
                v.timerMode = TimerMode::Audio;
                TLRENDER_ASSERT(v == v);
                TLRENDER_ASSERT(v != PlayerOptions());
            }
        }
    } // namespace timeline_tests
} // namespace tl
