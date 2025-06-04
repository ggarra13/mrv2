// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/Util.h>

#include <tlTimeline/MemoryReference.h>

#include <tlIO/System.h>

#include <tlCore/Assert.h>
#include <tlCore/Error.h>
#include <tlCore/FileInfo.h>
#include <tlCore/PathMapping.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <opentimelineio/clip.h>
#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>

#include <ctime>

#include <mz.h>
#include <mz_os.h>
#include <mz_strm.h>
#include <mz_zip.h>
#include <mz_zip_rw.h>

namespace tl
{
    namespace timeline
    {
        std::vector<std::string> getExtensions(
            int types, const std::shared_ptr<system::Context>& context)
        {
            std::vector<std::string> out;
            //! \todo Get extensions for the Python adapters?
            if (types & static_cast<int>(io::FileType::Movie))
            {
                out.push_back(".otio");
                out.push_back(".otioz");
            }
            if (auto ioSystem = context->getSystem<io::System>())
            {
                for (const auto& plugin : ioSystem->getPlugins())
                {
                    const auto& extensions = plugin->getExtensions(types);
                    out.insert(out.end(), extensions.begin(), extensions.end());
                }
            }
            return out;
        }

        std::vector<otime::TimeRange>
        toRanges(std::vector<otime::RationalTime> frames)
        {
            std::vector<otime::TimeRange> out;
            if (!frames.empty())
            {
                std::sort(frames.begin(), frames.end());
                auto i = frames.begin();
                auto j = i;
                do
                {
                    auto k = j + 1;
                    if (k != frames.end() && (*k - *j).value() > 1)
                    {
                        out.push_back(
                            otime::TimeRange::
                                range_from_start_end_time_inclusive(*i, *j));
                        i = k;
                        j = k;
                    }
                    else if (k == frames.end())
                    {
                        out.push_back(
                            otime::TimeRange::
                                range_from_start_end_time_inclusive(*i, *j));
                        i = k;
                        j = k;
                    }
                    else
                    {
                        ++j;
                    }
                } while (j != frames.end());
            }
            return out;
        }

        otime::RationalTime loop(
            const otime::RationalTime& value, const otime::TimeRange& range,
            bool* looped)
        {
            auto out = value;
            if (out < range.start_time())
            {
                if (looped)
                {
                    *looped = true;
                }
                out = range.end_time_inclusive();
            }
            else if (out > range.end_time_inclusive())
            {
                if (looped)
                {
                    *looped = true;
                }
                out = range.start_time();
            }
            return out;
        }

        TLRENDER_ENUM_IMPL(CacheDirection, "Forward", "Reverse");
        TLRENDER_ENUM_SERIALIZE_IMPL(CacheDirection);

        std::vector<otime::TimeRange> loopCache(
            const otime::TimeRange& value, const otime::TimeRange& range,
            CacheDirection direction)
        {
            std::vector<otime::TimeRange> out;
            const otime::RationalTime min =
                std::min(value.duration(), range.duration());
            switch (direction)
            {
            case CacheDirection::Forward:
                if (value.start_time() < range.start_time())
                {
                    const otime::TimeRange a(range.start_time(), min);
                    TLRENDER_ASSERT(a.duration() == min);
                    out.push_back(a);
                }
                else if (value.start_time() > range.end_time_inclusive())
                {
                    const otime::TimeRange a(
                        range.end_time_exclusive() - min, min);
                    TLRENDER_ASSERT(a.duration() == min);
                    out.push_back(a);
                }
                else if (
                    value.end_time_inclusive() > range.end_time_exclusive())
                {
                    const otime::TimeRange clamped(value.start_time(), min);
                    const otime::TimeRange a =
                        otime::TimeRange::range_from_start_end_time_inclusive(
                            clamped.start_time(), range.end_time_inclusive());
                    const otime::TimeRange b = otime::TimeRange(
                        range.start_time(), clamped.duration() - a.duration());
                    TLRENDER_ASSERT(a.duration() + b.duration() == min);
                    if (a.duration().value() > 0.0)
                    {
                        out.push_back(a);
                    }
                    if (b.duration().value() > 0.0)
                    {
                        out.push_back(b);
                    }
                }
                else
                {
                    out.push_back(value);
                }
                break;
            case CacheDirection::Reverse:
                if (value.end_time_inclusive() > range.end_time_inclusive())
                {
                    const otime::TimeRange a(
                        range.end_time_exclusive() - min, min);
                    out.push_back(a);
                    TLRENDER_ASSERT(a.duration() == min);
                }
                else if (value.end_time_inclusive() < range.start_time())
                {
                    const otime::TimeRange a(range.start_time(), min);
                    out.push_back(a);
                    TLRENDER_ASSERT(a.duration() == min);
                }
                else if (value.start_time() <= range.start_time())
                {
                    const otime::TimeRange clamped =
                        otime::TimeRange::range_from_start_end_time_inclusive(
                            value.end_time_exclusive() - min,
                            value.end_time_inclusive());
                    const otime::TimeRange a =
                        otime::TimeRange::range_from_start_end_time_inclusive(
                            range.start_time(), clamped.end_time_inclusive());
                    const otime::RationalTime behind_duration =
                        clamped.duration() - a.duration();
                    const otime::TimeRange b =
                        otime::TimeRange::range_from_start_end_time_inclusive(
                            range.end_time_exclusive() - behind_duration,
                            range.end_time_inclusive());
                    TLRENDER_ASSERT(a.duration() + b.duration() == min);
                    if (a.duration().value() > 0.0)
                    {
                        out.push_back(a);
                    }
                    if (b.duration().value() > 0.0)
                    {
                        out.push_back(b);
                    }
                }
                else
                {
                    out.push_back(value);
                }
                break;
            default:
                break;
            }
            return out;
        }

