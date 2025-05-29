// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Context.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace tl
{
    namespace tests
    {
        class ITest : public std::enable_shared_from_this<ITest>
        {
            TLRENDER_NON_COPYABLE(ITest);

        protected:
            ITest(
                const std::string& name,
                const std::shared_ptr<system::Context>&);

        public:
            virtual ~ITest() = 0;

            const std::string& getName() const;

            virtual void run() = 0;

        protected:
            template <typename T>
            void _enum(
                const std::string&, const std::function<std::vector<T>(void)>&);

            void _print(const std::string&);
            void _printError(const std::string&);

            std::shared_ptr<system::Context> _context;
            std::string _name;
        };
    } // namespace tests
} // namespace tl

#include <tlTestLib/ITestInline.h>
