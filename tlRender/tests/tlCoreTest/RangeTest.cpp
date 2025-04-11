// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCoreTest/RangeTest.h>

#include <tlCore/Assert.h>
#include <tlCore/Range.h>

using namespace tl::math;

namespace tl
{
    namespace core_tests
    {
        RangeTest::RangeTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::RangeTest", context)
        {
        }

        std::shared_ptr<RangeTest>
        RangeTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<RangeTest>(new RangeTest(context));
        }

        void RangeTest::run()
        {
            {
                const auto r = IntRange();
                TLRENDER_ASSERT(0 == r.getMin());
                TLRENDER_ASSERT(0 == r.getMax());
            }
            {
                const auto r = IntRange(1);
                TLRENDER_ASSERT(1 == r.getMin());
                TLRENDER_ASSERT(1 == r.getMax());
            }
            {
                const auto r = IntRange(1, 10);
                TLRENDER_ASSERT(1 == r.getMin());
                TLRENDER_ASSERT(10 == r.getMax());
            }
            {
                auto r = IntRange(1, 10);
                r.zero();
                TLRENDER_ASSERT(0 == r.getMin());
                TLRENDER_ASSERT(0 == r.getMax());
            }
            {
                const auto r = IntRange(1, 10);
                TLRENDER_ASSERT(r.contains(1));
                TLRENDER_ASSERT(r.contains(10));
                TLRENDER_ASSERT(!r.contains(0));
                TLRENDER_ASSERT(!r.contains(11));
            }
            {
                const auto r = IntRange(1, 10);
                TLRENDER_ASSERT(r.intersects(IntRange(0, 1)));
                TLRENDER_ASSERT(r.intersects(IntRange(10, 11)));
                TLRENDER_ASSERT(!r.intersects(IntRange(12, 20)));
            }
            {
                auto r = IntRange(1, 10);
                r.expand(20);
                TLRENDER_ASSERT(IntRange(1, 20) == r);
            }
            {
                auto r = IntRange(1, 10);
                r.expand(IntRange(0, 20));
                TLRENDER_ASSERT(IntRange(0, 20) == r);
            }
            {
                TLRENDER_ASSERT(IntRange(1, 10) == IntRange(1, 10));
                TLRENDER_ASSERT(IntRange(1, 10) != IntRange(0, 11));
                TLRENDER_ASSERT(IntRange(0, 10) < IntRange(1, 11));
            }
            {
                const IntRange r(1, 10);
                nlohmann::json json;
                to_json(json, r);
                IntRange r2;
                from_json(json, r2);
                TLRENDER_ASSERT(r == r2);
            }
            {
                const SizeTRange r(1, 10);
                nlohmann::json json;
                to_json(json, r);
                SizeTRange r2;
                from_json(json, r2);
                TLRENDER_ASSERT(r == r2);
            }
            {
                const FloatRange r(1.F, 10.F);
                nlohmann::json json;
                to_json(json, r);
                FloatRange r2;
                from_json(json, r2);
                TLRENDER_ASSERT(r == r2);
            }
            {
                const DoubleRange r(1.0, 10.0);
                nlohmann::json json;
                to_json(json, r);
                DoubleRange r2;
                from_json(json, r2);
                TLRENDER_ASSERT(r == r2);
            }
            {
                const IntRange r(1, 10);
                std::stringstream ss;
                ss << r;
                IntRange r2;
                ss >> r2;
                TLRENDER_ASSERT(r == r2);
            }
            try
            {
                IntRange r;
                std::stringstream ss("...");
                ss >> r;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {
            }
            {
                const SizeTRange r(1, 10);
                std::stringstream ss;
                ss << r;
                SizeTRange r2;
                ss >> r2;
                TLRENDER_ASSERT(r == r2);
            }
            try
            {
                SizeTRange r;
                std::stringstream ss("...");
                ss >> r;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {
            }
            {
                const FloatRange r(1.F, 10.F);
                std::stringstream ss;
                ss << r;
                FloatRange r2;
                ss >> r2;
                TLRENDER_ASSERT(r == r2);
            }
            try
            {
                FloatRange r;
                std::stringstream ss("...");
                ss >> r;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {
            }
            {
                const DoubleRange r(1.0, 10.0);
                std::stringstream ss;
                ss << r;
                DoubleRange r2;
                ss >> r2;
                TLRENDER_ASSERT(r == r2);
            }
            try
            {
                DoubleRange r;
                std::stringstream ss("...");
                ss >> r;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {
            }
        }
    } // namespace core_tests
} // namespace tl