        const otio::Composable* getRoot(const otio::Composable* composable)
        {
            const otio::Composable* out = composable;
            for (; out->parent(); out = out->parent())
                ;
            return out;
        }

        std::optional<otime::RationalTime>
        getDuration(const otio::Timeline* otioTimeline, const std::string& kind)
        {
            std::optional<otime::RationalTime> out;
            otio::ErrorStatus errorStatus;
            for (auto track :
                 otioTimeline->find_children<otio::Track>(&errorStatus))
            {
                if (kind == track->kind())
                {
                    const otime::RationalTime duration =
                        track->duration(&errorStatus);
                    if (out.has_value())
                    {
                        out = std::max(out.value(), duration);
                    }
                    else
                    {
                        out = duration;
                    }
                }
            }
            return out;
        }

        otime::TimeRange getTimeRange(const otio::Timeline* otioTimeline)
        {
            otime::TimeRange out = time::invalidTimeRange;
            auto duration =
                timeline::getDuration(otioTimeline, otio::Track::Kind::video);
            if (!duration.has_value())
            {
                duration = timeline::getDuration(
                    otioTimeline, otio::Track::Kind::audio);
            }
            if (duration.has_value())
            {
                const otime::RationalTime startTime =
                    otioTimeline->global_start_time().has_value()
                        ? otioTimeline->global_start_time().value().rescaled_to(
                              duration->rate())
                        : otime::RationalTime(0, duration->rate());
                out = otime::TimeRange(startTime, duration.value());
            }
            return out;
        }

        std::vector<file::Path> getPaths(
            const file::Path& path, const file::PathOptions& pathOptions,
            const std::shared_ptr<system::Context>& context)
        {
            std::vector<file::Path> out;
            const auto fileInfo = file::FileInfo(path);
            switch (fileInfo.getType())
            {
            case file::Type::Directory:
            {
                auto ioSystem = context->getSystem<io::System>();
                file::ListOptions listOptions;
                listOptions.maxNumberDigits = pathOptions.maxNumberDigits;
                std::vector<file::FileInfo> list;
                file::list(
                    path.get(-1, file::PathType::Path), list, listOptions);
                for (const auto& fileInfo : list)
                {
                    const file::Path& path = fileInfo.getPath();
                    const std::string extension =
                        string::toLower(path.getExtension());
                    switch (ioSystem->getFileType(extension))
                    {
                    case io::FileType::Sequence:
                    case io::FileType::Movie:
                    case io::FileType::Audio:
                        out.push_back(path);
                        break;
                    default:
                        //! \todo Get extensions for the Python adapters?
                        if (".otio" == extension || ".otioz" == extension)
                        {
                            out.push_back(path);
                        }
                        break;
                    }
                }
                break;
            }
            default:
                out.push_back(path);
                break;
            }
            return out;
        }

