// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <string>
#include <vector>

namespace tl
{
    //! Operating system
    namespace os
    {
        //! \name Information
        ///@{

        //! Operating system information.
        struct SystemInfo
        {
            std::string name;
            size_t cores = 0;
            size_t ram = 0;
            size_t ramGB = 0;
        };

        //! Get operating system information.
        SystemInfo getSystemInfo();

        ///@}

        //! \name Environment Variables
        ///@{

        //! Environment variable list separators.
        const std::vector<char> envListSeparators = {':', ';'};

        //! Environment variable list separator.
#if defined(_WINDOWS)
        const char envListSeparator = ';';
#else  // _WINDOWS
        const char envListSeparator = ':';
#endif // _WINDOWS

        //! Get an environment variable.
        //!
        //! Throws:
        //! - std::exception
        bool getEnv(const std::string& name, std::string&);

        //! Get an environment variable and convert it to an integer. If the
        //! variable is empty then zero is returned.
        //!
        //! Throws:
        //! - std::exception
        bool getEnv(const std::string& name, int& value);

        //! Get an environment variable and convert it to a list of strings.
        //!
        //! Throws:
        //! - std::exception
        bool getEnv(const std::string& name, std::vector<std::string>&);

        //! Set an environment variable.
        //!
        //! Throws:
        //! - std::exception
        bool setEnv(const std::string& name, const std::string&);

        //! Delete an environment variable.
        //!
        //! Throws:
        //! - std::exception
        bool delEnv(const std::string& name);

        ///@}
    } // namespace os
} // namespace tl
