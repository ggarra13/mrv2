// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace timeline_tests
    {
        class UtilTest : public tests::ITest
        {
        protected:
            UtilTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<UtilTest>
            create(const std::shared_ptr<system::Context>&);

            void run() override;

        private:
            void _enums();
            void _extensions();
            void _ranges();
            void _util();
            void _otioz();
        };
    } // namespace timeline_tests
} // namespace tl
