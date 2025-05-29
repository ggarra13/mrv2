// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace io_tests
    {
        class STBTest : public tests::ITest
        {
        protected:
            STBTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<STBTest>
            create(const std::shared_ptr<system::Context>&);

            void run() override;

        private:
            void _io();
        };
    } // namespace io_tests
} // namespace tl
