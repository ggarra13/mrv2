// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace io_tests
    {
        class IOTest : public tl::tests::ITest
        {
        protected:
            IOTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<IOTest> create(const std::shared_ptr<system::Context>&);

            void run() override;

        private:
            void _videoData();
            void _ioSystem();
            void _decode();
            void _seqDecode();
            void _missingFrames();
            void _seqRange();
            void _structural();
        };
    }
}
