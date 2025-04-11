// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlAppTest/CmdLineTest.h>

#include <tlBaseApp/CmdLine.h>

using namespace tl::app;

namespace tl
{
    namespace app_tests
    {
        CmdLineTest::CmdLineTest(
            const std::shared_ptr<system::Context>& context) :
            ITest("AppTest::CmdLineTest", context)
        {
        }

        std::shared_ptr<CmdLineTest>
        CmdLineTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<CmdLineTest>(new CmdLineTest(context));
        }

        void CmdLineTest::run()
        {
            std::string input;
            std::string output;
            bool flag = false;
            int intValue = 0;
            std::string string = "moo";

            auto inputArg = CmdLineValueArg<std::string>::create(
                input, "input", "This is help for the input argument.");
            auto outputArg = CmdLineValueArg<std::string>::create(
                output, "output", "This is help for the output argument.",
                true);
            auto flagOption = CmdLineFlagOption::create(
                flag, {"-flag", "-f"}, "This is help for the flag option.");
            auto intOption = CmdLineValueOption<int>::create(
                intValue, {"-int", "-i"}, "This is help for the int option.");
            auto stringOption = CmdLineValueOption<std::string>::create(
                string, {"-string", "-s"},
                "This is help for the string option.", "moo", "moo, omo, oom");
            const std::vector<std::shared_ptr<ICmdLineArg> > args = {
                inputArg, outputArg};
            const std::vector<std::shared_ptr<ICmdLineOption> > options = {
                flagOption, intOption, stringOption};
            const std::vector<std::shared_ptr<ICmdLineOption> > valueOptions = {
                intOption, stringOption};
            for (const auto& i : args)
            {
                const std::pair<char, char> c = std::make_pair(
                    i->isOptional() ? '[' : '(', i->isOptional() ? ']' : ')');
                _print(c.first + i->getName() + c.second);
                _print("    " + i->getHelp());
            }
            for (const auto& i : options)
            {
                std::string indent;
                for (const auto& j : i->getHelpText())
                {
                    _print(indent + j);
                    indent = "    ";
                }
            }

            {
                std::vector<std::string> cmdLine = {
                    "input", "output", "-flag", "-int", "1", "-string", "omo"};
                for (const auto& i : options)
                {
                    i->parse(cmdLine);
                    const std::string s = i->getMatchedName();
                    if (!s.empty())
                    {
                        std::stringstream ss;
                        ss << "Matched: " << s;
                        _print(ss.str());
                    }
                }
                for (const auto& i : args)
                {
                    i->parse(cmdLine);
                }
                TLRENDER_ASSERT(flag);
                TLRENDER_ASSERT(1 == intValue);
                TLRENDER_ASSERT("omo" == string);
                TLRENDER_ASSERT("input" == input);
                TLRENDER_ASSERT("output" == output);
            }

            flag = false;
            intValue = 0;
            string = "moo";
            {
                std::vector<std::string> cmdLine = {
                    "-f", "-i", "2", "-s", "oom"};
                for (const auto& i : options)
                {
                    i->parse(cmdLine);
                    const std::string s = i->getMatchedName();
                    if (!s.empty())
                    {
                        std::stringstream ss;
                        ss << "Matched: " << s;
                        _print(ss.str());
                    }
                }
                TLRENDER_ASSERT(flag);
                TLRENDER_ASSERT(2 == intValue);
                TLRENDER_ASSERT("oom" == string);
            }

            {
                std::vector<std::string> cmdLine;
                try
                {
                    inputArg->parse(cmdLine);
                    TLRENDER_ASSERT(false);
                }
                catch (const std::exception&)
                {
                }
            }
            {
                std::vector<std::string> cmdLine = {"-int"};
                try
                {
                    intOption->parse(cmdLine);
                    TLRENDER_ASSERT(false);
                }
                catch (const std::exception&)
                {
                }
            }
            {
                std::vector<std::string> cmdLine = {"-string"};
                try
                {
                    stringOption->parse(cmdLine);
                    TLRENDER_ASSERT(false);
                }
                catch (const std::exception&)
                {
                }
            }
        }
    } // namespace app_tests
} // namespace tl
