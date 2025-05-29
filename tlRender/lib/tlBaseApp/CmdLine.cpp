// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
// All rights reserved.

#include <tlBaseApp/CmdLine.h>

namespace tl
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
} // namespace tl
