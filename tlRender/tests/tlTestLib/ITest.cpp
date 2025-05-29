// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTestLib/ITest.h>

#include <iostream>

namespace tl
{
    namespace tests
    {
        ITest::ITest(
            const std::string& name,
            const std::shared_ptr<system::Context>& context) :
            _context(context),
            _name(name)
        {
        }

        ITest::~ITest() {}

        const std::string& ITest::getName() const
        {
            return _name;
        }

        void ITest::_print(const std::string& value)
        {
            std::cout << "    " << value << std::endl;
        }

        void ITest::_printError(const std::string& value)
        {
            std::cout << "    ERROR: " << value << std::endl;
        }
    } // namespace tests
} // namespace tl
