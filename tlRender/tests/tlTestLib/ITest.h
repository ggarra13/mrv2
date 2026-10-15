// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Assert.h>
#include <tlCore/Context.h>
#include <tlCore/Util.h>

#include <filesystem>
#include <memory>
#include <string>

namespace tl
{
    namespace file
    {
        class TmpDir;
    }

    namespace tests
    {
        //! Base class for tests.
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

            //! Get the number of failures reported through _fail().
            size_t getFailureCount() const;

            virtual void run() = 0;

        protected:
            //! Get a temporary directory for this test's file output. The
            //! directory is created on first use and removed automatically when
            //! the test is destroyed.
            const std::filesystem::path& _getTempDir();

            template <typename T>
            void _enum(
                const std::string&, const std::function<std::vector<T>(void)>&);

            void _print(const std::string&);

            //! Report something the test tolerates: an expected exception, or
            //! a capability the environment does not have. This does not fail
            //! the test, which is why _fail() exists alongside it.
            void _error(const std::string&);

            //! Report a genuine failure, with a message saying what was
            //! expected and what happened. Use this where the message can say
            //! something the expression cannot -- the values that did not
            //! match, or which case failed. Where the expression is the whole
            //! story, TLRENDER_ASSERT is shorter and names the line for free.
            void _fail(const std::string&);

            //! Report a failed check. Use TLRENDER_ASSERT rather than calling this.
            void _checkFailed(
                const char* expression,
                const char* file,
                int line);

            std::shared_ptr<system::Context> _context;
            std::string _name;

        private:
            std::unique_ptr<file::TmpDir> _tmpDir;
            size_t _failureCount = 0;
        };
    } // namespace tests
} // namespace tl

#include <tlTestLib/ITestInline.h>
