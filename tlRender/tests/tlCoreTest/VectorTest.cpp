// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCoreTest/VectorTest.h>

#include <tlCore/Assert.h>
#include <tlCore/Vector.h>

using namespace tl::math;

namespace tl
{
    namespace core_tests
    {
        VectorTest::VectorTest(
            const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::VectorTest", context)
        {
        }

        std::shared_ptr<VectorTest>
        VectorTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<VectorTest>(new VectorTest(context));
        }

        void VectorTest::run()
        {
            _ctors();
            _components();
            _operators();
            _serialize();
        }

        void VectorTest::_ctors()
        {
            {
                Vector2i v;
                TLRENDER_ASSERT(0 == v.x);
                TLRENDER_ASSERT(0 == v.y);
                v = Vector2i(1, 2);
                TLRENDER_ASSERT(1 == v.x);
                TLRENDER_ASSERT(2 == v.y);
            }
            {
                Vector2f v;
                TLRENDER_ASSERT(0.F == v.x);
                TLRENDER_ASSERT(0.F == v.y);
                v = Vector2f(1.F, 2.F);
                TLRENDER_ASSERT(1.F == v.x);
                TLRENDER_ASSERT(2.F == v.y);
            }
            {
                Vector3f v;
                TLRENDER_ASSERT(0.F == v.x);
                TLRENDER_ASSERT(0.F == v.y);
                TLRENDER_ASSERT(0.F == v.z);
                v = Vector3f(1.F, 2.F, 3.F);
                TLRENDER_ASSERT(1.F == v.x);
                TLRENDER_ASSERT(2.F == v.y);
                TLRENDER_ASSERT(3.F == v.z);
            }
            {
                Vector4f v;
                TLRENDER_ASSERT(0.F == v.x);
                TLRENDER_ASSERT(0.F == v.y);
                TLRENDER_ASSERT(0.F == v.z);
                TLRENDER_ASSERT(0.F == v.w);
                v = Vector4f(1.F, 2.F, 3.F, 4.F);
                TLRENDER_ASSERT(1.F == v.x);
                TLRENDER_ASSERT(2.F == v.y);
                TLRENDER_ASSERT(3.F == v.z);
                TLRENDER_ASSERT(4.F == v.w);
            }
        }

