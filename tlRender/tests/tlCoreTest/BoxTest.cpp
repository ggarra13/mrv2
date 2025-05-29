// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCoreTest/BoxTest.h>

#include <tlCore/Assert.h>
#include <tlCore/Box.h>

using namespace tl::math;

namespace tl
{
    namespace core_tests
    {
        BoxTest::BoxTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::BoxTest", context)
        {
        }

        std::shared_ptr<BoxTest>
        BoxTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<BoxTest>(new BoxTest(context));
        }

        void BoxTest::run()
        {
            _ctors();
            _components();
            _dimensions();
            _intersections();
            _expand();
            _margin();
            _operators();
            _serialize();
        }

        void BoxTest::_ctors()
        {
            {
                const Box2i b;
                TLRENDER_ASSERT(0 == b.min.x);
                TLRENDER_ASSERT(0 == b.min.y);
                TLRENDER_ASSERT(0 == b.max.x);
                TLRENDER_ASSERT(0 == b.max.y);
            }
            {
                const Box2f b;
                TLRENDER_ASSERT(0.F == b.min.x);
                TLRENDER_ASSERT(0.F == b.min.y);
                TLRENDER_ASSERT(0.F == b.max.x);
                TLRENDER_ASSERT(0.F == b.max.y);
            }
            {
                const Box2i b(Vector2i(1, 2));
                TLRENDER_ASSERT(1 == b.min.x);
                TLRENDER_ASSERT(2 == b.min.y);
                TLRENDER_ASSERT(1 == b.max.x);
                TLRENDER_ASSERT(2 == b.max.y);
            }
            {
                const Box2f b(Vector2f(1.F, 2.F));
                TLRENDER_ASSERT(1.F == b.min.x);
                TLRENDER_ASSERT(2.F == b.min.y);
                TLRENDER_ASSERT(1.F == b.max.x);
                TLRENDER_ASSERT(2.F == b.max.y);
            }
            {
                const Box2i b(Size2i(2, 3));
                TLRENDER_ASSERT(0 == b.min.x);
                TLRENDER_ASSERT(0 == b.min.y);
                TLRENDER_ASSERT(1 == b.max.x);
                TLRENDER_ASSERT(2 == b.max.y);
            }
            {
                const Box2f b(Size2f(2.F, 3.F));
                TLRENDER_ASSERT(0.F == b.min.x);
                TLRENDER_ASSERT(0.F == b.min.y);
                TLRENDER_ASSERT(2.F == b.max.x);
                TLRENDER_ASSERT(3.F == b.max.y);
            }
            {
                const Box2i b(Vector2i(1, 2), Vector2i(3, 4));
                TLRENDER_ASSERT(1 == b.min.x);
                TLRENDER_ASSERT(2 == b.min.y);
                TLRENDER_ASSERT(3 == b.max.x);
                TLRENDER_ASSERT(4 == b.max.y);
            }
            {
                const Box2f b(Vector2f(1.F, 2.F), Vector2f(3.F, 4.F));
                TLRENDER_ASSERT(1.F == b.min.x);
                TLRENDER_ASSERT(2.F == b.min.y);
                TLRENDER_ASSERT(3.F == b.max.x);
                TLRENDER_ASSERT(4.F == b.max.y);
            }
            {
                const Box2i b(1, 2, 3, 4);
                TLRENDER_ASSERT(1 == b.min.x);
                TLRENDER_ASSERT(2 == b.min.y);
                TLRENDER_ASSERT(3 == b.max.x);
                TLRENDER_ASSERT(5 == b.max.y);
            }
            {
                const Box2f b(1.F, 2.F, 3.F, 4.F);
                TLRENDER_ASSERT(1.F == b.min.x);
                TLRENDER_ASSERT(2.F == b.min.y);
                TLRENDER_ASSERT(4.F == b.max.x);
                TLRENDER_ASSERT(6.F == b.max.y);
            }
        }

        void BoxTest::_components()
        {
            {
                const Box2i b(1, 2, 3, 4);
                TLRENDER_ASSERT(1 == b.x());
                TLRENDER_ASSERT(2 == b.y());
                TLRENDER_ASSERT(3 == b.w());
                TLRENDER_ASSERT(4 == b.h());
            }
            {
                const Box2f b(1.F, 2.F, 3.F, 4.F);
                TLRENDER_ASSERT(1.F == b.x());
                TLRENDER_ASSERT(2.F == b.y());
                TLRENDER_ASSERT(3.F == b.w());
                TLRENDER_ASSERT(4.F == b.h());
            }
            {
                TLRENDER_ASSERT(!Box2i().isValid());
                TLRENDER_ASSERT(!Box2f().isValid());
            }
            {
                Box2i b(1, 2, 3, 4);
                b.zero();
                TLRENDER_ASSERT(0 == b.min.x);
                TLRENDER_ASSERT(0 == b.min.y);
                TLRENDER_ASSERT(0 == b.max.x);
                TLRENDER_ASSERT(0 == b.max.y);
            }
            {
                Box2i b(1.F, 2.F, 3.F, 4.F);
                b.zero();
                TLRENDER_ASSERT(0.F == b.min.x);
                TLRENDER_ASSERT(0.F == b.min.y);
                TLRENDER_ASSERT(0.F == b.max.x);
                TLRENDER_ASSERT(0.F == b.max.y);
            }
        }

