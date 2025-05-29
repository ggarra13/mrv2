// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCoreTest/HDRTest.h>

#include <tlCore/Assert.h>
#include <tlCore/HDR.h>

using namespace tl::image;

namespace tl
{
    namespace core_tests
    {
        HDRTest::HDRTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::HDRTest", context)
        {
        }

        std::shared_ptr<HDRTest>
        HDRTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<HDRTest>(new HDRTest(context));
        }

        void HDRTest::run()
        {
            {
                HDRData a;
                HDRData b;
                TLRENDER_ASSERT(a == b);
                a.eotf = 1;
                TLRENDER_ASSERT(a != b);
            }
            {
                HDRData value;
                value.eotf = 1;
                value.primaries[0].x = .1F;
                value.primaries[0].y = 1.F;
                value.primaries[1].x = .2F;
                value.primaries[1].y = .9F;
                value.primaries[2].x = .3F;
                value.primaries[2].y = .8F;
                value.primaries[3].x = .4F;
                value.primaries[3].y = .7F;
                value.displayMasteringLuminance = math::FloatRange(.5F, .6F);
                value.maxCLL = 0.1F;
                value.maxFALL = 0.2F;
                nlohmann::json json;
                to_json(json, value);
                HDRData value2;
                from_json(json, value2);
                TLRENDER_ASSERT(value == value2);
            }
        }
    } // namespace core_tests
} // namespace tl
