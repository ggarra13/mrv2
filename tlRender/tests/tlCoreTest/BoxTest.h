// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the feather-tk project.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace core_tests
    {
        class BoxTest : public tests::ITest
        {
        protected:
            BoxTest(const std::shared_ptr<system::Context>&);

        public:
            virtual ~BoxTest();

            static std::shared_ptr<BoxTest> create(
                const std::shared_ptr<system::Context>&);

            void run() override;

        private:
            void _members();
            void _functions();
            void _operators();
            void _serialize();
        };
    }
}
