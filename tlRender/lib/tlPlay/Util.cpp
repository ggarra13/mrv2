// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlay/Util.h>

#include <tlCore/File.h>
#include <tlCore/StringFormat.h>
#include <tlCore/Path.h>

namespace tl
{
    namespace play
    {
        std::string appDocsPath()
        {
            const std::string documentsPath =
                file::getUserPath(file::UserPath::Documents);
            if (!file::exists(documentsPath))
            {
                file::mkdir(documentsPath);
            }
            const std::string out = file::Path(documentsPath, "tlRender").get();
            if (!file::exists(out))
            {
                file::mkdir(out);
            }
            return out;
        }

        std::string
        logFileName(const std::string& appName, const std::string& appDirPath)
        {
            const std::string fileName = string::Format("{0}.{1}.log")
                                             .arg(appName)
                                             .arg(TLRENDER_VERSION);
            return file::Path(appDirPath, fileName).get();
        }

        std::string
        settingsName(const std::string& appName, const std::string& appDirPath)
        {
            const std::string fileName = string::Format("{0}.{1}.json")
                                             .arg(appName)
                                             .arg(TLRENDER_VERSION);
            return file::Path(appDirPath, fileName).get();
        }
    } // namespace play
} // namespace tl
