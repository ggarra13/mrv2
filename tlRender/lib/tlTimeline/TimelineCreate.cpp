// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimeline/TimelinePrivate.h>

#include <tlTimeline/MemoryReference.h>
#include <tlTimeline/Util.h>

#include <tlIO/System.h>

#include <tlCore/File.h>
#include <tlCore/FileInfo.h>
#include <tlCore/StringFormat.h>

#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>
#include <opentimelineio/serializableCollection.h>

#include <minizip/mz.h>
#include <minizip/mz_strm.h>
#include <minizip/mz_zip.h>
#include <minizip/mz_zip_rw.h>

#if defined(TLRENDER_PYTHON)
#    include <Python.h>
#endif // TLRENDER_PYTHON

namespace tl
{
    namespace timeline
    {
        namespace
        {
            file::Path getAudioPath(
                const file::Path& path,
                const FileSequenceAudio& fileSequenceAudio,
                const std::string& fileSequenceAudioFileName,
                const std::string& fileSequenceAudioDirectory,
                const file::PathOptions& pathOptions,
                const std::shared_ptr<system::Context>& context)
            {
                file::Path out;
                auto ioSystem = context->getSystem<io::System>();
                const auto audioExtensions = ioSystem->getExtensions(
                    static_cast<int>(io::FileType::Audio));
                switch (fileSequenceAudio)
                {
                case FileSequenceAudio::BaseName:
                {
                    std::vector<std::string> names;
                    names.push_back(path.getDirectory() + path.getBaseName());
                    std::string tmp = path.getBaseName();
                    if (!tmp.empty() && '.' == tmp[tmp.size() - 1])
                    {
                        tmp.pop_back();
                    }
                    names.push_back(path.getDirectory() + tmp);
                    for (const auto& name : names)
                    {
                        for (const auto& extension : audioExtensions)
                        {
                            const file::Path audioPath(
                                name + extension, pathOptions);
                            if (file::exists(audioPath.get()))
                            {
                                out = audioPath;
                                break;
                            }
                        }
                    }
                    break;
                }
                case FileSequenceAudio::FileName:
                    out = file::Path(
                        path.getDirectory() + fileSequenceAudioFileName,
                        pathOptions);
                    break;
                case FileSequenceAudio::Directory:
                {
                    const file::Path directoryPath(
                        path.getDirectory(), fileSequenceAudioDirectory,
                        pathOptions);
                    file::ListOptions listOptions;
                    listOptions.maxNumberDigits = pathOptions.maxNumberDigits;
                    std::vector<file::FileInfo> list;
                    file::list(directoryPath.get(), list, listOptions);
                    for (const auto& fileInfo : list)
                    {
                        if (file::Type::File == fileInfo.getType())
                        {
                            for (const auto& extension : audioExtensions)
                            {
                                if (extension ==
                                    fileInfo.getPath().getExtension())
                                {
                                    out = fileInfo.getPath();
                                    break;
                                }
                            }
                        }
                    }
                    break;
                }
                default:
                    break;
                }
                return out;
            }

#if defined(TLRENDER_PYTHON)
            class PyObjectRef
            {
            public:
                PyObjectRef(PyObject* o) :
                    o(o)
                {
                    if (!o)
                    {
                        throw std::runtime_error("Python error");
                    }
                }

                ~PyObjectRef() { Py_XDECREF(o); }

                PyObject* o = nullptr;

                operator PyObject*() const { return o; }
            };
#endif // TLRENDER_PYTHON
        } // namespace

