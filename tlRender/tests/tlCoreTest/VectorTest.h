// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace core_tests
    {
        class VectorTest : public tests::ITest
        {
        protected:
            VectorTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<VectorTest>
            create(const std::shared_ptr<system::Context>&);

            void run() override;

        private:
            void _ctors();
            void _components();
            void _operators();
            void _serialize();
        };
    } // namespace core_tests
} // namespace tl
