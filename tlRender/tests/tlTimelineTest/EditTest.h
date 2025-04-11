// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace timeline_tests
    {
        class EditTest : public tests::ITest
        {
        protected:
            EditTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<EditTest>
            create(const std::shared_ptr<system::Context>&);

            void run() override;

        private:
            void _move();
        };
    } // namespace timeline_tests
} // namespace tl
