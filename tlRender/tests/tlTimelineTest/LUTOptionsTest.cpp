// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineTest/LUTOptionsTest.h>

#include <tlTimeline/LUTOptions.h>

#include <tlCore/Assert.h>
#include <tlCore/String.h>

using namespace tl::timeline;

namespace tl
{
    namespace timeline_tests
    {
        LUTOptionsTest::LUTOptionsTest(
            const std::shared_ptr<system::Context>& context) :
            ITest("timeline_tests::LUTOptionsTest", context)
        {
        }

        std::shared_ptr<LUTOptionsTest>
        LUTOptionsTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<LUTOptionsTest>(new LUTOptionsTest(context));
        }

        void LUTOptionsTest::run()
        {
            {
                _enum<LUTOrder>("LUTOrder", getLUTOrderEnums);
            }
            {
                _print(
                    "LUT format names: " +
                    string::join(getLUTFormatNames(), ", "));
            }
            {
                _print(
                    "LUT format extensions: " +
                    string::join(getLUTFormatExtensions(), ", "));
            }
        }
    } // namespace timeline_tests
} // namespace tl
