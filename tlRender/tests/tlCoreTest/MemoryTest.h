// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace core_tests
    {
        class MemoryTest : public tests::ITest
        {
        protected:
            MemoryTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<MemoryTest>
            create(const std::shared_ptr<system::Context>&);

            void run() override;

        private:
            void _enums();
            void _endian();
            void _bits();
        };
    } // namespace core_tests
} // namespace tl
