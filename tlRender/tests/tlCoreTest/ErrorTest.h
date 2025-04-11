// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace core_tests
    {
        class ErrorTest : public tests::ITest
        {
        protected:
            ErrorTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<ErrorTest>
            create(const std::shared_ptr<system::Context>&);

            void run() override;
        };
    } // namespace core_tests
} // namespace tl
