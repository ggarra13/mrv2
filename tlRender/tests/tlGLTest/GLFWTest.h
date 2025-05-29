// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace gl_tests
    {
        class GLFWTest : public tests::ITest
        {
        protected:
            GLFWTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<GLFWTest>
            create(const std::shared_ptr<system::Context>&);

            void run() override;
        };
    } // namespace gl_tests
} // namespace tl
