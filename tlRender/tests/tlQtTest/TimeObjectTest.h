// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace qt_tests
    {
        class TimeObjectTest : public tests::ITest
        {
        protected:
            TimeObjectTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<TimeObjectTest>
            create(const std::shared_ptr<system::Context>&);

            void run() override;
        };
    } // namespace qt_tests
} // namespace tl
