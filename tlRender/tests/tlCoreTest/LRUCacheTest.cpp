// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCoreTest/LRUCacheTest.h>

#include <tlCore/Assert.h>
#include <tlCore/LRUCache.h>
#include <tlCore/Memory.h>

using namespace tl::memory;

namespace tl
{
    namespace core_tests
    {
        LRUCacheTest::LRUCacheTest(
            const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::LRUCacheTest", context)
        {
        }

        std::shared_ptr<LRUCacheTest>
        LRUCacheTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<LRUCacheTest>(new LRUCacheTest(context));
        }

        void LRUCacheTest::run()
        {
            {
                LRUCache<int, int> c;
                TLRENDER_ASSERT(0 == c.getSize());
                TLRENDER_ASSERT(0.F == c.getPercentage());
            }
            {
                LRUCache<int, int> c;
                TLRENDER_ASSERT(!c.contains(0));
                int v = 0;
                TLRENDER_ASSERT(!c.get(0, v));
                c.add(0, 1);
                TLRENDER_ASSERT(1 == c.getSize());
                TLRENDER_ASSERT(c.contains(0));
                TLRENDER_ASSERT(c.get(0, v));
                TLRENDER_ASSERT(1 == v);
                c.remove(0);
                TLRENDER_ASSERT(!c.contains(0));
                c.add(0, 1);
                c.clear();
                TLRENDER_ASSERT(!c.contains(0));
            }
            {
                LRUCache<int, int> c;
                c.setMax(3);
                TLRENDER_ASSERT(3 == c.getMax());
                c.add(0, 1);
                c.add(1, 2);
                c.add(2, 3);
                c.add(3, 4);
                TLRENDER_ASSERT(c.contains(1));
                TLRENDER_ASSERT(c.contains(2));
                TLRENDER_ASSERT(c.contains(3));
                int v = 0;
                c.get(1, v);
                c.add(4, 5);
                TLRENDER_ASSERT(c.contains(1));
                TLRENDER_ASSERT(c.contains(3));
                TLRENDER_ASSERT(c.contains(4));
                const auto l = c.getKeys();
                TLRENDER_ASSERT(std::vector<int>({1, 3, 4}) == c.getKeys());
                TLRENDER_ASSERT(std::vector<int>({2, 4, 5}) == c.getValues());
            }
            {
                LRUCache<int, int> c;
                c.setMax(3 * memory::megabyte);
                c.add(0, 1, memory::megabyte);
                c.add(1, 2, memory::megabyte);
                c.add(2, 3, memory::megabyte);
                c.add(3, 4, memory::megabyte);
                TLRENDER_ASSERT(c.contains(1));
                TLRENDER_ASSERT(c.contains(2));
                TLRENDER_ASSERT(c.contains(3));
                int v = 0;
                c.get(1, v);
                c.add(4, 5, memory::megabyte);
                TLRENDER_ASSERT(c.contains(1));
                TLRENDER_ASSERT(c.contains(3));
                TLRENDER_ASSERT(c.contains(4));
                const auto l = c.getKeys();
                TLRENDER_ASSERT(std::vector<int>({1, 3, 4}) == c.getKeys());
                TLRENDER_ASSERT(std::vector<int>({2, 4, 5}) == c.getValues());
            }
        }
    } // namespace core_tests
} // namespace tl