        file::Path getPath(
            const std::string& url, const std::string& directory,
            const file::PathOptions& pathOptions)
        {
            file::Path out(url, pathOptions);
            if (out.isFileProtocol() && !out.isAbsolute())
            {
                out.setDirectory(
                    file::appendSeparator(directory) + out.getDirectory());
            }
            return out;
        }

        file::Path getPath(
            const otio::MediaReference* ref, const std::string& directory,
            file::PathOptions pathOptions)
        {
            std::string url;
            math::IntRange sequence;
            if (auto externalRef =
                    dynamic_cast<const otio::ExternalReference*>(ref))
            {
                url = externalRef->target_url();
                pathOptions.maxNumberDigits = 0;
            }
            else if (
                auto imageSequenceRef =
                    dynamic_cast<const otio::ImageSequenceReference*>(ref))
            {
                std::stringstream ss;
                ss << imageSequenceRef->target_url_base()
                   << imageSequenceRef->name_prefix() << std::setfill('0')
                   << std::setw(imageSequenceRef->frame_zero_padding())
                   << imageSequenceRef->start_frame()
                   << imageSequenceRef->name_suffix();
                url = ss.str();
                sequence = math::IntRange(
                    imageSequenceRef->start_frame(),
                    imageSequenceRef->end_frame());
            }
            else if (
                auto rawMemoryRef =
                    dynamic_cast<const RawMemoryReference*>(ref))
            {
                url = rawMemoryRef->target_url();
                pathOptions.maxNumberDigits = 0;
            }
            else if (
                auto sharedMemoryRef =
                    dynamic_cast<const SharedMemoryReference*>(ref))
            {
                url = sharedMemoryRef->target_url();
                pathOptions.maxNumberDigits = 0;
            }
            else if (
                auto rawMemorySequenceRef =
                    dynamic_cast<const RawMemorySequenceReference*>(ref))
            {
                url = rawMemorySequenceRef->target_url();
            }
            else if (
                auto sharedMemorySequenceRef =
                    dynamic_cast<const SharedMemorySequenceReference*>(ref))
            {
                url = sharedMemorySequenceRef->target_url();
            }

            std::string local_url = url;
            path_mapping::replace_path(local_url);

            file::Path out =
                timeline::getPath(local_url, directory, pathOptions);
            if (sequence.getMin() != sequence.getMax())
            {
                out.setSequence(sequence);
            }
            return out;
        }

        std::vector<file::MemoryRead>
        getMemoryRead(const otio::MediaReference* ref)
        {
            std::vector<file::MemoryRead> out;
            if (auto rawMemoryReference =
                    dynamic_cast<const RawMemoryReference*>(ref))
            {
                out.push_back(file::MemoryRead(
                    rawMemoryReference->memory(),
                    rawMemoryReference->memory_size()));
            }
            else if (
                auto sharedMemoryReference =
                    dynamic_cast<const SharedMemoryReference*>(ref))
            {
                if (const auto& memory = sharedMemoryReference->memory())
                {
                    out.push_back(
                        file::MemoryRead(memory->data(), memory->size()));
                }
            }
            else if (
                auto rawMemorySequenceReference =
                    dynamic_cast<const RawMemorySequenceReference*>(ref))
            {
                const auto& memory = rawMemorySequenceReference->memory();
                const size_t memory_size = memory.size();
                const auto& memory_sizes =
                    rawMemorySequenceReference->memory_sizes();
                const size_t memory_sizes_size = memory_sizes.size();
                for (size_t i = 0; i < memory_size && i < memory_sizes_size;
                     ++i)
                {
                    out.push_back(file::MemoryRead(memory[i], memory_sizes[i]));
                }
            }
            else if (
                auto sharedMemorySequenceReference =
                    dynamic_cast<const SharedMemorySequenceReference*>(ref))
            {
                for (const auto& memory :
                     sharedMemorySequenceReference->memory())
                {
                    if (memory)
                    {
                        out.push_back(
                            file::MemoryRead(memory->data(), memory->size()));
                    }
                }
            }
            return out;
        }

        TLRENDER_ENUM_IMPL(ToMemoryReference, "Shared", "Raw");
        TLRENDER_ENUM_SERIALIZE_IMPL(ToMemoryReference);

