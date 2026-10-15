// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/ZFile.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <array>
#include <sstream>

namespace tl
{
    namespace zfile
    {

        void ReadPlugin::_init(const std::shared_ptr<log::System>& logSystem)
        {
            std::map<std::string, io::FileType> exts;
            exts[".z"] = io::FileType::Sequence;
            exts[".zfile"] = io::FileType::Sequence;
            IReadPlugin::_init("ZFILE", exts, logSystem);
        }

        std::shared_ptr<ReadPlugin> ReadPlugin::create(
            const std::shared_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<ReadPlugin>(new ReadPlugin);
            out->_init(logSystem);
            return out;
        }

        std::shared_ptr<io::IDecode> ReadPlugin::decode(const io::Options&)
        {
            return Decode::create();
        }

        std::string ReadPlugin::getPluginInfo(const io::Options&) const
        {
            return "ZFILE";
        }
    } // namespace zfile
} // namespace tl
