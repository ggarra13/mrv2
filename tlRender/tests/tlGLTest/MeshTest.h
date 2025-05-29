// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace gl_tests
    {
        class MeshTest : public tests::ITest
        {
        protected:
            MeshTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<MeshTest>
            create(const std::shared_ptr<system::Context>&);

            void run() override;

        private:
            void _enums();
            void _convert();
            void _mesh();
        };
    } // namespace gl_tests
} // namespace tl
