// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace core_tests
    {
        class StringTest : public tests::ITest
        {
        protected:
            StringTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<StringTest>
            create(const std::shared_ptr<system::Context>&);

            void run() override;

        private:
            void _split();
            void _format();
            void _compare();
            void _convert();
            void _escape();
        };
    } // namespace core_tests
} // namespace tl
