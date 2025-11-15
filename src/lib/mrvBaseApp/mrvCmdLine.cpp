// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// Copyright (c) 2024-Present Gonzalo Garramuño
// All rights reserved.

#include "mrvBaseApp/mrvCmdLine.h"

namespace mrv
{
    namespace app
    {
        ICmdLineOption::ICmdLineOption(
            const std::vector<std::string>& names, const std::string& help) :
            _names(names),
            _help(help)
        {
        }

        ICmdLineOption::~ICmdLineOption() {}

        CmdLineHeader::CmdLineHeader(
            const std::vector<std::string>& names,
            const std::string& help) :
            ICmdLineOption(names, help)
        {
        }

        std::shared_ptr<CmdLineHeader> CmdLineHeader::create(
            const std::vector<std::string>& names,
            const std::string& help)
        {
            return std::shared_ptr<CmdLineHeader>(
                new CmdLineHeader(names, help));
        }

        std::vector<std::string> CmdLineHeader::getHelpText() const
        {
            std::vector<std::string> out;
            out.push_back(_help);
            return out;
        }
        
        CmdLineFlagOption::CmdLineFlagOption(
            bool& value, const std::vector<std::string>& names,
            const std::string& help) :
            ICmdLineOption(names, help),
            _value(value)
        {
        }

        std::shared_ptr<CmdLineFlagOption> CmdLineFlagOption::create(
            bool& value, const std::vector<std::string>& names,
            const std::string& help)
        {
            return std::shared_ptr<CmdLineFlagOption>(
                new CmdLineFlagOption(value, names, help));
        }

        void CmdLineFlagOption::parse(std::vector<std::string>& args)
        {
            for (const auto& name : _names)
            {
                auto i = std::find(args.begin(), args.end(), name);
                if (i != args.end())
                {
                    _matchedName = name;
                    _value = true;
                    i = args.erase(i);
                }
            }
        }

        std::vector<std::string> CmdLineFlagOption::getHelpText() const
        {
            std::vector<std::string> out;
            out.push_back(string::join(_names, ", "));
            out.push_back(_help);
            return out;
        }
    } // namespace app
} // namespace mrv