        void toMemoryReferences(
            otio::Timeline* otioTimeline, const std::string& directory,
            ToMemoryReference toMemoryReference,
            const file::PathOptions& pathOptions)
        {
            // Recursively iterate over all clips in the timeline.
            for (auto clip : otioTimeline->find_children<otio::Clip>())
            {
                if (auto ref = dynamic_cast<otio::ExternalReference*>(
                        clip->media_reference()))
                {
                    // Get the external reference path.
                    const auto path =
                        getPath(ref->target_url(), directory, pathOptions);

                    // Read the external reference into memory.
                    auto fileIO =
                        file::FileIO::create(path.get(), file::Mode::Read);
                    const size_t size = fileIO->getSize();

                    // Replace the external reference with a memory reference.
                    switch (toMemoryReference)
                    {
                    case ToMemoryReference::Shared:
                    {
                        auto memory = std::make_shared<MemoryReferenceData>();
                        memory->resize(size);
                        fileIO->read(memory->data(), size);
                        clip->set_media_reference(new SharedMemoryReference(
                            ref->target_url(), memory, clip->available_range(),
                            ref->metadata()));
                        break;
                    }
                    case ToMemoryReference::Raw:
                    {
                        uint8_t* memory = new uint8_t[size];
                        fileIO->read(memory, size);
                        clip->set_media_reference(new RawMemoryReference(
                            ref->target_url(), memory, size,
                            clip->available_range(), ref->metadata()));
                        break;
                    }
                    default:
                        break;
                    }
                }
                else if (
                    auto ref = dynamic_cast<otio::ImageSequenceReference*>(
                        clip->media_reference()))
                {
                    // Get the image sequence reference path.
                    const int padding = ref->frame_zero_padding();
                    std::stringstream ss;
                    ss << ref->target_url_base() << ref->name_prefix()
                       << std::setfill('0') << std::setw(padding)
                       << ref->start_frame() << ref->name_suffix();
                    const auto path = getPath(ss.str(), directory, pathOptions);

                    // Read the image sequence reference into memory.
                    std::vector<std::shared_ptr<MemoryReferenceData> >
                        sharedMemoryList;
                    std::vector<const uint8_t*> rawMemoryList;
                    std::vector<size_t> rawMemorySizeList;
                    const auto range = clip->trimmed_range();
                    for (int64_t frame = ref->start_frame();
                         frame < ref->start_frame() + range.duration().value();
                         ++frame)
                    {
                        const auto fileName = path.get(frame);
                        auto fileIO =
                            file::FileIO::create(fileName, file::Mode::Read);
                        const size_t size = fileIO->getSize();
                        switch (toMemoryReference)
                        {
                        case ToMemoryReference::Shared:
                        {
                            auto memory =
                                std::make_shared<MemoryReferenceData>();
                            memory->resize(size);
                            fileIO->read(memory->data(), size);
                            sharedMemoryList.push_back(memory);
                            break;
                        }
                        case ToMemoryReference::Raw:
                        {
                            auto memory = new uint8_t[size];
                            fileIO->read(memory, size);
                            rawMemoryList.push_back(memory);
                            rawMemorySizeList.push_back(size);
                            break;
                        }
                        default:
                            break;
                        }
                    }

                    // Replace the image sequence reference with a memory
                    // sequence reference.
                    switch (toMemoryReference)
                    {
                    case ToMemoryReference::Shared:
                        clip->set_media_reference(
                            new SharedMemorySequenceReference(
                                path.get(), sharedMemoryList,
                                clip->available_range(), ref->metadata()));
                        break;
                    case ToMemoryReference::Raw:
                        clip->set_media_reference(
                            new RawMemorySequenceReference(
                                path.get(), rawMemoryList, rawMemorySizeList,
                                clip->available_range(), ref->metadata()));
                        break;
                    default:
                        break;
                    }
                }
            }
        }

        otime::RationalTime toVideoMediaTime(
            const otime::RationalTime& time,
            const otime::TimeRange& trimmedRangeInParent,
            const otime::TimeRange& trimmedRange, double rate)
        {
            otime::RationalTime out = time - trimmedRangeInParent.start_time() +
                                      trimmedRange.start_time();
            out = out.rescaled_to(rate).round();
            return out;
        }

