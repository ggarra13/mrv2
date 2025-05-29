// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCoreTest/FileTest.h>

#include <tlCore/Assert.h>
#include <tlCore/File.h>
#include <tlCore/FileIO.h>
#include <tlCore/StringFormat.h>

using namespace tl::file;

namespace tl
{
    namespace core_tests
    {
        FileTest::FileTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::FileTest", context)
        {
        }

        std::shared_ptr<FileTest>
        FileTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<FileTest>(new FileTest(context));
        }

        void FileTest::run()
        {
            _file();
            _dir();
            _temp();
        }

        void FileTest::_file()
        {
            const std::string fileName = "File Test";
            {
                FileIO::create(fileName, Mode::Write);
            }
            TLRENDER_ASSERT(exists(fileName));
            TLRENDER_ASSERT(rm(fileName));
        }

        void FileTest::_dir()
        {
            {
                bool r = mkdir("File Test");
                TLRENDER_ASSERT(r);
                r = mkdir("File Test");
                TLRENDER_ASSERT(!r);
                r = rmdir("File Test");
                TLRENDER_ASSERT(r);
                r = rmdir("File Test");
                TLRENDER_ASSERT(!r);
            }
            {
                _print(string::Format("CWD: {0}").arg(getCWD()));
            }
        }

        void FileTest::_temp()
        {
            {
                std::stringstream ss;
                ss << "Temp dir: " << getTemp();
                _print(ss.str());
            }
            {
                std::stringstream ss;
                ss << "Temp dir: " << createTempDir();
                _print(ss.str());
            }
        }
    } // namespace core_tests
} // namespace tl