        void BoxTest::_dimensions()
        {
            {
                Box2i b(1, 2, 3, 4);
                TLRENDER_ASSERT(Size2i(3, 4) == b.getSize());
                TLRENDER_ASSERT(Vector2i(2, 4) == b.getCenter());
            }
            {
                Box2f b(1.F, 2.F, 3.F, 4.F);
                TLRENDER_ASSERT(Size2f(3.F, 4.F) == b.getSize());
                const auto c = b.getCenter();
                TLRENDER_ASSERT(Vector2f(2.5F, 4.F) == c);
            }
        }

        void BoxTest::_intersections()
        {
            {
                TLRENDER_ASSERT(Box2i(0, 0, 1, 1).contains(Box2i(0, 0, 1, 1)));
                TLRENDER_ASSERT(!Box2i(0, 0, 1, 1).contains(Box2i(1, 1, 1, 1)));
                TLRENDER_ASSERT(
                    !Box2i(0, 0, 1, 1).contains(Box2i(-1, -1, 1, 1)));
            }
            {
                TLRENDER_ASSERT(Box2f(0.F, 0.F, 1.F, 1.F)
                                    .contains(Box2f(0.F, 0.F, 1.F, 1.F)));
                TLRENDER_ASSERT(!Box2f(0.F, 0.F, 1.F, 1.F)
                                     .contains(Box2f(1.F, 1.F, 1.F, 1.F)));
                TLRENDER_ASSERT(!Box2f(0.F, 0.F, 1.F, 1.F)
                                     .contains(Box2f(-1.F, -1.F, 1.F, 1.F)));
            }
            {
                TLRENDER_ASSERT(
                    Box2i(0, 0, 1, 1).intersects(Box2i(0, 0, 1, 1)));
                TLRENDER_ASSERT(
                    !Box2i(0, 0, 1, 1).intersects(Box2i(2, 2, 1, 1)));
                TLRENDER_ASSERT(
                    !Box2i(0, 0, 1, 1).intersects(Box2i(-2, -2, 1, 1)));
            }
            {
                TLRENDER_ASSERT(Box2f(0.F, 0.F, 1.F, 1.F)
                                    .intersects(Box2f(0.F, 0.F, 1.F, 1.F)));
                TLRENDER_ASSERT(!Box2f(0.F, 0.F, 1.F, 1.F)
                                     .intersects(Box2f(2.F, 2.F, 1.F, 1.F)));
                TLRENDER_ASSERT(!Box2f(0.F, 0.F, 1.F, 1.F)
                                     .intersects(Box2f(-2.F, -2.F, 1.F, 1.F)));
            }
            {
                TLRENDER_ASSERT(
                    Box2i(0, 0, 1, 1).intersect(Box2i(0, 0, 1, 1)) ==
                    Box2i(0, 0, 1, 1));
                TLRENDER_ASSERT(
                    Box2i(0, 0, 1, 1).intersect(Box2i(-1, -1, 2, 2)) ==
                    Box2i(0, 0, 1, 1));
                TLRENDER_ASSERT(
                    !Box2i(Box2i(0, 0, 1, 1).intersect(Box2i(2, 2, 1, 1)))
                         .isValid());
                TLRENDER_ASSERT(
                    !Box2i(Box2i(0, 0, 1, 1).intersect(Box2i(-2, -2, 1, 1)))
                         .isValid());
            }
            {
                TLRENDER_ASSERT(
                    Box2f(0.F, 0.F, 1.F, 1.F)
                        .intersect(Box2f(0.F, 0.F, 1.F, 1.F)) ==
                    Box2f(0.F, 0.F, 1.F, 1.F));
                TLRENDER_ASSERT(
                    Box2f(0.F, 0.F, 1.F, 1.F)
                        .intersect(Box2f(-1.F, -1.F, 2.F, 2.F)) ==
                    Box2f(0.F, 0.F, 1.F, 1.F));
                TLRENDER_ASSERT(
                    !Box2f(Box2f(0.F, 0.F, 1.F, 1.F)
                               .intersect(Box2f(2.F, 2.F, 1.F, 1.F)))
                         .isValid());
                TLRENDER_ASSERT(
                    !Box2f(Box2f(0.F, 0.F, 1.F, 1.F)
                               .intersect(Box2f(-2.F, -2.F, 1.F, 1.F)))
                         .isValid());
            }
        }

