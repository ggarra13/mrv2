// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCoreTest/ColorTest.h>

#include <tlCore/Assert.h>
#include <tlCore/Color.h>

using namespace tl::image;

namespace tl
{
    namespace core_tests
    {
        ColorTest::ColorTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::ColorTest", context)
        {
        }

        std::shared_ptr<ColorTest>
        ColorTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<ColorTest>(new ColorTest(context));
        }

        void ColorTest::run()
        {
            {
                const Color4f c;
                TLRENDER_ASSERT(0.F == c.r);
                TLRENDER_ASSERT(0.F == c.g);
                TLRENDER_ASSERT(0.F == c.b);
                TLRENDER_ASSERT(0.F == c.a);
            }
            {
                const Color4f c(1.F, 2.F, 3.F, 4.F);
                TLRENDER_ASSERT(1.F == c.r);
                TLRENDER_ASSERT(2.F == c.g);
                TLRENDER_ASSERT(3.F == c.b);
                TLRENDER_ASSERT(4.F == c.a);
            }
            {
                Color4f a;
                Color4f b;
                TLRENDER_ASSERT(a == b);
                a.r = 1.F;
                TLRENDER_ASSERT(a != b);
            }
            {
                const Color4f c(1.F, .5F, 0.F);
                nlohmann::json json;
                to_json(json, c);
                Color4f c2;
                from_json(json, c2);
                TLRENDER_ASSERT(c == c2);
            }
            {
                const Color4f c(1.F, .5F, 0.F);
                std::stringstream ss;
                ss << c;
                Color4f c2;
                ss >> c2;
                TLRENDER_ASSERT(c == c2);
            }
            try
            {
                Color4f c;
                std::stringstream ss("...");
                ss >> c;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {
            }
        }
    } // namespace core_tests
} // namespace tl
