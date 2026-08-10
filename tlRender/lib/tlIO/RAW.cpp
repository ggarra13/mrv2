// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/RAW.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <array>
#include <sstream>

namespace tl
{
    namespace raw
    {

        void ReadPlugin::_init(const std::shared_ptr<log::System>& logSystem)
        {
            IReadPlugin::_init(
                "RAW", {{".3fr", io::FileType::Sequence},
                        {".arw", io::FileType::Sequence},
                        {".bay", io::FileType::Sequence},
                        {".bmq", io::FileType::Sequence},
                        {".cap", io::FileType::Sequence},
                        {".cine", io::FileType::Sequence},
                        {".cr2", io::FileType::Sequence},
                        {".cr3", io::FileType::Sequence},
                        {".crw", io::FileType::Sequence},
                        {".cs1", io::FileType::Sequence},
                        {".dc2", io::FileType::Sequence},
                        {".dcr", io::FileType::Sequence},
                        {".dng", io::FileType::Sequence},
                        {".drf", io::FileType::Sequence},
                        {".dsc", io::FileType::Sequence},
                        {".erf", io::FileType::Sequence},
                        {".fff", io::FileType::Sequence},
                        {".ia", io::FileType::Sequence},
                        {".iiq", io::FileType::Sequence},
                        {".k25", io::FileType::Sequence},
                        {".kc2", io::FileType::Sequence},
                        {".kdc", io::FileType::Sequence},
                        {".mdc", io::FileType::Sequence},
                        {".mef", io::FileType::Sequence},
                        {".mos", io::FileType::Sequence},
                        {".mrw", io::FileType::Sequence},
                        {".nef", io::FileType::Sequence},
                        {".nrw", io::FileType::Sequence},
                        {".orf", io::FileType::Sequence},
                        {".pef", io::FileType::Sequence},
                        {".ptx", io::FileType::Sequence},
                        {".pxn", io::FileType::Sequence},
                        {".qtk", io::FileType::Sequence},
                        {".raf", io::FileType::Sequence},
                        {".raw", io::FileType::Sequence},
                        {".rdc", io::FileType::Sequence},
                        {".rw2", io::FileType::Sequence},
                        {".rwl", io::FileType::Sequence},
                        {".rwz", io::FileType::Sequence},
                        {".sr2", io::FileType::Sequence},
                        {".srf", io::FileType::Sequence},
                        {".srw", io::FileType::Sequence},
                        {".sti", io::FileType::Sequence},
                        {".x3f", io::FileType::Sequence}},
                logSystem);
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
            return "RAW";
        }
    } // namespace raw
} // namespace tl
