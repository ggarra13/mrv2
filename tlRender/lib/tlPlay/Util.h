// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <string>

namespace tl
{
    //! Common functionality for the "tlplay" and "tlplay-qt" applications.
    namespace play
    {
        //! Get the path to the application documents directory. The directory
        //! is automatically created if it does not exist.
        std::string appDocsPath();

        //! Get the log file name.
        std::string
        logFileName(const std::string& appName, const std::string& appDirPath);

        //! Get the settings file name.
        std::string
        settingsName(const std::string& appName, const std::string& appDirPath);
    } // namespace play
} // namespace tl
