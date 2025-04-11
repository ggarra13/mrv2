// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace core_tests
    {
        class MatrixTest : public tests::ITest
        {
        protected:
            MatrixTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<MatrixTest>
            create(const std::shared_ptr<system::Context>&);

            void run() override;
        };
    } // namespace core_tests
} // namespace tl
