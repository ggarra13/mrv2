// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTestLib/ITest.h>

namespace tl
{
    namespace core_tests
    {
        class TimeTest : public tests::ITest
        {
        protected:
            TimeTest(const std::shared_ptr<system::Context>&);

        public:
            static std::shared_ptr<TimeTest>
            create(const std::shared_ptr<system::Context>&);

            void run() override;

        private:
            void _otime();
            void _sleep();
            void _util();
            void _keycode();
            void _timecode();
            void _timer();
            void _serialize();
        };
    } // namespace core_tests
} // namespace tl
