// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCoreTest/ErrorTest.h>

#include <tlCore/Assert.h>
#include <tlCore/Error.h>

namespace tl
{
    namespace core_tests
    {
        ErrorTest::ErrorTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::ErrorTest", context)
        {
        }

        std::shared_ptr<ErrorTest>
        ErrorTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<ErrorTest>(new ErrorTest(context));
        }

        void ErrorTest::run()
        {
            try
            {
                throw error::ParseError();
            }
            catch (const std::exception& e)
            {
                _print(e.what());
            }
        }
    } // namespace core_tests
} // namespace tl
