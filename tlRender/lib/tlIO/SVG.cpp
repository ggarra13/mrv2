// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/SVG.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace svg
    {

        void ReadPlugin::_init(const std::shared_ptr<log::System>& logSystem)
        {
            IReadPlugin::_init(
                "SVG",
                { { ".svg", io::FileType::Sequence } },
                logSystem);

            logSystem->print(
                "tl::svg::ReadPlugin",
                string::Format(
                    "\n"
                    "    * Formats: {0}").arg(".svg"));
        }

        std::shared_ptr<ReadPlugin>
        ReadPlugin::create(const std::shared_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<ReadPlugin>(new ReadPlugin);
            out->_init(logSystem);
            return out;
        }

        std::shared_ptr<io::IDecode>
        ReadPlugin::decode(const io::Options& options)
        {
            return Decode::create();
        }

        std::string ReadPlugin::getPluginInfo(const io::Options&) const
        {
            return "0.5";
        }

    } // namespace svg
} // namespace tl
