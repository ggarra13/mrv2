// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace app_tests
    {
        class AppTest : public tests::ITest
        {
        protected:
            AppTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<AppTest>
            create(const std::shared_ptr<system::Context>&);

            void run() override;

        private:
            void _convert();
            void _app();
        };
    } // namespace app_tests
} // namespace tl
