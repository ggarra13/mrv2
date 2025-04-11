// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCoreTest/ValueObserverTest.h>

#include <tlCore/Assert.h>
#include <tlCore/ValueObserver.h>

namespace tl
{
    namespace core_tests
    {
        ValueObserverTest::ValueObserverTest(
            const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::ValueObserverTest", context)
        {
        }

        std::shared_ptr<ValueObserverTest> ValueObserverTest::create(
            const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<ValueObserverTest>(
                new ValueObserverTest(context));
        }

        void ValueObserverTest::run()
        {
            auto value = observer::Value<int>::create();
            TLRENDER_ASSERT(0 == value->get());
            value = observer::Value<int>::create(1);
            TLRENDER_ASSERT(1 == value->get());

            int result = 0;
            auto observer = observer::ValueObserver<int>::create(
                value, [&result](int value) { result = value; });
            bool changed = value->setIfChanged(2);
            TLRENDER_ASSERT(changed);
            TLRENDER_ASSERT(2 == result);
            changed = value->setIfChanged(2);
            TLRENDER_ASSERT(!changed);

            {
                int result2 = 0;
                auto observer2 = observer::ValueObserver<int>::create(
                    value, [&result2](int value) { result2 = value; });
                value->setIfChanged(3);
                TLRENDER_ASSERT(3 == result);
                TLRENDER_ASSERT(3 == result2);

                TLRENDER_ASSERT(2 == value->getObserversCount());
            }

            TLRENDER_ASSERT(1 == value->getObserversCount());
        }
    } // namespace core_tests
} // namespace tl
