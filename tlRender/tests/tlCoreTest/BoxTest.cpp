// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the feather-tk project.

#include <tlCoreTest/BoxTest.h>

#include <tlCore/Assert.h>
#include <tlCore/Box.h>

#include <sstream>

namespace tl
{
    namespace core_tests
    {
        BoxTest::BoxTest(const std::shared_ptr<system::Context>& context) :
            ITest("tl::core_test::BoxTest", context)
        {}

        BoxTest::~BoxTest()
        {}

        std::shared_ptr<BoxTest> BoxTest::create(
            const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<BoxTest>(new BoxTest(context));
        }

        void BoxTest::run()
        {
            _members();
            _functions();
            _operators();
            _serialize();
        }

        void BoxTest::_members()
        {
            {
                const math::Box2i b2;
                TLRENDER_ASSERT(math::Vector2i() == b2.min);
                TLRENDER_ASSERT(math::Vector2i() == b2.max);
                const math::Box3f b3;
                TLRENDER_ASSERT(math::Vector3f() == b3.min);
                TLRENDER_ASSERT(math::Vector3f() == b3.max);
            }
            {
                const math::Vector2i min2(1, 2);
                const math::Vector2i max2(3, 4);
                const math::Box2i b2(min2, max2);
                TLRENDER_ASSERT(min2 == b2.min);
                TLRENDER_ASSERT(max2 == b2.max);
                TLRENDER_ASSERT(1 == b2.x());
                TLRENDER_ASSERT(2 == b2.y());
                TLRENDER_ASSERT(3 == b2.w());
                TLRENDER_ASSERT(3 == b2.h());
                const math::Vector3f min3(1.F, 2.F, 3.F);
                const math::Vector3f max3(4.F, 5.F, 6.F);
                const math::Box3f b3(min3, max3);
                TLRENDER_ASSERT(min3 == b3.min);
                TLRENDER_ASSERT(max3 == b3.max);
                TLRENDER_ASSERT(1.F == b3.x());
                TLRENDER_ASSERT(2.F == b3.y());
                TLRENDER_ASSERT(3.F == b3.z());
                TLRENDER_ASSERT(3.F == b3.w());
                TLRENDER_ASSERT(3.F == b3.h());
                TLRENDER_ASSERT(3.F == b3.d());
            }
            {
                const math::Vector2i min2(1, 2);
                const math::Size2i size2(3, 2);
                const math::Box2i b2(min2, size2);
                TLRENDER_ASSERT(min2 == b2.min);
                TLRENDER_ASSERT(math::Vector2i(3, 3) == b2.max);
                TLRENDER_ASSERT(size2 == b2.getSize());
                const math::Vector2f min2f(1.F, 2.F);
                const math::Size2f size2f(3.F, 2.F);
                const math::Box2f b2f(min2f, size2f);
                TLRENDER_ASSERT(min2f == b2f.min);
                TLRENDER_ASSERT(math::Vector2f(4.F, 4.F) == b2f.max);
                TLRENDER_ASSERT(size2f == b2f.getSize());
                const math::Vector3f min3(1.F, 2.F, 3.F);
                const math::Size3f size3(4.F, 5.F, 6.F);
                const math::Box3f b3(min3, size3);
                TLRENDER_ASSERT(min3 == b3.min);
                TLRENDER_ASSERT(math::Vector3f(5.F, 7.F, 9.F) == b3.max);
                TLRENDER_ASSERT(size3 == b3.getSize());
            }
        }

