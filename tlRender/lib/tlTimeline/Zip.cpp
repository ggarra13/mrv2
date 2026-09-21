// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlTimeline/ZipPrivate.h>

#include <tlCore/FileIO.h>
#include <tlCore/StringFormat.h>

#include <algorithm>
#include <array>
#include <filesystem>
#include <fstream>
#include <vector>

namespace tl
{
    namespace
    {
        uint16_t readLE16(const uint8_t* p)
        {
            return
                static_cast<uint16_t>(p[0]) |
                (static_cast<uint16_t>(p[1]) << 8);
        }

        uint32_t readLE32(const uint8_t* p)
        {
            return
                static_cast<uint32_t>(p[0]) |
                (static_cast<uint32_t>(p[1]) << 8) |
                (static_cast<uint32_t>(p[2]) << 16) |
                (static_cast<uint32_t>(p[3]) << 24);
        }

        constexpr uint32_t zipHeaderMagic = 0x04034b50u;
        constexpr size_t zipHeaderNameOffset = 26;
        constexpr size_t zipHeaderExtraLenOffset = 28;
        constexpr size_t zipHeaderSize = 30;

        //! The general purpose flag saying a data descriptor follows the data.
        constexpr uint16_t zipFlagDataDescriptor = 0x8;

        //! The extra field is the only unknown between a local header and its
        //! data, and it is length prefixed with sixteen bits.
        constexpr int64_t zipMaxExtraSize = 65535;

        //! The largest size a thirty-two bit field can hold. An entry above
        //! it needs sixty-four bit sizes, which makes its data descriptor
        //! eight bytes longer.
        constexpr int64_t zipMaxU32 = 0xFFFFFFFF;

        //! Entries are grouped by what follows their data, since that is what
        //! the derivation has to account for.
        enum Group
        {
            GroupNone,      // nothing between the data and the next header
            GroupTrailer,   // a data descriptor with thirty-two bit sizes
            GroupTrailer64, // a data descriptor with sixty-four bit sizes
            GroupCount
        };

        //! The largest data descriptor: a signature, a CRC and two eight
        //! byte sizes.
        constexpr int64_t zipMaxTrailerSize = 24;

        //! How many entries to measure against the file.
        //!
        //! Measuring every one would be the scattered read per entry that
        //! deriving them exists to avoid. A bundle is written by one writer in
        //! one pass, so entries are laid out the same way throughout, and a
        //! sample spread across the file sees a writer that does otherwise.
        constexpr size_t zipVerifySamples = 64;

        //! Read a local header to get where an entry's data actually starts.
        int64_t readDataOffset(
            const std::shared_ptr<file::FileIO>& io,
            const std::string& fileName,
            int64_t headerOffset)
        {
            uint8_t hdr[zipHeaderSize];
            io->readAt(hdr, headerOffset, zipHeaderSize);
            if (readLE32(hdr) != zipHeaderMagic)
            {
                throw std::runtime_error(string::Format(
                                             "Bad local zip header: \"{0}\"").arg(fileName));
            }
            return
                headerOffset +
                zipHeaderSize +
                readLE16(hdr + zipHeaderNameOffset) +
                readLE16(hdr + zipHeaderExtraLenOffset);
        }
    }

    namespace timeline
    {

        void ZipReader::MZReaderDeleter::operator()(void* p) const
        {
            if (p) mz_zip_reader_delete(&p);
        }

        ZipReader::MZEntryScope::MZEntryScope(void* p) :
            p(p)
        {}

        ZipReader::MZEntryScope::~MZEntryScope()
        {
            if (p) mz_zip_reader_entry_close(p);
        }

        ZipReader::ZipReader(const std::shared_ptr<log::System>& logSystem) :
            _logSystem(logSystem)
        {}