        void BoxTest::_expand()
        {
            {
                Box2i b(0, 1, 2, 3);
                b.expand(Box2i(4, 5, 6, 7));
                TLRENDER_ASSERT(Box2i(0, 1, 10, 11) == b);
            }
            {
                Box2f b(0.F, 1.F, 2.F, 3.F);
                b.expand(Box2f(4.F, 5.F, 6.F, 7.F));
                TLRENDER_ASSERT(Box2f(0.F, 1.F, 10.F, 11.F) == b);
            }
            {
                Box2i b(0, 1, 2, 3);
                b.expand(Vector2i(6, 7));
                TLRENDER_ASSERT(Box2i(0, 1, 7, 7) == b);
            }
            {
                Box2f b(0.F, 1.F, 2.F, 3.F);
                b.expand(Vector2f(6.F, 7.F));
                TLRENDER_ASSERT(Box2f(0.F, 1.F, 6.F, 6.F) == b);
            }
        }

        void BoxTest::_margin()
        {
            {
                TLRENDER_ASSERT(
                    Box2i(0, 1, 2, 3).margin(Vector2i(1, 2)) ==
                    Box2i(-1, -1, 4, 7));
                TLRENDER_ASSERT(
                    Box2f(0.F, 1.F, 2.F, 3.F).margin(Vector2f(1.F, 2.F)) ==
                    Box2f(-1.F, -1.F, 4.F, 7.F));
            }
            {
                TLRENDER_ASSERT(
                    Box2i(0, 1, 2, 3).margin(1) == Box2i(-1, 0, 4, 5));
                TLRENDER_ASSERT(
                    Box2f(0.F, 1.F, 2.F, 3.F).margin(1.F) ==
                    Box2f(-1.F, 0.F, 4.F, 5.F));
            }
            {
                const auto b = Box2i(0, 1, 2, 3).margin(1, 2, 3, 4);
                TLRENDER_ASSERT(
                    Box2i(0, 1, 2, 3).margin(1, 2, 3, 4) ==
                    Box2i(-1, -1, 6, 9));
                const auto b2 =
                    Box2f(0.F, 1.F, 2.F, 3.F).margin(1.F, 2.F, 3.F, 4.F);
                TLRENDER_ASSERT(
                    Box2f(0.F, 1.F, 2.F, 3.F).margin(1.F, 2.F, 3.F, 4.F) ==
                    Box2f(-1.F, -1.F, 6.F, 9.F));
            }
        }

        void BoxTest::_operators()
        {
            {
                TLRENDER_ASSERT(Box2i(0, 1, 2, 3) == Box2i(0, 1, 2, 3));
                TLRENDER_ASSERT(Box2i(0, 1, 2, 3) != Box2i(3, 2, 1, 0));
                TLRENDER_ASSERT(
                    Box2f(0.F, 1.F, 2.F, 3.F) == Box2f(0.F, 1.F, 2.F, 3.F));
                TLRENDER_ASSERT(
                    Box2f(0.F, 1.F, 2.F, 3.F) != Box2f(3.F, 2.F, 1.F, 0.F));
            }
            {
                const auto b = Box2i(0, 1, 2, 3) * 2.F;
                TLRENDER_ASSERT(b == Box2i(0, 2, 3, 5));
                const auto b2 = Box2f(0.F, 1.F, 2.F, 3.F) * 2.F;
                TLRENDER_ASSERT(b2 == Box2f(0.F, 2.F, 4.F, 6.F));
            }
        }

        void BoxTest::_serialize()
        {
            {
                const Box2i b(1, 2, 3, 4);
                nlohmann::json json;
                to_json(json, b);
                Box2i b2;
                from_json(json, b2);
                TLRENDER_ASSERT(b == b2);
            }
            {
                const Box2f b(1.F, 2.F, 3.F, 4.F);
                nlohmann::json json;
                to_json(json, b);
                Box2f b2;
                from_json(json, b2);
                TLRENDER_ASSERT(b == b2);
            }
            {
                const Box2i b(1, 2, 3, 4);
                std::stringstream ss;
                ss << b;
                Box2i b2;
                ss >> b2;
                TLRENDER_ASSERT(b == b2);
            }
            {
                const Box2f b(1.F, 2.F, 3.F, 4.F);
                std::stringstream ss;
                ss << b;
                Box2f b2;
                ss >> b2;
                TLRENDER_ASSERT(b == b2);
            }
            try
            {
                Box2i b;
                std::stringstream ss("...");
                ss >> b;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {
            }
            try
            {
                Box2f b;
                std::stringstream ss("...");
                ss >> b;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {
            }
        }
    } // namespace core_tests
} // namespace tl
