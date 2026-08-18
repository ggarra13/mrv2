// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#pragma once

#include <tlCore/LogSystem.h>

#include <map>
#include <optional>

#include <minizip/mz.h>
#include <minizip/mz_strm.h>
#include <minizip/mz_zip.h>
#include <minizip/mz_zip_rw.h>

namespace tl
{
    namespace timeline
    {

        class ZipReader
        {
            TLRENDER_NON_COPYABLE(ZipReader);

            struct MZReaderDeleter
            {
                void operator()(void*) const;
            };
            using MZReaderPtr = std::unique_ptr<void, MZReaderDeleter>;

            struct MZEntryScope
            {
                MZEntryScope(void*);
                ~MZEntryScope();
                void* p = nullptr;
            };

        public:
            ZipReader(const std::shared_ptr<log::System>&);

            void open(
                const std::string& fileName,
                size_t fileSize);

            struct Entry { int64_t offset; int64_t size; };

            std::optional<Entry> find(const std::string& name) const;

            std::string readText(const std::string& name);

            //! Extract every entry under "media/" to a directory on disk,
            //! using the entry's file name (without its path inside the
            //! archive) as the file name on disk.
            void saveMedia(const std::string& outputDir = "/tmp/media") const;

        private:
            std::shared_ptr<log::System> _logSystem;
            std::string _fileName;
            size_t _fileSize = 0;
            MZReaderPtr _reader;
            std::map<std::string, Entry> _entries;
        };

    }
}
