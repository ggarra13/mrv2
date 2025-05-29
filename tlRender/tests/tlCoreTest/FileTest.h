// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace core_tests
    {
        class FileTest : public tests::ITest
        {
        protected:
            FileTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<FileTest>
            create(const std::shared_ptr<system::Context>&);

            void run() override;

        private:
            void _file();
            void _dir();
            void _temp();
        };
    } // namespace core_tests
} // namespace tl