        void ZipReader::open(
            const std::string& fileName,
            size_t fileSize)
        {
            if (_reader)
            {
                _reader.reset();
                _entries.clear();
            }

            _fileName = fileName;
            _fileSize = fileSize;

            _reader.reset(mz_zip_reader_create());
            if (!_reader.get())
            {
                throw std::runtime_error(string::Format(
                                             "Cannot create zip reader: \"{0}\"").arg(fileName));
            }
            int32_t err = mz_zip_reader_open_file(_reader.get(), fileName.c_str());
            if (err != MZ_OK)
            {
                throw std::runtime_error(string::Format(
                                             "Cannot open zip reader: \"{0}\"").arg(fileName));
            }

            err = mz_zip_reader_goto_first_entry(_reader.get());
            if (err != MZ_OK)
            {
                throw std::runtime_error(string::Format(
                                             "Cannot goto first zip entry: \"{0}\"").arg(fileName));
            }

            // Collect the central directory, which minizip has already read as one
            // contiguous region at the end of the file. Nothing here touches the
            // rest of the bundle.
            struct Record
            {
                std::string name;
                int64_t     headerOffset = 0;
                int64_t     size = 0;
                int64_t     minDataOffset = 0;
                bool        trailer = false;
                int64_t     dataOffset = -1;
                int         group = GroupNone;
            };
            std::vector<Record> records;
            // Where every entry's local header starts, including the entries this
            // reader cannot use. An entry's data ends at the next header
            // whatever kind of entry that is, so leaving out the compressed ones
            // would put the boundary past them and derive an offset that is
            // wrong by their size -- small enough, for a small entry, to pass the
            // bounds check below.
            std::vector<int64_t> boundaries;
            bool derivable = true;
            while (MZ_OK == err)
            {
                mz_zip_file* fileInfo = nullptr;
                err = mz_zip_reader_entry_get_info(_reader.get(), &fileInfo);
                if (err != MZ_OK || !fileInfo)
                {
                    throw std::runtime_error(string::Format(
                                                 "Cannot get zip entry information: \"{0}\"").arg(fileName));
                }
                if (fileInfo->disk_offset >= 0 &&
                    static_cast<size_t>(fileInfo->disk_offset) + zipHeaderSize <= _fileSize)
                {
                    boundaries.push_back(fileInfo->disk_offset);
                }
                if (fileInfo->disk_number != 0)
                {
                    // Split across volumes, so an offset is relative to a disk
                    // rather than to this file.
                    derivable = false;
                }
                if (mz_zip_reader_entry_is_dir(_reader.get()) != MZ_OK &&
                    0 == fileInfo->compression_method)
                {
                    if (fileInfo->disk_offset < 0 ||
                        static_cast<size_t>(fileInfo->disk_offset) + zipHeaderSize > _fileSize)
                    {
                        throw std::runtime_error(string::Format(
                                                     "Local zip header entry out of bounds: \"{0}\"").arg(fileName));
                    }
                    Record record;
                    record.name          = fileInfo->filename;
                    record.headerOffset  = fileInfo->disk_offset;
                    record.size          = fileInfo->uncompressed_size;
                    record.minDataOffset =
                        fileInfo->disk_offset + zipHeaderSize + fileInfo->filename_size;
                    record.trailer       = (fileInfo->flag & zipFlagDataDescriptor) != 0;
                    records.push_back(record);
                }
                err = mz_zip_reader_goto_next_entry(_reader.get());
                if (err != MZ_OK && err != MZ_END_OF_LIST)
                {
                    throw std::runtime_error(string::Format(
                                                 "Cannot goto next zip entry: \"{0}\"").arg(fileName));
                }
            }

            std::sort(
                records.begin(),
                records.end(),
                [](const Record& a, const Record& b)
                    {
                        return a.headerOffset < b.headerOffset;
                    });

            // Where an entry's data starts is only recorded in its local header,
            // and those sit one before each file's data, spread across the whole
            // bundle. Reading them all is what made opening a large bundle take
            // minutes: measured on Windows, 25,000 headers of thirty bytes each
            // pulled tens of gigabytes from disk at 0% CPU.
            //
            // They do not have to be read. An entry's data ends where the next
            // entry's local header begins, so the offset follows from the size in
            // the central directory. That leaves the last entry, and any entry
            // whose data is followed by a descriptor, to be read.
            auto io = file::FileIO::create(
                fileName,
                file::Mode::Read,
                file::Read::Normal,
                file::Access::Random);
            size_t readCount = 0;

            // How many bytes sit between an entry's data and the next local
            // header. Nothing, for an entry written to a file. An entry written
            // to a stream is followed by a data descriptor, whose size depends on
            // the writer and on whether that entry needed zip64 -- so rather than
            // guess it, measure it. A handful of entries say what the whole
            // archive does. Measured on a bundle of 25,099 streamed entries the
            // gap is sixteen bytes throughout.
            std::sort(boundaries.begin(), boundaries.end());
            const auto nextBoundary = [&boundaries](int64_t headerOffset) -> int64_t
                {
                    const auto i = std::upper_bound(
                        boundaries.begin(), boundaries.end(), headerOffset);
                    return i != boundaries.end() ? *i : -1;
                };
            // Which descriptor a streamed entry gets follows from its own size,
            // so group by that rather than hoping a sample lands on the few large
            // entries in an archive of mostly small ones.
            std::array<std::vector<size_t>, GroupCount> groups;
            for (size_t i = 0; i < records.size(); ++i)
            {
                records[i].group = !records[i].trailer ?
                                   GroupNone :
                                   (records[i].size > zipMaxU32 ? GroupTrailer64 : GroupTrailer);
                if (nextBoundary(records[i].headerOffset) >= 0)
                {
                    groups[records[i].group].push_back(i);
                }
            }
            const auto measureGap =
                [&](const std::vector<size_t>& group, int64_t& gap) -> bool
                    {
                        if (group.empty())
                        {
                            return true;
                        }
                        const size_t count = std::min(zipVerifySamples, group.size());
                        for (size_t s = 0; s < count; ++s)
                        {
                            const size_t i = group[s * group.size() / count];
                            const int64_t dataOffset =
                                readDataOffset(io, fileName, records[i].headerOffset);
                            ++readCount;
                            const int64_t measured =
                                nextBoundary(records[i].headerOffset) -
                                records[i].size -
                                dataOffset;
                            if (measured < 0 || measured > zipMaxTrailerSize)
                            {
                                return false;
                            }
                            if (0 == s)
                            {
                                gap = measured;
                            }
                            else if (measured != gap)
                            {
                                return false;
                            }
                        }
                        return true;
                    };
            std::array<int64_t, GroupCount> gaps = { 0, 0, 0 };
            for (int i = 0; i < GroupCount && derivable; ++i)
            {
                derivable = measureGap(groups[i], gaps[i]);
            }
            if (derivable)
            {
                for (size_t i = 0; i < records.size(); ++i)
                {
                    Record& record = records[i];
                    const int64_t next = nextBoundary(record.headerOffset);
                    if (next < 0)
                    {
                        continue;
                    }
                    const int64_t derived = next - record.size - gaps[record.group];
                    if (derived >= record.minDataOffset &&
                        derived - record.minDataOffset <= zipMaxExtraSize)
                    {
                        record.dataOffset = derived;
                    }
                }
            }
            else
            {
                // The archive is not laid out the way the derivation describes,
                // so every offset has to come from its own header.
                _logSystem->print("tl::ZipReader", string::Format(
                                      "Zip entry offsets cannot be derived, reading {0} local "
                                      "headers: \"{1}\"").arg(records.size()).arg(fileName),
                                  log::Type::Warning);
            }

            // Whatever is left: the last entry, which has no successor, and any
            // entry the bounds check rejected.
            for (auto& record : records)
            {
                if (record.dataOffset < 0)
                {
                    record.dataOffset =
                        readDataOffset(io, fileName, record.headerOffset);
                    ++readCount;
                }
            }

            for (const auto& record : records)
            {
                if (record.dataOffset < 0 ||
                    static_cast<size_t>(record.dataOffset) > _fileSize ||
                    record.size < 0 ||
                    static_cast<size_t>(record.size) > _fileSize - record.dataOffset)
                {
                    throw std::runtime_error(string::Format(
                                                 "Local zip entry out of bounds: \"{0}\"").arg(fileName));
                }
                Entry entry{ record.dataOffset, record.size };
                if (!_entries.emplace(record.name, entry).second)
                {
                    _logSystem->print("tl::ZipReader", string::Format(
                                          "Duplicate zip entry, ignoring subsequent: \"{0}\"").arg(record.name),
                                      log::Type::Warning);
                }
            }

            _logSystem->print("tl::ZipReader", string::Format(
                                  "Opened \"{0}\": {1} entries, {2} local headers read").
                              arg(fileName).arg(records.size()).arg(readCount),
                              log::Type::Message);
        }

