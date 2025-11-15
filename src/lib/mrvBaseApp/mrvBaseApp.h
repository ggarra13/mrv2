// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2024-2025 Gonzalo Garramuño
// All rights reserved.

#pragma once

#include <tlBaseApp/CmdLine.h>

#include <tlCore/Context.h>

namespace tl
{
    namespace app
    {
        class ICmdLineArg;
        class ICmdLineOption;
    } // namespace app
} // namespace tl

namespace mrv
{
    //! Base application
    namespace app
    {
        using namespace tl;
        using namespace tl::app;

        //! Application options.
        struct Options
        {
            bool log = false;
            bool help = false;
        };

        //! Convert command line arguments.
        std::vector<std::string> convert(int argc, char* argv[]);

        //! Convert command line arguments.
        std::vector<std::string> convert(int argc, wchar_t* argv[]);

        //! Base class for applications.
        class BaseApp : public std::enable_shared_from_this<BaseApp>
        {
            TLRENDER_NON_COPYABLE(BaseApp);

        protected:
            void _init(
                const std::vector<std::string>&,
                const std::shared_ptr<system::Context>&,
                const std::string& cmdLineName,
                const std::string& cmdLineSummary,
                const std::vector<std::shared_ptr<ICmdLineArg> >& = {},
                const std::vector<std::shared_ptr<ICmdLineOption> >& = {});

            BaseApp();

        public:
            virtual ~BaseApp() = 0;

            //! Get the context.
            const std::shared_ptr<system::Context>& getContext() const;

            //! Get the exit code.
            int getExit() const;

            //! Get unused arguments
            const std::vector<std::string> getUnusedArgs() const;

        protected:
            const std::string& _getCmdLineName() const;

            void _log(const std::string&, log::Type = log::Type::Message);

            void _print(const std::string&);
            void _printNewline();
            void _printError(const std::string&);

            std::vector<std::string> _unusedArgs;
            std::shared_ptr<system::Context> _context;
            Options _options;
            int _exit = 0;

        private:
            int _parseCmdLine();
            void _printCmdLineHelp();

            TLRENDER_PRIVATE();
        };
    } // namespace app
} // namespace mrv
