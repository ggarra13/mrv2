// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace timeline_tests
    {
        class CompareOptionsTest : public tl::tests::ITest
        {
        protected:
            CompareOptionsTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<CompareOptionsTest> create(const std::shared_ptr<system::Context>&);

            void run() override;
        };
    }
}