        std::optional<ZipReader::Entry> ZipReader::find(const std::string& name) const
        {
            const auto i = _entries.find(name);
            return i != _entries.end() ? std::optional<Entry>(i->second) : std::nullopt;
        }

        std::string ZipReader::readText(const std::string& name)
        {
            int32_t err = mz_zip_reader_locate_entry(
                _reader.get(),
                name.c_str(),
                0);
            if (err != MZ_OK)
            {
                throw std::runtime_error(string::Format(
                                             "Cannot find zip entry: \"{0}\"").arg(name));
            }
            err = mz_zip_reader_entry_open(_reader.get());
            if (err != MZ_OK)
            {
                throw std::runtime_error(string::Format(
                                             "Cannot open zip entry: \"{0}\"").arg(name));
            }
            MZEntryScope entry(_reader.get());
            mz_zip_file* fileInfo = nullptr;
            err = mz_zip_reader_entry_get_info(_reader.get(), &fileInfo);
            if (err != MZ_OK || !fileInfo)
            {
                throw std::runtime_error(string::Format(
                                             "Cannot get zip entry information: \"{0}\"").arg(name));
            }
            if (fileInfo->uncompressed_size > INT32_MAX)
            {
                throw std::runtime_error(string::Format(
                                             "Text zip entry exceeds max size: \"{0}\"").arg(name));
            }
            std::string out(fileInfo->uncompressed_size, 0);
            err = mz_zip_reader_entry_read(
                _reader.get(),
                out.data(),
                fileInfo->uncompressed_size);
            if (err != fileInfo->uncompressed_size)
            {
                throw std::runtime_error(string::Format(
                                             "Cannot read zip entry: \"{0}\"").arg(name));
            }
            return out;
        }

