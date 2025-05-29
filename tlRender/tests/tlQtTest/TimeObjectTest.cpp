// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlQtTest/TimeObjectTest.h>

namespace tl
{
    namespace qt_tests
    {
        TimeObjectTest::TimeObjectTest(
            const std::shared_ptr<system::Context>& context) :
            ITest("qt_tests::TimeObjectTest", context)
        {
        }

        std::shared_ptr<TimeObjectTest>
        TimeObjectTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<TimeObjectTest>(new TimeObjectTest(context));
        }

        void TimeObjectTest::run() {}
    } // namespace qt_tests
} // namespace tl