        void VectorTest::_components()
        {
            {
                Vector2i v(1, 2);
                v.zero();
                TLRENDER_ASSERT(0 == v.x);
                TLRENDER_ASSERT(0 == v.y);
            }
            {
                Vector2f v(1.F, 2.F);
                v.zero();
                TLRENDER_ASSERT(0.F == v.x);
                TLRENDER_ASSERT(0.F == v.y);
            }
            {
                Vector3f v(1.F, 2.F, 3.F);
                v.zero();
                TLRENDER_ASSERT(0.F == v.x);
                TLRENDER_ASSERT(0.F == v.y);
                TLRENDER_ASSERT(0.F == v.z);
            }
            {
                Vector4f v(1.F, 2.F, 3.F, 4.F);
                v.zero();
                TLRENDER_ASSERT(0.F == v.x);
                TLRENDER_ASSERT(0.F == v.y);
                TLRENDER_ASSERT(0.F == v.z);
                TLRENDER_ASSERT(0.F == v.w);
            }
            {
                const Vector2i a(1, 2);
                const Vector2i b(3, 4);
                const Vector2i c = a + b;
                TLRENDER_ASSERT(4 == c.x);
                TLRENDER_ASSERT(6 == c.y);
            }
            {
                const Vector2f a(1.F, 2.F);
                const Vector2f b(3.F, 4.F);
                const Vector2f c = a + b;
                TLRENDER_ASSERT(4.F == c.x);
                TLRENDER_ASSERT(6.F == c.y);
            }
            {
                const Vector3f a(1.F, 2.F, 3.F);
                const Vector3f b(4.F, 5.F, 6.F);
                const Vector3f c = a + b;
                TLRENDER_ASSERT(5.F == c.x);
                TLRENDER_ASSERT(7.F == c.y);
                TLRENDER_ASSERT(9.F == c.z);
            }
            {
                const Vector4f a(1.F, 2.F, 3.F, 4.F);
                const Vector4f b(5.F, 6.F, 7.F, 8.F);
                const Vector4f c = a + b;
                TLRENDER_ASSERT(6.F == c.x);
                TLRENDER_ASSERT(8.F == c.y);
                TLRENDER_ASSERT(10.F == c.z);
                TLRENDER_ASSERT(12.F == c.w);
            }
            {
                const Vector2i a(1, 2);
                const Vector2i c = a + 3;
                TLRENDER_ASSERT(4 == c.x);
                TLRENDER_ASSERT(5 == c.y);
            }
            {
                const Vector2f a(1.F, 2.F);
                const Vector2f c = a + 3.F;
                TLRENDER_ASSERT(4.F == c.x);
                TLRENDER_ASSERT(5.F == c.y);
            }
            {
                const Vector3f a(1.F, 2.F, 3.F);
                const Vector3f c = a + 3.F;
                TLRENDER_ASSERT(4.F == c.x);
                TLRENDER_ASSERT(5.F == c.y);
                TLRENDER_ASSERT(6.F == c.z);
            }
            {
                const Vector4f a(1.F, 2.F, 3.F, 4.F);
                const Vector4f c = a + 3.F;
                TLRENDER_ASSERT(4.F == c.x);
                TLRENDER_ASSERT(5.F == c.y);
                TLRENDER_ASSERT(6.F == c.z);
                TLRENDER_ASSERT(7.F == c.w);
            }
            {
                const Vector2i a(1, 2);
                const Vector2i b(3, 4);
                const Vector2i c = b - a;
                TLRENDER_ASSERT(2 == c.x);
                TLRENDER_ASSERT(2 == c.y);
            }
            {
                const Vector2f a(1.F, 2.F);
                const Vector2f b(3.F, 4.F);
                const Vector2f c = b - a;
                TLRENDER_ASSERT(2.F == c.x);
                TLRENDER_ASSERT(2.F == c.y);
            }
            {
                const Vector3f a(1.F, 2.F, 3.F);
                const Vector3f b(4.F, 5.F, 6.F);
                const Vector3f c = b - a;
                TLRENDER_ASSERT(3.F == c.x);
                TLRENDER_ASSERT(3.F == c.y);
                TLRENDER_ASSERT(3.F == c.z);
            }
            {
                const Vector4f a(1.F, 2.F, 3.F, 4.F);
                const Vector4f b(5.F, 6.F, 7.F, 8.F);
                const Vector4f c = b - a;
                TLRENDER_ASSERT(4.F == c.x);
                TLRENDER_ASSERT(4.F == c.y);
                TLRENDER_ASSERT(4.F == c.z);
                TLRENDER_ASSERT(4.F == c.w);
            }
            {
                const Vector2i a(1, 2);
                const Vector2i c = a - 1;
                TLRENDER_ASSERT(0 == c.x);
                TLRENDER_ASSERT(1 == c.y);
            }
            {
                const Vector2f a(1.F, 2.F);
                const Vector2f c = a - 1.F;
                TLRENDER_ASSERT(0.F == c.x);
                TLRENDER_ASSERT(1.F == c.y);
            }
            {
                const Vector3f a(1.F, 2.F, 3.F);
                const Vector3f c = a - 1.F;
                TLRENDER_ASSERT(0.F == c.x);
                TLRENDER_ASSERT(1.F == c.y);
                TLRENDER_ASSERT(2.F == c.z);
            }
            {
                const Vector4f a(1.F, 2.F, 3.F, 4.F);
                const Vector4f c = a - 1.F;
                TLRENDER_ASSERT(0.F == c.x);
                TLRENDER_ASSERT(1.F == c.y);
                TLRENDER_ASSERT(2.F == c.z);
                TLRENDER_ASSERT(3.F == c.w);
            }
            {
                const Vector2i a(3, 4);
                const Vector2i c = a * 2.F;
                TLRENDER_ASSERT(6 == c.x);
                TLRENDER_ASSERT(8 == c.y);
            }
            {
                const Vector2f a(3.F, 4.F);
                const Vector2f c = a * 2.F;
                TLRENDER_ASSERT(6.F == c.x);
                TLRENDER_ASSERT(8.F == c.y);
            }
            {
                const Vector3f a(3.F, 4.F, 5.F);
                const Vector3f c = a * 2.F;
                TLRENDER_ASSERT(6.F == c.x);
                TLRENDER_ASSERT(8.F == c.y);
                TLRENDER_ASSERT(10.F == c.z);
            }
            {
                const Vector4f a(3.F, 4.F, 5.F, 6.F);
                const Vector4f c = a * 2.F;
                TLRENDER_ASSERT(6.F == c.x);
                TLRENDER_ASSERT(8.F == c.y);
                TLRENDER_ASSERT(10.F == c.z);
                TLRENDER_ASSERT(12.F == c.w);
            }
            {
                const Vector2i a(3, 4);
                const Vector2i c = a / 2.F;
                TLRENDER_ASSERT(1 == c.x);
                TLRENDER_ASSERT(2 == c.y);
            }
            {
                const Vector2f a(3.F, 4.F);
                const Vector2f c = a / 2.F;
                TLRENDER_ASSERT(1.5F == c.x);
                TLRENDER_ASSERT(2.F == c.y);
            }
            {
                const Vector3f a(3.F, 4.F, 5.F);
                const Vector3f c = a / 2.F;
                TLRENDER_ASSERT(1.5F == c.x);
                TLRENDER_ASSERT(2.F == c.y);
                TLRENDER_ASSERT(2.5F == c.z);
            }
            {
                const Vector4f a(3.F, 4.F, 5.F, 6.F);
                const Vector4f c = a / 2.F;
                TLRENDER_ASSERT(1.5F == c.x);
                TLRENDER_ASSERT(2.F == c.y);
                TLRENDER_ASSERT(2.5F == c.z);
                TLRENDER_ASSERT(3.F == c.w);
            }
        }