        class ZipReader
        {
        public:
            ZipReader(const std::string& fileName)
            {
                mz_zip_reader_create(&reader);
                if (!reader)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot create zip reader")
                            .arg(fileName));
                }
                int32_t err = mz_zip_reader_open_file(reader, fileName.c_str());
                if (err != MZ_OK)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot open zip reader")
                            .arg(fileName));
                }
            }

            ~ZipReader() { mz_zip_reader_delete(&reader); }

            void* reader = nullptr;
        };

        class ZipReaderFile
        {
        public:
            ZipReaderFile(void* reader, const std::string& fileName) :
                reader(reader)
            {
                int32_t err = mz_zip_reader_entry_open(reader);
                if (err != MZ_OK)
                {
                    throw std::runtime_error(
                        string::Format("{0}: Cannot open zip entry")
                            .arg(fileName));
                }
            }

            ~ZipReaderFile() { mz_zip_reader_entry_close(reader); }

            void* reader = nullptr;
        };

        otio::SerializableObject::Retainer<otio::Timeline>
        readOTIO(const file::Path& path, otio::ErrorStatus* errorStatus)
        {
            otio::SerializableObject::Retainer<otio::Timeline> out;
            const std::string fileName = path.get();
            const std::string extension = string::toLower(path.getExtension());
            if (".otio" == extension)
            {
                auto timeline =
                    otio::Timeline::from_json_file(fileName, errorStatus);
                out = dynamic_cast<otio::Timeline*>(timeline);
                if (!out)
                {
                    auto collection =
                        dynamic_cast<otio::SerializableCollection*>(timeline);
                    if (collection)
                    {
                        auto children =
                            collection->find_children<otio::Timeline>();
                        if (children.size() > 1)
                        {
                            throw std::runtime_error(
                                string::Format(
                                    "{0}: Only one timeline is supported.")
                                    .arg(fileName));
                        }
                        else if (children.size() == 1)
                        {
                            out = otio::dynamic_retainer_cast<otio::Timeline>(
                                children[0]);
                        }
                    }
                }
            }
            else if (".otioz" == extension)
            {
                {
                    ZipReader zipReader(fileName);

                    const std::string contentFileName = "content.otio";
                    int32_t err = mz_zip_reader_locate_entry(
                        zipReader.reader, contentFileName.c_str(), 0);
                    if (err != MZ_OK)
                    {
                        throw std::runtime_error(
                            string::Format("{0}: Cannot find zip entry")
                                .arg(contentFileName));
                    }
                    mz_zip_file* fileInfo = nullptr;
                    err = mz_zip_reader_entry_get_info(
                        zipReader.reader, &fileInfo);
                    if (err != MZ_OK)
                    {
                        throw std::runtime_error(
                            string::Format(
                                "{0}: Cannot get zip entry information")
                                .arg(contentFileName));
                    }
                    ZipReaderFile zipReaderFile(
                        zipReader.reader, contentFileName);
                    std::vector<char> buf;
                    buf.resize(fileInfo->uncompressed_size + 1);
                    err = mz_zip_reader_entry_read(
                        zipReader.reader, buf.data(),
                        fileInfo->uncompressed_size);
                    if (err != fileInfo->uncompressed_size)
                    {
                        throw std::runtime_error(
                            string::Format("{0}: Cannot read zip entry")
                                .arg(contentFileName));
                    }
                    buf[fileInfo->uncompressed_size] = 0;

                    out = dynamic_cast<otio::Timeline*>(
                        otio::Timeline::from_json_string(
                            buf.data(), errorStatus));

                    auto fileIO =
                        file::FileIO::create(fileName, file::Mode::Read);
                    for (auto clip : out->find_children<otio::Clip>())
                    {
                        if (auto externalReference =
                                dynamic_cast<otio::ExternalReference*>(
                                    clip->media_reference()))
                        {
                            const std::string mediaFileName =
                                file::Path(externalReference->target_url())
                                    .get();

                            int32_t err = mz_zip_reader_locate_entry(
                                zipReader.reader, mediaFileName.c_str(), 0);
                            if (err != MZ_OK)
                            {
                                throw std::runtime_error(
                                    string::Format("{0}: Cannot find zip entry")
                                        .arg(mediaFileName));
                            }
                            err = mz_zip_reader_entry_get_info(
                                zipReader.reader, &fileInfo);
                            if (err != MZ_OK)
                            {
                                throw std::runtime_error(
                                    string::Format(
                                        "{0}: Cannot get zip entry information")
                                        .arg(mediaFileName));
                            }

                            const size_t headerSize = 30 +
                                                      fileInfo->filename_size +
                                                      fileInfo->extrafield_size;
                            auto memoryReference = new ZipMemoryReference(
                                fileIO, externalReference->target_url(),
                                fileIO->getMemoryStart() +
                                    fileInfo->disk_offset + headerSize,
                                fileInfo->uncompressed_size,
                                externalReference->available_range(),
                                externalReference->metadata());
                            clip->set_media_reference(memoryReference);
                        }
                        else if (
                            auto imageSequenceReference =
                                dynamic_cast<otio::ImageSequenceReference*>(
                                    clip->media_reference()))
                        {
                            std::vector<const uint8_t*> memory;
                            std::vector<size_t> memory_sizes;
                            for (int number = 0;
                                 number < imageSequenceReference
                                              ->number_of_images_in_sequence();
                                 ++number)
                            {
                                const std::string mediaFileName =
                                    file::Path(
                                        imageSequenceReference
                                            ->target_url_for_image_number(
                                                number))
                                        .get();

                                int32_t err = mz_zip_reader_locate_entry(
                                    zipReader.reader, mediaFileName.c_str(), 0);
                                if (err != MZ_OK)
                                {
                                    throw std::runtime_error(
                                        string::Format(
                                            "{0}: Cannot find zip entry")
                                            .arg(mediaFileName));
                                }
                                err = mz_zip_reader_entry_get_info(
                                    zipReader.reader, &fileInfo);
                                if (err != MZ_OK)
                                {
                                    throw std::runtime_error(
                                        string::Format("{0}: Cannot get zip "
                                                       "entry information")
                                            .arg(mediaFileName));
                                }

                                const size_t headerSize =
                                    30 + fileInfo->filename_size +
                                    fileInfo->extrafield_size;
                                memory.push_back(
                                    fileIO->getMemoryStart() +
                                    fileInfo->disk_offset + headerSize);
                                memory_sizes.push_back(
                                    fileInfo->uncompressed_size);
                            }
                            auto memoryReference =
                                new ZipMemorySequenceReference(
                                    fileIO,
                                    imageSequenceReference
                                        ->target_url_for_image_number(0),
                                    memory, memory_sizes,
                                    imageSequenceReference->available_range(),
                                    imageSequenceReference->metadata());
                            clip->set_media_reference(memoryReference);
                        }
                    }
                }
            }
            else
            {
#if defined(TLRENDER_PYTHON)
                Py_Initialize();
                try
                {
                    auto pyModule = PyObjectRef(
                        PyImport_ImportModule("opentimelineio.adapters"));

                    auto pyReadFromFile = PyObjectRef(
                        PyObject_GetAttrString(pyModule, "read_from_file"));
                    auto pyReadFromFileArgs = PyObjectRef(PyTuple_New(1));
                    auto pyReadFromFileArg = PyUnicode_FromStringAndSize(
                        fileName.c_str(), fileName.size());
                    if (!pyReadFromFileArg)
                    {
                        throw std::runtime_error("Cannot create arg");
                    }
                    PyTuple_SetItem(pyReadFromFileArgs, 0, pyReadFromFileArg);
                    auto pyTimeline = PyObjectRef(PyObject_CallObject(
                        pyReadFromFile, pyReadFromFileArgs));

                    auto pyToJSONString = PyObjectRef(
                        PyObject_GetAttrString(pyTimeline, "to_json_string"));
                    auto pyJSONString =
                        PyObjectRef(PyObject_CallObject(pyToJSONString, NULL));
                    out = otio::SerializableObject::Retainer<otio::Timeline>(
                        dynamic_cast<otio::Timeline*>(
                            otio::Timeline::from_json_string(
                                PyUnicode_AsUTF8AndSize(pyJSONString, NULL),
                                errorStatus)));
                }
                catch (const std::exception& e)
                {
                    errorStatus->outcome =
                        otio::ErrorStatus::Outcome::FILE_OPEN_FAILED;
                    errorStatus->details = e.what();
                }
                if (PyErr_Occurred())
                {
                    PyErr_Print();
                }
                Py_Finalize();
#endif // TLRENDER_PYTHON
            }
            return out;
        }

        otio::SerializableObject::Retainer<otio::Timeline> create(
            const file::Path& path,
            const std::shared_ptr<system::Context>& context,
            const otime::RationalTime& offsetTime, const Options& options)
        {
            return create(path, file::Path(), context, offsetTime, options);
        }

        otio::SerializableObject::Retainer<otio::Timeline> create(
            const file::Path& inputPath, const file::Path& inputAudioPath,
            const std::shared_ptr<system::Context>& context,
            const otime::RationalTime& offsetTime, const Options& options)
        {
            otio::SerializableObject::Retainer<otio::Timeline> out;
            std::string error;
            file::Path path = inputPath;
            file::Path audioPath = inputAudioPath;

            try
            {
                auto ioSystem = context->getSystem<io::System>();

                // Is the input a sequence?
                const bool isSequence =
                    io::FileType::Sequence ==
                        ioSystem->getFileType(path.getExtension()) &&
                    !path.getNumber().empty();
                if (isSequence)
                {
                    if (!path.isSequence())
                    {
                        // Check for other files in the sequence.
                        std::vector<file::FileInfo> list;
                        file::ListOptions listOptions;
                        listOptions.sequenceExtensions = {path.getExtension()};
                        listOptions.maxNumberDigits =
                            options.pathOptions.maxNumberDigits;
                        file::list(path.getDirectory(), list, listOptions);
                        const auto i = std::find_if(
                            list.begin(), list.end(),
                            [path](const file::FileInfo& value)
                            { return value.getPath().sequence(path); });
                        if (i != list.end())
                        {
                            path = i->getPath();
                        }
                    }
                    if (audioPath.isEmpty())
                    {
                        // Check for an associated audio file.
                        audioPath = getAudioPath(
                            path, options.fileSequenceAudio,
                            options.fileSequenceAudioFileName,
                            options.fileSequenceAudioDirectory,
                            options.pathOptions, context);
                    }
                }

                // Is the input a video or audio file?
                if (auto read = ioSystem->read(path, options.ioOptions))
                {
                    const auto info = read->getInfo().get();

                    otime::RationalTime startTime = time::invalidTime;
                    otio::Track* videoTrack = nullptr;
                    otio::Track* audioTrack = nullptr;
                    otio::ErrorStatus errorStatus;

                    // Read the video.
                    if (!info.video.empty())
                    {
                        startTime = info.videoTime.start_time();
                        auto videoClip = new otio::Clip;
                        videoClip->set_source_range(info.videoTime);
                        if (isSequence)
                        {
                            auto mediaReference =
                                new otio::ImageSequenceReference(
                                    path.isFileProtocol()
                                        ? std::string()
                                        : (path.getProtocol() +
                                           path.getDirectory()),
                                    path.getBaseName(), path.getExtension(),
                                    info.videoTime.start_time().value(), 1,
                                    info.videoTime.duration().rate(),
                                    path.getPadding());
                            mediaReference->set_available_range(info.videoTime);
                            videoClip->set_media_reference(mediaReference);
                        }
                        else
                        {
                            if (time::isValid(offsetTime))
                            {
                                startTime += otime::RationalTime(
                                    offsetTime.value(), startTime.rate());
                            }
                            videoClip->set_media_reference(
                                new otio::ExternalReference(
                                    path.get(
                                        -1, path.isFileProtocol()
                                                ? file::PathType::FileName
                                                : file::PathType::Full),
                                    info.videoTime));
                        }
                        videoTrack = new otio::Track(
                            "Video", std::nullopt, otio::Track::Kind::video);
                        videoTrack->append_child(videoClip, &errorStatus);
                        if (otio::is_error(errorStatus))
                        {
                            throw std::runtime_error("Cannot append child");
                        }
                    }

                    // Read the separate audio if provided.
                    if (!audioPath.isEmpty())
                    {
                        if (auto audioRead =
                                ioSystem->read(audioPath, options.ioOptions))
                        {
                            bool protocol = audioPath.isFileProtocol();
                            if (file::exists(
                                    audioPath.get(-1, file::PathType::Full)))
                                protocol = false;
                            const std::string cwd = file::getCWD();
                            if (file::exists(cwd + audioPath.get()))
                            {
                                audioPath = file::Path(cwd + audioPath.get());
                                protocol = false;
                            }

                            const auto audioInfo = audioRead->getInfo().get();

                            auto audioClip = new otio::Clip;
                            audioClip->set_source_range(audioInfo.audioTime);
                            audioClip->set_media_reference(
                                new otio::ExternalReference(
                                    audioPath.get(
                                        -1, protocol ? file::PathType::FileName
                                                     : file::PathType::Full),
                                    audioInfo.audioTime));

                            audioTrack = new otio::Track(
                                "Audio", std::nullopt,
                                otio::Track::Kind::audio);
                            audioTrack->append_child(audioClip, &errorStatus);
                            if (otio::is_error(errorStatus))
                            {
                                throw std::runtime_error("Cannot append child");
                            }
                        }
                    }
                    else if (info.audio.isValid())
                    {
                        if (startTime.is_invalid_time())
                        {
                            startTime = info.audioTime.start_time();
                        }

                        auto audioClip = new otio::Clip;
                        audioClip->set_source_range(info.audioTime);
                        audioClip->set_media_reference(
                            new otio::ExternalReference(
                                path.get(
                                    -1, path.isFileProtocol()
                                            ? file::PathType::FileName
                                            : file::PathType::Full),
                                info.audioTime));

                        audioTrack = new otio::Track(
                            "Audio", std::nullopt, otio::Track::Kind::audio);
                        audioTrack->append_child(audioClip, &errorStatus);
                        if (otio::is_error(errorStatus))
                        {
                            throw std::runtime_error("Cannot append child");
                        }
                    }

                    // Create the stack.
                    auto otioStack = new otio::Stack;
                    if (videoTrack)
                    {
                        otioStack->append_child(videoTrack, &errorStatus);
                        if (otio::is_error(errorStatus))
                        {
                            throw std::runtime_error("Cannot append child");
                        }
                    }
                    if (audioTrack)
                    {
                        otioStack->append_child(audioTrack, &errorStatus);
                        if (otio::is_error(errorStatus))
                        {
                            throw std::runtime_error("Cannot append child");
                        }
                    }

                    // Create the timeline.
                    out = new otio::Timeline(path.get());
                    out->set_tracks(otioStack);
                    if (time::isValid(startTime))
                    {
                        out->set_global_start_time(startTime);
                    }
                }
            }
            catch (const std::exception& e)
            {
                error = e.what();
            }

            auto logSystem = context->getLogSystem();
            logSystem->print(
                "tl::timeline::create",
                string::Format("\n"
                               "    Create from path: {0}\n"
                               "    Audio path: {1}")
                    .arg(path.get())
                    .arg(audioPath.get()));

            // Is the input an OTIO file?
            if (!out)
            {
                otio::ErrorStatus errorStatus;
                out = readOTIO(path, &errorStatus);
                if (otio::is_error(errorStatus))
                {
                    out = nullptr;
                    error = errorStatus.full_description;
                }
                else if (!out)
                {
                    error = string::Format("{0}: Cannot read timeline")
                                .arg(path.get());
                }
            }
            if (!out)
            {
                throw std::runtime_error(error);
            }

            otio::AnyDictionary dict;
            dict["path"] = path.get();
            dict["audioPath"] = audioPath.get();
            out->metadata()["tlRender"] = dict;

            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const otio::SerializableObject::Retainer<otio::Timeline>& timeline,
            const std::shared_ptr<system::Context>& context,
            const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            out->_init(timeline, context, options);
            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const std::string& fileName,
            const std::shared_ptr<system::Context>& context,
            const otime::RationalTime& offsetTime, const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            auto otioTimeline = timeline::create(
                file::Path(fileName, options.pathOptions), context, offsetTime,
                options);
            out->_init(otioTimeline, context, options);
            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const file::Path& path,
            const std::shared_ptr<system::Context>& context,
            const otime::RationalTime& offsetTime, const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            auto otioTimeline =
                timeline::create(path, context, offsetTime, options);
            out->_init(otioTimeline, context, options);
            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const std::string& fileName, const std::string& audioFileName,
            const std::shared_ptr<system::Context>& context,
            const otime::RationalTime& offsetTime, const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            auto otioTimeline = timeline::create(
                file::Path(fileName, options.pathOptions),
                file::Path(audioFileName, options.pathOptions), context,
                offsetTime, options);
            out->_init(otioTimeline, context, options);
            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            const file::Path& path, const file::Path& audioPath,
            const std::shared_ptr<system::Context>& context,
            const otime::RationalTime& offsetTime, const Options& options)
        {
            auto out = std::shared_ptr<Timeline>(new Timeline);
            auto otioTimeline =
                timeline::create(path, audioPath, context, offsetTime, options);
            out->_init(otioTimeline, context, options);
            return out;
        }
    } // namespace timeline
} // namespace tl
