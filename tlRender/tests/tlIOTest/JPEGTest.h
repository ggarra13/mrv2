// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace io_tests
    {
        class JPEGTest : public tests::ITest
        {
        protected:
            JPEGTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<JPEGTest>
            create(const std::shared_ptr<system::Context>&);

            void run() override;
        };
    } // namespace io_tests
} // namespace tl
