// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "mrvCore/mrvI8N.h"

#include "mrvBaseApp/mrvBaseApp.h"

#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <iostream>

namespace mrv
{
    namespace app
    {
        using namespace tl;

        std::vector<std::string> convert(int argc, char* argv[])
        {
            std::vector<std::string> out;
            for (int i = 0; i < argc; ++i)
            {
                out.push_back(argv[i]);
            }
            return out;
        }

        std::vector<std::string> convert(int argc, wchar_t* argv[])
        {
            std::vector<std::string> out;
            for (int i = 0; i < argc; ++i)
            {
                out.push_back(string::fromWide(argv[i]));
            }
            return out;
        }

        struct BaseApp::Private
        {
            struct CmdLineData
            {
                std::vector<std::string> argv;
                std::string name;
                std::string summary;
                std::vector<std::shared_ptr<ICmdLineArg> > args;
                std::vector<std::shared_ptr<ICmdLineOption> > options;
            };
            CmdLineData cmdLine;

            std::shared_ptr<observer::ListObserver<log::Item> > logObserver;
        };

        void BaseApp::_init(
            const std::vector<std::string>& argv,
            const std::shared_ptr<system::Context>& context,
            const std::string& cmdLineName, const std::string& cmdLineSummary,
            const std::vector<std::shared_ptr<ICmdLineArg> >& cmdLineArgs,
            const std::vector<std::shared_ptr<ICmdLineOption> >& cmdLineOptions)
        {
            TLRENDER_P();

            _context = context;

            // Parse the command line.
            for (size_t i = 1; i < argv.size(); ++i)
            {
                p.cmdLine.argv.push_back(argv[i]);
            }
            p.cmdLine.name = cmdLineName;
            p.cmdLine.summary = cmdLineSummary;
            p.cmdLine.args = cmdLineArgs;
            p.cmdLine.options = cmdLineOptions;
            p.cmdLine.options.push_back(CmdLineFlagOption::create(
                _options.log, {"-log"}, _("Print the log to the console.")));
            p.cmdLine.options.push_back(CmdLineFlagOption::create(
                _options.help, {"-help", "-h", "--help", "--h"},
                _("Show this message.")));
            _exit = _parseCmdLine();

            // Setup the log.
            if (_options.log)
            {
                p.logObserver = observer::ListObserver<log::Item>::create(
                    context->getSystem<log::System>()->observeLog(),
                    [this](const std::vector<log::Item>& value)
                    {
                        const size_t options =
                            static_cast<size_t>(log::StringConvert::Time) |
                            static_cast<size_t>(log::StringConvert::Prefix);
                        for (const auto& i : value)
                        {
                            _print("[LOG] " + toString(i, options));
                        }
                    },
                    observer::CallbackAction::Suppress);
            }
        }

        BaseApp::BaseApp() :
            _p(new Private)
        {
        }

        BaseApp::~BaseApp() {}

        const std::shared_ptr<system::Context>& BaseApp::getContext() const
        {
            return _context;
        }

        int BaseApp::getExit() const
        {
            return _exit;
        }

        const std::vector<std::string> BaseApp::getUnusedArgs() const
        {
            return _unusedArgs;
        }

        const std::string& BaseApp::_getCmdLineName() const
        {
            return _p->cmdLine.name;
        }

        void BaseApp::_log(const std::string& value, log::Type type)
        {
            _context->log(_p->cmdLine.name, value, type);
        }

        void BaseApp::_print(const std::string& value)
        {
            std::cout << value << std::endl;
        }

        void BaseApp::_printNewline()
        {
            std::cout << std::endl;
        }

        void BaseApp::_printError(const std::string& value)
        {
            std::cerr << value << std::endl;
        }

        int BaseApp::_parseCmdLine()
        {
            TLRENDER_P();
            for (const auto& i : p.cmdLine.options)
            {
                try
                {
                    if (!p.cmdLine.argv.empty())
                    {
                        i->parse(p.cmdLine.argv);
                    }
                }
                catch (const std::exception& e)
                {
                    /* xgettext:c-format */
                    throw std::runtime_error(
                        string::Format(_("Cannot parse option \"{0}\": {1}"))
                            .arg(i->getMatchedName())
                            .arg(e.what()));
                }
            }
            bool unusedArgs = false;
            size_t requiredArgs = 0;
            size_t optionalArgs = 0;
            for (const auto& i : p.cmdLine.args)
            {
                if (i->isUnused())
                {
                    unusedArgs = true;
                }
                else if (!i->isOptional())
                {
                    ++requiredArgs;
                }
                else
                {
                    ++optionalArgs;
                }
            }
            if (p.cmdLine.argv.size() < requiredArgs ||
                (p.cmdLine.argv.size() > requiredArgs + optionalArgs &&
                 !unusedArgs) ||
                _options.help)
            {
                _printCmdLineHelp();
                return 1;
            }
            for (const auto& i : p.cmdLine.args)
            {
                try
                {
                    if (!p.cmdLine.argv.empty())
                    {
                        if (i->isUnused())
                        {
                            _unusedArgs = p.cmdLine.argv;
                            break;
                        }
                        i->parse(p.cmdLine.argv);
                    }
                }
                catch (const std::exception& e)
                {
                    /* xgettext:c-format */
                    throw std::runtime_error(
                        string::Format(_("Cannot parse argument \"{0}\": {1}"))
                            .arg(i->getName())
                            .arg(e.what()));
                }
            }
            return 0;
        }

        void BaseApp::_printCmdLineHelp()
        {
            TLRENDER_P();
            _print("\n" + p.cmdLine.name + "\n");
            _print("    " + p.cmdLine.summary + "\n");
            _print(_("Usage:\n"));
            {
                std::stringstream ss;
                ss << "    " + p.cmdLine.name;
                if (p.cmdLine.args.size())
                {
                    std::vector<std::string> args;
                    for (const auto& i : p.cmdLine.args)
                    {
                        const bool optional = i->isOptional();
                        const bool unused = i->isUnused();
                        args.push_back(
                            (optional ? "[" : "(") +
                            string::toLower(i->getName()) +
                            (optional ? "]" : ")"));
                        if (unused)
                            args.push_back("...");
                    }
                    ss << " " << string::join(args, " ");
                }
                if (p.cmdLine.options.size())
                {
                    ss << _(" [option]...");
                }
                _print(ss.str());
                _printNewline();
            }
            _print(_("Arguments:\n"));
            for (const auto& i : p.cmdLine.args)
            {
                _print("    " + i->getName());
                _print("        " + i->getHelp());
                _printNewline();
            }
            _print(_("Options:\n"));
            for (const auto& i : p.cmdLine.options)
            {
                bool first = true;
                if (i->isTitle())
                {
                    for (const auto& j : i->getHelpText())
                    {
                        _print(j);
                    }
                    _printNewline();
                    continue;
                }
                for (const auto& j : i->getHelpText())
                {
                    if (first)
                    {
                        first = false;
                        _print("    " + j);
                    }
                    else
                    {
                        _print("        " + j);
                    }
                }
                _printNewline();
            }
        }
    } // namespace app
} // namespace mrv
