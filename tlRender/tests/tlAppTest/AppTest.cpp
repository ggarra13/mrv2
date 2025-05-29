// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlAppTest/AppTest.h>

#include <tlBaseApp/BaseApp.h>

#include <tlCore/FileInfo.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Time.h>

#include <cstring>

#include <wchar.h>

using namespace tl::app;

namespace tl
{
    namespace app_tests
    {
        AppTest::AppTest(const std::shared_ptr<system::Context>& context) :
            ITest("AppTest::AppTest", context)
        {
        }

        std::shared_ptr<AppTest>
        AppTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<AppTest>(new AppTest(context));
        }

        void AppTest::run()
        {
            _convert();
            _app();
        }

        void AppTest::_convert()
        {
            {
                const std::vector<std::string> s = {"app", "arg1", "arg2"};
                char** argv = nullptr;
                argv = new char*[3];
                argv[0] = new char[4];
                argv[1] = new char[5];
                argv[2] = new char[5];
                strcpy(argv[0], "app");
                strcpy(argv[1], "arg1");
                strcpy(argv[2], "arg2");
                TLRENDER_ASSERT(s == convert(s.size(), argv));
                delete[] argv[0];
                delete[] argv[1];
                delete[] argv[2];
                delete[] argv;
            }
            {
                const std::vector<std::string> s = {"app", "arg1", "arg2"};
                wchar_t** argv = nullptr;
                argv = new wchar_t*[3];
                argv[0] = new wchar_t[4];
                argv[1] = new wchar_t[5];
                argv[2] = new wchar_t[5];
                wcscpy(argv[0], L"app");
                wcscpy(argv[1], L"arg1");
                wcscpy(argv[2], L"arg2");
                TLRENDER_ASSERT(s == convert(s.size(), argv));
                delete[] argv[0];
                delete[] argv[1];
                delete[] argv[2];
                delete[] argv;
            }
        }

        namespace
        {
            class App : public BaseApp
            {
                void _init(
                    const std::vector<std::string>& args,
                    const std::shared_ptr<system::Context>& context)
                {
                    BaseApp::_init(
                        args, context, "test", "Test application.",
                        {CmdLineValueArg<file::Type>::create(
                             _input, "input",
                             "This is help for the input argument."),
                         CmdLineValueArg<std::string>::create(
                             _output, "output",
                             "This is help for the output argument.", true)},
                        {CmdLineValueOption<int>::create(
                             _intOption, {"-int"},
                             "This is the help for the option."),
                         CmdLineValueOption<file::ListSort>::create(
                             _listSortOption, {"-listSort", "-ls"},
                             "This is the help for the option.")});

                    _log("Log test");

                    _printError("Error test");
                }

            public:
                static std::shared_ptr<App> create(
                    const std::vector<std::string>& args,
                    const std::shared_ptr<system::Context>& context)
                {
                    auto out = std::shared_ptr<App>(new App);
                    out->_init(args, context);
                    return out;
                }

                file::Type getInput() const { return _input; }
                const std::string& getOutput() const { return _output; }
                int getIntOption() const { return _intOption; }
                file::ListSort getListSortOption() const
                {
                    return _listSortOption;
                }

            private:
                file::Type _input = file::Type::First;
                std::string _output;
                int _intOption = 0;
                file::ListSort _listSortOption = file::ListSort::First;
            };
        } // namespace

        void AppTest::_app()
        {
            {
                auto app = App::create({"app"}, _context);
                TLRENDER_ASSERT(app->getContext());
                TLRENDER_ASSERT(1 == app->getExit());
            }
            {
                auto app = App::create({"app", "-h"}, _context);
                TLRENDER_ASSERT(1 == app->getExit());
            }
            {
                auto app = App::create(
                    {"app", "directory", "output", "-int", "10", "-listSort",
                     "Extension"},
                    _context);
                TLRENDER_ASSERT(0 == app->getExit());
                TLRENDER_ASSERT(file::Type::Directory == app->getInput());
                TLRENDER_ASSERT("output" == app->getOutput());
                TLRENDER_ASSERT(10 == app->getIntOption());
                TLRENDER_ASSERT(
                    file::ListSort::Extension == app->getListSortOption());
            }
            {
                auto app = App::create({"app", "directory", "-log"}, _context);
                for (size_t i = 0; i < 3; ++i)
                {
                    _context->log(
                        "AppTest::_app", string::Format("Tick: {0}").arg(i));
                    _context->tick();
                    time::sleep(std::chrono::milliseconds(1000));
                }
                TLRENDER_ASSERT(0 == app->getExit());
            }
            try
            {
                auto app = App::create({"app", "input"}, _context);
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {
            }
            try
            {
                auto app = App::create({"app", "input", "-int"}, _context);
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {
            }
            try
            {
                auto app = App::create({"app", "input", "-listSort"}, _context);
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {
            }
        }
    } // namespace app_tests
} // namespace tl
