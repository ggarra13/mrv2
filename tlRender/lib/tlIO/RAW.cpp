// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2023 Darby Johnston
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
        Plugin::Plugin() {}

        std::shared_ptr<Plugin> Plugin::create(
            const std::shared_ptr<io::Cache>& cache,
            const std::weak_ptr<log::System>& logSystem)
        {
            auto out = std::shared_ptr<Plugin>(new Plugin);
            out->_init(
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
                cache, logSystem);
            return out;
        }

        std::shared_ptr<io::IRead>
        Plugin::read(const file::Path& path, const io::Options& options)
        {
            return Read::create(path, options, _cache, _logSystem);
        }

        std::shared_ptr<io::IRead> Plugin::read(
            const file::Path& path, const std::vector<file::MemoryRead>& memory,
            const io::Options& options)
        {
            return Read::create(path, memory, options, _cache, _logSystem);
        }
    } // namespace raw
} // namespace tl