        void BoxTest::_functions()
        {
            {
                const math::Box3f b(1.F, 2.F, 3.F, 4.F, 5.F, 6.F);
                const math::Vector3f c = center(b);
                TLRENDER_ASSERT(math::Vector3f(3.F, 4.5F, 6.F) == c);
            }
            {
                const math::Box2f b(1.F, 2.F, 3.F, 4.F);
                const float a = area(b);
                TLRENDER_ASSERT(12.F == a);
            }
            {
                const math::Box3f b(1.F, 2.F, 3.F, 4.F, 5.F, 6.F);
                const float v = volume(b);
                TLRENDER_ASSERT(120.F == v);
            }
            {
                const math::Box2f b(1.F, 2.F, 3.F, 4.F);
                const math::Box2f b1 = move(b, math::Vector2f(-1.F, -2.F));
                TLRENDER_ASSERT(math::Box2f(0.F, 0.F, 3.F, 4.F) == b1);
            }
            {
                const math::Box2i b(0, 0, 3, 3);
                TLRENDER_ASSERT(contains(b, math::Box2i(0, 0, 1, 1)));
                TLRENDER_ASSERT(!contains(b, math::Box2i(-1, -1, 1, 1)));
                TLRENDER_ASSERT(contains(b, math::Vector2i(1, 1)));
                TLRENDER_ASSERT(!contains(b, math::Vector2i(-1, -1)));
                // The edges, which an integer box holds: its size is
                // max - min + 1, so the last row and column are inside it.
                TLRENDER_ASSERT(contains(b, b.min));
                TLRENDER_ASSERT(contains(b, b.max));
                TLRENDER_ASSERT(contains(b, math::Vector2i(b.max.x, b.min.y)));
                TLRENDER_ASSERT(contains(b, math::Vector2i(b.min.x, b.max.y)));
                TLRENDER_ASSERT(!contains(b, math::Vector2i(b.max.x + 1, b.max.y)));
                TLRENDER_ASSERT(!contains(b, math::Vector2i(b.max.x, b.max.y + 1)));
            }
            {
                const math::Box2f b(0.F, 0.F, 2.F, 2.F);
                TLRENDER_ASSERT(contains(b, math::Box2f(0.F, 0.F, 1.F, 1.F)));
                TLRENDER_ASSERT(!contains(b, math::Box2f(-1.F, -1.F, 1.F, 1.F)));
                TLRENDER_ASSERT(contains(b, math::Vector2f(1.F, 1.F)));
                TLRENDER_ASSERT(!contains(b, math::Vector2f(-1.F, -1.F)));
            }
            {
                const math::Box2i b(0, 0, 3, 3);
                TLRENDER_ASSERT(intersects(b, math::Box2i(-1, -1, 2, 2)));
                TLRENDER_ASSERT(!intersects(b, math::Box2i(-2, -2, 2, 2)));
                const math::Box2i b1 = intersect(b, math::Box2i(-1, -1, 2, 2));
                TLRENDER_ASSERT(math::Box2i(0, 0, 1, 1) == b1);
            }
            {
                const math::Box2f b(0.F, 0.F, 2.F, 2.F);
                TLRENDER_ASSERT(intersects(b, math::Box2f(-1.F, -1.F, 2.F, 2.F)));
                TLRENDER_ASSERT(!intersects(b, math::Box2f(-2.F, -2.F, 1.F, 1.F)));
                const math::Box2f b1 = intersect(b, math::Box2f(-1.F, -1.F, 2.F, 2.F));
                TLRENDER_ASSERT(math::Box2f(0.F, 0.F, 1.F, 1.F) == b1);
            }
            {
                const math::Box2i b(1, 2, 3, 4);
                const math::Box2i b1 = expand(b, math::Box2i(1, 2, 4, 5));
                const math::Box2i b2 = expand(b, math::Vector2i(4, 5));
                TLRENDER_ASSERT(math::Box2i(1, 2, 4, 5) == b1);
                TLRENDER_ASSERT(math::Box2i(1, 2, 4, 4) == b2);
            }
            {
                const math::Box2i b(1, 2, 3, 4);
                const math::Box2i b1 = margin(b, math::Vector2i(1, 2));
                const math::Box2i b2 = margin(b, 1);
                const math::Box2i b3 = margin(b, 1, 2);
                const math::Box2i b4 = margin(b, 1, 2, 3, 4);
                TLRENDER_ASSERT(math::Box2i(0, 0, 5, 8) == b1);
                TLRENDER_ASSERT(math::Box2i(0, 1, 5, 6) == b2);
                TLRENDER_ASSERT(math::Box2i(0, 0, 5, 8) == b3);
                TLRENDER_ASSERT(math::Box2i(0, 0, 7, 10) == b4);
            }
            {
                TLRENDER_ASSERT(math::Box2f(1.F, 2.F, 3.F, 4.F) == convert(math::Box2i(1, 2, 3, 4)));
            }
        }

        void BoxTest::_operators()
        {
            {
                const math::Box2i b = math::Box2i(1, 2, 3, 4) + math::Vector2i(5, 6);
                TLRENDER_ASSERT(b == math::Box2i(6, 8, 3, 4));
            }
            {
                const math::Box2i b = math::Box2i(1, 2, 3, 4) - math::Vector2i(5, 6);
                TLRENDER_ASSERT(b == math::Box2i(-4, -4, 3, 4));
            }
            {
                const math::Box2i b(1, 2, 3, 4);
                TLRENDER_ASSERT(b == math::Box2i(1, 2, 3, 4));
                TLRENDER_ASSERT(b != math::Box2i(0, 1, 2, 3));
            }
        }

        void BoxTest::_serialize()
        {
            {
                const math::Box2i b(1, 2, 3, 4);
                const std::string tmp = to_string(b);
                math::Box2i b1;
                TLRENDER_ASSERT(from_string(tmp, b1));
                TLRENDER_ASSERT(b == b1);
            }
            {
                const math::Box2f b(1.F, 2.F, 3.F, 4.F);
                const std::string tmp = to_string(b);
                math::Box2f b1;
                TLRENDER_ASSERT(from_string(tmp, b1));
                TLRENDER_ASSERT(b == b1);
            }
            {
                const math::Box3f b(1.F, 2.F, 3.F, 4.F, 5.F, 6.F);
                const std::string tmp = to_string(b);
                math::Box3f b1;
                TLRENDER_ASSERT(from_string(tmp, b1));
                TLRENDER_ASSERT(b == b1);
            }
            {
                const math::Box2i b(1, 2, 3, 4);
                nlohmann::json json;
                to_json(json, b);
                math::Box2i b2;
                from_json(json, b2);
                TLRENDER_ASSERT(b == b2);
            }
            {
                const math::Box2f b(1.F, 2.F, 3.F, 4.F);
                nlohmann::json json;
                to_json(json, b);
                math::Box2f b2;
                from_json(json, b2);
                TLRENDER_ASSERT(b == b2);
            }
            {
                const math::Box3f b(1.F, 2.F, 3.F, 4.F, 5.F, 6.F);
                nlohmann::json json;
                to_json(json, b);
                math::Box3f b2;
                from_json(json, b2);
                TLRENDER_ASSERT(b == b2);
            }
            {
                const math::Box2i b(1, 2, 3, 4);
                std::stringstream ss;
                ss << b;
            }
            {
                const math::Box2f b(1.F, 2.F, 3.F, 4.F);
                std::stringstream ss;
                ss << b;
            }
            {
                const math::Box3f b(1.F, 2.F, 3.F, 4.F, 5.F, 6.F);
                std::stringstream ss;
                ss << b;
            }
        }
    }
}