        otime::TimeRange toAudioMediaTime(
            const otime::TimeRange& timeRange,
            const otime::TimeRange& trimmedRangeInParent,
            const otime::TimeRange& trimmedRange, double sampleRate)
        {
            otime::TimeRange out = otime::TimeRange(
                timeRange.start_time() - trimmedRangeInParent.start_time() +
                    trimmedRange.start_time(),
                timeRange.duration());
            out = otime::TimeRange(
                out.start_time().rescaled_to(sampleRate).round(),
                out.duration().rescaled_to(sampleRate).round());
            return out;
        }

        namespace
        {
            class OTIOZWriter
            {
            public:
                OTIOZWriter(
                    const std::string& fileName,
                    const otio::SerializableObject::Retainer<otio::Timeline>&,
                    const std::string& directory = std::string());

                ~OTIOZWriter();

            private:
                void _addCompressed(
                    const std::string& content,
                    const std::string& fileNameInZip);
                void _addUncompressed(
                    const std::string& fileName,
                    const std::string& fileNameInZip);

                static std::string _getMediaFileName(
                    const std::string& url, const std::string& directory);
                static std::string _getFileNameInZip(const std::string& url);
                static std::string
                _normzalizePathSeparators(const std::string&);
                static bool _isFileNameAbsolute(const std::string&);

                void* _writer = nullptr;
            };

            OTIOZWriter::OTIOZWriter(
                const std::string& fileName,
                const otio::SerializableObject::Retainer<otio::Timeline>&
                    timeline,
                const std::string& directory)
            {
                // Copy the timeline.
                otio::SerializableObject::Retainer<otio::Timeline> timelineCopy(
                    dynamic_cast<otio::Timeline*>(
                        otio::Timeline::from_json_string(
                            timeline->to_json_string())));

                // Find the media references.
                std::map<std::string, std::string> mediaFilesNames;
                std::string directoryTmp = _normzalizePathSeparators(directory);
                if (!directoryTmp.empty() && directoryTmp.back() != '/')
                {
                    directoryTmp += '/';
                }
                for (const auto& clip : timelineCopy->find_clips())
                {
                    if (auto ref = dynamic_cast<otio::ExternalReference*>(
                            clip->media_reference()))
                    {
                        const std::string& url = ref->target_url();
                        const std::string mediaFileName =
                            _getMediaFileName(url, directoryTmp);
                        const std::string fileNameInZip =
                            _getFileNameInZip(url);
                        mediaFilesNames[mediaFileName] = fileNameInZip;
                        ref->set_target_url(fileNameInZip);
                    }
                    else if (
                        auto ref = dynamic_cast<otio::ImageSequenceReference*>(
                            clip->media_reference()))
                    {
                        const int padding = ref->frame_zero_padding();
                        std::stringstream ss;
                        ss << ref->target_url_base() << ref->name_prefix()
                           << std::setfill('0') << std::setw(padding)
                           << ref->start_frame() << ref->name_suffix();
                        const file::Path path(
                            _getMediaFileName(ss.str(), directoryTmp));
                        const auto range = clip->trimmed_range();
                        for (int64_t frame = ref->start_frame();
                             frame <
                             ref->start_frame() + range.duration().value();
                             ++frame)
                        {
                            const std::string mediaFileName = path.get(frame);
                            const std::string fileNameInZip =
                                _getFileNameInZip(mediaFileName);
                            mediaFilesNames[mediaFileName] = fileNameInZip;
                        }
                        ref->set_target_url_base(
                            _getFileNameInZip(ref->target_url_base()));
                    }
                }

                // Open the output file.
                mz_zip_writer_create(&_writer);
                if (!_writer)
                {
                    throw std::runtime_error("Cannot create writer");
                }
                int32_t err =
                    mz_zip_writer_open_file(_writer, fileName.c_str(), 0, 0);
                if (err != MZ_OK)
                {
                    throw std::runtime_error("Cannot open output file");
                }

                // Add the content and version files.
                _addCompressed("1.0.0", "version.txt");
                _addCompressed(timelineCopy->to_json_string(), "content.otio");

                // Add the media files.
                for (const auto& i : mediaFilesNames)
                {
                    _addUncompressed(i.first, i.second);
                }

                // Close the file.
                err = mz_zip_writer_close(_writer);
                if (err != MZ_OK)
                {
                    throw std::runtime_error("Cannot close output file");
                }
            }

            OTIOZWriter::~OTIOZWriter()
            {
                if (_writer)
                {
                    mz_zip_writer_delete(&_writer);
                }
            }

