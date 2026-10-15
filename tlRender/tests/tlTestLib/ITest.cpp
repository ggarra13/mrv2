// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTestLib/ITest.h>
#include <tlCore/Path.h>

#include <iostream>
#include <sstream>

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

        size_t ITest::getFailureCount() const
        {
            return _failureCount;
        }

        const std::filesystem::path& ITest::_getTempDir()
        {
            if (!_tmpDir)
            {
                _tmpDir = std::make_unique<file::TmpDir>();
            }
            return _tmpDir->getPath();
        }

        void ITest::_print(const std::string& value)
        {
            std::cout << _name << ": " << value << std::endl;
        }

        void ITest::_error(const std::string& value)
        {
            std::cout << _name << " ERROR: " << value << std::endl;
        }

        void ITest::_fail(const std::string& value)
        {
            ++_failureCount;
            std::cout << _name << " FAIL: " << value << std::endl;
        }

        void ITest::_checkFailed(
            const char* expression,
            const char* file,
            int line)
        {
            std::stringstream ss;
            ss << expression << " (" << file << ":" << line << ")";
            _fail(ss.str());
        }
    } // namespace tests
} // namespace tl