        void ZipReader::saveMedia(const std::string& outputDir) const
        {
            namespace fs = std::filesystem;

            std::error_code ec;
            fs::create_directories(outputDir, ec);
            if (ec)
            {
                throw std::runtime_error(string::Format(
                                             "Cannot create media directory: \"{0}\"").arg(outputDir));
            }

            // Entries were already located and bounds-checked in open(), so
            // this is a plain read of the raw (stored, uncompressed) bytes
            // at each entry's derived offset.
            auto io = file::FileIO::create(
                _fileName,
                file::Mode::Read,
                file::Read::Normal,
                file::Access::Random);

            static const std::string mediaPrefix = "media/";
            std::vector<uint8_t> buffer;
            size_t savedCount = 0;
            for (const auto& i : _entries)
            {
                const std::string& name = i.first;
                if (name.compare(0, mediaPrefix.size(), mediaPrefix) != 0)
                {
                    // Not a media entry, e.g. the bundle's "content.otio".
                    continue;
                }
                const Entry& entry = i.second;

                // Use just the file name, discarding any directory
                // structure the entry had inside the archive.
                const size_t slash = name.find_last_of('/');
                const std::string outName = slash != std::string::npos ?
                                            name.substr(slash + 1) :
                                            name;
                if (outName.empty())
                {
                    // A directory entry, nothing to write.
                    continue;
                }

                const fs::path outPath = fs::path(outputDir) / outName;

                buffer.resize(static_cast<size_t>(entry.size));
                if (entry.size > 0)
                {
                    io->readAt(buffer.data(), entry.offset, static_cast<size_t>(entry.size));
                }

                std::ofstream out(outPath, std::ios::binary | std::ios::trunc);
                if (!out)
                {
                    throw std::runtime_error(string::Format(
                                                 "Cannot write media file: \"{0}\"").arg(outPath.string()));
                }
                if (entry.size > 0)
                {
                    out.write(reinterpret_cast<const char*>(buffer.data()), entry.size);
                }
                if (!out)
                {
                    throw std::runtime_error(string::Format(
                                                 "Cannot write media file: \"{0}\"").arg(outPath.string()));
                }
                out.close();
                ++savedCount;

                _logSystem->print("tl::ZipReader", string::Format(
                                      "Saved zip entry \"{0}\" to \"{1}\" ({2} bytes)").
                                  arg(name).arg(outPath.string()).arg(entry.size),
                                  log::Type::Message);
            }

            _logSystem->print("tl::ZipReader", string::Format(
                                  "Saved {0} media entries to \"{1}\"").
                              arg(savedCount).arg(outputDir),
                              log::Type::Message);
        }
    }
}
