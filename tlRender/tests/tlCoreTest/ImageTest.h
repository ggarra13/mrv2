// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace core_tests
    {
        class ImageTest : public tests::ITest
        {
        protected:
            ImageTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<ImageTest>
            create(const std::shared_ptr<system::Context>&);

            void run() override;

        private:
            void _size();
            void _enums();
            void _info();
            void _util();
            void _image();
            void _serialize();
        };
    } // namespace core_tests
} // namespace tl
