// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCoreTest/LogSystemTest.h>

#include <tlCore/LogSystem.h>

using namespace tl::log;

namespace tl
{
    namespace core_tests
    {
        LogSystemTest::LogSystemTest(
            const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::LogSystemTest", context)
        {
        }

        std::shared_ptr<LogSystemTest>
        LogSystemTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<LogSystemTest>(new LogSystemTest(context));
        }

        void LogSystemTest::run()
        {
            {
                Item item;
                item.message = "Test";
                TLRENDER_ASSERT(item == item);
                TLRENDER_ASSERT(item != Item());
            }
        }
    } // namespace core_tests
} // namespace tl
