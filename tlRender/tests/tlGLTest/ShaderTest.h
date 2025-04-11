// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace gl_tests
    {
        class ShaderTest : public tests::ITest
        {
        protected:
            ShaderTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<ShaderTest>
            create(const std::shared_ptr<system::Context>&);

            void run() override;
        };
    } // namespace gl_tests
} // namespace tl
