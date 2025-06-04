// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCoreTest/SizeTest.h>

#include <tlCore/Assert.h>
#include <tlCore/Size.h>

using namespace tl::math;

namespace tl
{
    namespace core_tests
    {
        SizeTest::SizeTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::SizeTest", context)
        {
        }

        std::shared_ptr<SizeTest>
        SizeTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<SizeTest>(new SizeTest(context));
        }

        void SizeTest::run()
        {
            _ctors();
            _components();
            _dimensions();
            _operators();
            _serialize();
        }

        void SizeTest::_ctors()
        {
            {
                Size2i v;
                TLRENDER_ASSERT(0 == v.w);
                TLRENDER_ASSERT(0 == v.h);
                v = Size2i(1, 2);
                TLRENDER_ASSERT(1 == v.w);
                TLRENDER_ASSERT(2 == v.h);
            }
            {
                Size2f v;
                TLRENDER_ASSERT(0.F == v.w);
                TLRENDER_ASSERT(0.F == v.h);
                v = Size2f(1.F, 2.F);
                TLRENDER_ASSERT(1.F == v.w);
                TLRENDER_ASSERT(2.F == v.h);
            }
        }

        void SizeTest::_components()
        {
            {
                TLRENDER_ASSERT(!Size2i().isValid());
                TLRENDER_ASSERT(!Size2f().isValid());
            }
            {
                Size2i v(1, 2);
                v.zero();
                TLRENDER_ASSERT(0 == v.w);
                TLRENDER_ASSERT(0 == v.h);
            }
            {
                Size2f v(1, 2);
                v.zero();
                TLRENDER_ASSERT(0.F == v.w);
                TLRENDER_ASSERT(0.F == v.h);
            }
        }

        void SizeTest::_dimensions()
        {
            {
                Size2i v(1, 2);
                TLRENDER_ASSERT(2.F == v.getArea());
            }
            {
                Size2f v(1.F, 2.F);
                TLRENDER_ASSERT(2.F == v.getArea());
            }
            {
                Size2i v(2, 1);
                TLRENDER_ASSERT(2.F == v.getAspect());
            }
            {
                Size2f v(2.F, 1.F);
                TLRENDER_ASSERT(2.F == v.getAspect());
            }
        }

        void SizeTest::_operators()
        {
            {
                Size2i a;
                Size2i b;
                TLRENDER_ASSERT(a == b);
                a.w = 1;
                TLRENDER_ASSERT(a != b);
            }
            {
                Size2f a;
                Size2f b;
                TLRENDER_ASSERT(a == b);
                a.w = 1.F;
                TLRENDER_ASSERT(a != b);
            }
            {
                const Size2i a(1, 2);
                const Size2i b(3, 4);
                const Size2i c = a + b;
                TLRENDER_ASSERT(4 == c.w);
                TLRENDER_ASSERT(6 == c.h);
            }
            {
                const Size2f a(1.F, 2.F);
                const Size2f b(3.F, 4.F);
                const Size2f c = a + b;
                TLRENDER_ASSERT(4.F == c.w);
                TLRENDER_ASSERT(6.F == c.h);
            }
            {
                const Size2i a(1, 2);
                const Size2i c = a + 3;
                TLRENDER_ASSERT(4 == c.w);
                TLRENDER_ASSERT(5 == c.h);
            }
            {
                const Size2f a(1.F, 2.F);
                const Size2f c = a + 3.F;
                TLRENDER_ASSERT(4.F == c.w);
                TLRENDER_ASSERT(5.F == c.h);
            }
            {
                const Size2i a(1, 2);
                const Size2i b(3, 4);
                const Size2i c = b - a;
                TLRENDER_ASSERT(2 == c.w);
                TLRENDER_ASSERT(2 == c.h);
            }
            {
                const Size2f a(1.F, 2.F);
                const Size2f b(3.F, 4.F);
                const Size2f c = b - a;
                TLRENDER_ASSERT(2.F == c.w);
                TLRENDER_ASSERT(2.F == c.h);
            }
            {
                const Size2i a(1, 2);
                const Size2i c = a - 1;
                TLRENDER_ASSERT(0 == c.w);
                TLRENDER_ASSERT(1 == c.h);
            }
            {
                const Size2f a(1.F, 2.F);
                const Size2f c = a - 1.F;
                TLRENDER_ASSERT(0.F == c.w);
                TLRENDER_ASSERT(1.F == c.h);
            }
            {
                const Size2i a(3, 4);
                const Size2i c = a * 2.F;
                TLRENDER_ASSERT(6 == c.w);
                TLRENDER_ASSERT(8 == c.h);
            }
            {
                const Size2f a(3.F, 4.F);
                const Size2f c = a * 2.F;
                TLRENDER_ASSERT(6.F == c.w);
                TLRENDER_ASSERT(8.F == c.h);
            }
            {
                const Size2i a(3, 4);
                const Size2i c = a / 2.F;
                TLRENDER_ASSERT(1 == c.w);
                TLRENDER_ASSERT(2 == c.h);
            }
            {
                const Size2f a(3.F, 4.F);
                const Size2f c = a / 2.F;
                TLRENDER_ASSERT(1.5F == c.w);
                TLRENDER_ASSERT(2.F == c.h);
            }
        }

        void SizeTest::_serialize()
        {
            {
                const Size2i v(1, 2);
                nlohmann::json json;
                to_json(json, v);
                Size2i v2;
                from_json(json, v2);
                TLRENDER_ASSERT(v == v2);
            }
            {
                const Size2f v(1.F, 2.F);
                nlohmann::json json;
                to_json(json, v);
                Size2f v2;
                from_json(json, v2);
                TLRENDER_ASSERT(v == v2);
            }
            {
                const Size2i v(1, 2);
                std::stringstream ss;
                ss << v;
                Size2i v2;
                ss >> v2;
                TLRENDER_ASSERT(v == v2);
            }
            {
                const Size2f v(1.F, 2.F);
                std::stringstream ss;
                ss << v;
                Size2f v2;
                ss >> v2;
                TLRENDER_ASSERT(v == v2);
            }
            try
            {
                Size2i v;
                std::stringstream ss("...");
                ss >> v;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {
            }
            try
            {
                Size2f v;
                std::stringstream ss("...");
                ss >> v;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {
            }
        }
    } // namespace core_tests
} // namespace tl