        void VectorTest::_operators()
        {
            {
                Vector2i a;
                Vector2i b;
                TLRENDER_ASSERT(a == b);
                a.x = 1;
                TLRENDER_ASSERT(a != b);
            }
            {
                Vector2f a;
                Vector2f b;
                TLRENDER_ASSERT(a == b);
                a.x = 1.F;
                TLRENDER_ASSERT(a != b);
            }
            {
                Vector3f a;
                Vector3f b;
                TLRENDER_ASSERT(a == b);
                a.x = 1.F;
                TLRENDER_ASSERT(a != b);
            }
            {
                Vector4f a;
                Vector4f b;
                TLRENDER_ASSERT(a == b);
                a.x = 1.F;
                TLRENDER_ASSERT(a != b);
            }
        }

        void VectorTest::_serialize()
        {
            {
                const Vector2i v(1, 2);
                nlohmann::json json;
                to_json(json, v);
                Vector2i v2;
                from_json(json, v2);
                TLRENDER_ASSERT(v == v2);
            }
            {
                const Vector2f v(1.F, 2.F);
                nlohmann::json json;
                to_json(json, v);
                Vector2f v2;
                from_json(json, v2);
                TLRENDER_ASSERT(v == v2);
            }
            {
                const Vector3f v(1.F, 2.F, 3.F);
                nlohmann::json json;
                to_json(json, v);
                Vector3f v2;
                from_json(json, v2);
                TLRENDER_ASSERT(v == v2);
            }
            {
                const Vector4f v(1.F, 2.F, 3.F, 4.F);
                nlohmann::json json;
                to_json(json, v);
                Vector4f v2;
                from_json(json, v2);
                TLRENDER_ASSERT(v == v2);
            }
            {
                const Vector2i v(1, 2);
                std::stringstream ss;
                ss << v;
                Vector2i v2;
                ss >> v2;
                TLRENDER_ASSERT(v == v2);
            }
            {
                const Vector2f v(1.F, 2.F);
                std::stringstream ss;
                ss << v;
                Vector2f v2;
                ss >> v2;
                TLRENDER_ASSERT(v == v2);
            }
            {
                const Vector3f v(1.F, 2.F, 3.F);
                std::stringstream ss;
                ss << v;
                Vector3f v2;
                ss >> v2;
                TLRENDER_ASSERT(v == v2);
            }
            {
                const Vector4f v(1.F, 2.F, 3.F, 4.F);
                std::stringstream ss;
                ss << v;
                Vector4f v2;
                ss >> v2;
                TLRENDER_ASSERT(v == v2);
            }
            try
            {
                Vector2i v;
                std::stringstream ss("...");
                ss >> v;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {
            }
            try
            {
                Vector2f v;
                std::stringstream ss("...");
                ss >> v;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {
            }
            try
            {
                Vector3f v;
                std::stringstream ss("...");
                ss >> v;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {
            }
            try
            {
                Vector4f v;
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