            void OTIOZWriter::_addCompressed(
                const std::string& content, const std::string& fileNameInZip)
            {
                mz_zip_file fileInfo;
                memset(&fileInfo, 0, sizeof(mz_zip_file));
                mz_zip_writer_set_compress_level(
                    _writer, MZ_COMPRESS_LEVEL_NORMAL);
                fileInfo.version_madeby = MZ_VERSION_MADEBY;
                fileInfo.flag = MZ_ZIP_FLAG_UTF8;
                fileInfo.modified_date = std::time(nullptr);
                fileInfo.compression_method = MZ_COMPRESS_METHOD_DEFLATE;
                fileInfo.filename = fileNameInZip.c_str();
                int32_t err = mz_zip_writer_add_buffer(
                    _writer, (void*)content.c_str(), content.size(), &fileInfo);
                if (err != MZ_OK)
                {
                    throw std::runtime_error("Cannot add file");
                }
            }

            void OTIOZWriter::_addUncompressed(
                const std::string& fileName, const std::string& fileNameInZip)
            {
                /*auto fileIO = file::FileIO::create(fileName,
                file::Mode::Read); std::vector<uint8_t> buf(fileIO->getSize());
                fileIO->read(buf.data(), buf.size());
                mz_zip_file fileInfo;
                memset(&fileInfo, 0, sizeof(mz_zip_file));
                fileInfo.version_madeby = MZ_VERSION_MADEBY;
                fileInfo.modified_date = std::time(nullptr);
                fileInfo.compression_method = MZ_COMPRESS_METHOD_STORE;
                fileInfo.filename = fileNameInZip.c_str();
                int32_t err = mz_zip_writer_add_buffer(
                    _writer,
                    (void*)buf.data(),
                    buf.size(),
                    &fileInfo);
                if (err != MZ_OK)
                {
                    throw std::runtime_error("Cannot add file");
                }*/
                mz_zip_writer_set_compress_method(
                    _writer, MZ_COMPRESS_METHOD_STORE);
                int32_t err = mz_zip_writer_add_file(
                    _writer, fileName.c_str(), fileNameInZip.c_str());
                if (err != MZ_OK)
                {
                    throw std::runtime_error("Cannot add file: " + fileName);
                }
            }

            std::string OTIOZWriter::_getFileNameInZip(const std::string& url)
            {
                std::string::size_type r = url.rfind('/');
                if (std::string::npos == r)
                {
                    r = url.rfind('\\');
                }
                const std::string fileName =
                    std::string::npos == r ? url : url.substr(r + 1);
                return "media/" + fileName;
            }

            std::string OTIOZWriter::_getMediaFileName(
                const std::string& url, const std::string& directory)
            {
                std::string fileName = url;
                if ("file://" == fileName.substr(7))
                {
                    fileName.erase(0, 7);
                }
                if (!_isFileNameAbsolute(fileName))
                {
                    fileName = directory + fileName;
                }
                return fileName;
            }

            std::string
            OTIOZWriter::_normzalizePathSeparators(const std::string& fileName)
            {
                std::string out = fileName;
                for (size_t i = 0; i < out.size(); ++i)
                {
                    if ('\\' == out[i])
                    {
                        out[i] = '/';
                    }
                }
                return out;
            }

            bool OTIOZWriter::_isFileNameAbsolute(const std::string& fileName)
            {
                bool out = false;
                if (!fileName.empty() && '/' == fileName[0])
                {
                    out = true;
                }
                else if (!fileName.empty() && '\\' == fileName[0])
                {
                    out = true;
                }
                else if (
                    fileName.size() >= 2 &&
                    (fileName[0] >= 'A' && fileName[0] <= 'Z' ||
                     fileName[0] >= 'a' && fileName[0] <= 'z') &&
                    ':' == fileName[1])
                {
                    out = true;
                }
                return out;
            }
        } // namespace

        bool writeOTIOZ(
            const std::string& fileName,
            const otio::SerializableObject::Retainer<otio::Timeline>& timeline,
            const std::string& directory)
        {
            bool out = false;
            try
            {
                OTIOZWriter(fileName, timeline, directory);
                out = true;
            }
            catch (const std::exception&)
            {
            }
            return out;
        }
    } // namespace timeline
} // namespace tl
