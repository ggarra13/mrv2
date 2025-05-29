// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace app_tests
    {
        class CmdLineTest : public tests::ITest
        {
        protected:
            CmdLineTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<CmdLineTest>
            create(const std::shared_ptr<system::Context>&);

            void run() override;
        };
    } // namespace app_tests
} // namespace tl
