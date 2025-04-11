// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace core_tests
    {
        class FileIOTest : public tests::ITest
        {
        protected:
            FileIOTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<FileIOTest>
            create(const std::shared_ptr<system::Context>&);

            void run() override;

        private:
            void _enums();
            void _tests();

            std::string _fileName;
            std::string _text;
            std::string _text2;
        };
    } // namespace core_tests
} // namespace tl
