// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <filesystem>
namespace fs = std::filesystem;

#include <tlTimeline/TimelinePrivate.h>

#include <tlIO/IOSystem.h>

#include <tlCore/File.h>
#include <tlCore/FileInfo.h>
#include <tlCore/StringFormat.h>

#include <opentimelineio/externalReference.h>
#include <opentimelineio/imageSequenceReference.h>

#include "mrvFl/mrvTimelineCreate.h"

namespace tl
{
    namespace timeline
    {
        namespace
        {

            std::string _getRelativePath(const file::Path& path,
                                         fs::path otioFilename)
            {
                std::string file = path.get();
                fs::path filePath = file;
                otioFilename = otioFilename.parent_path(); // Remove .otio file.
                fs::path relative = fs::relative( filePath, otioFilename );
                file = relative.generic_string();
                return file;
            }

            std::string _getAbsolutePath(const file::Path& path)
            {
                std::string file = path.get();
                if (!path.isAbsolute())
                {
                    file = fs::current_path().generic_string() + '/' + file;
                }
                return file;
            }

            file::Path _getAudioPath(
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
                    for (const auto& fileInfo :
                         file::dirList(directoryPath.get(), pathOptions))
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

        } // namespace

        static otio::SerializableObject::Retainer<otio::Timeline>
        read(const std::string& fileName, otio::ErrorStatus* errorStatus)
        {
            otio::SerializableObject::Retainer<otio::Timeline> out;
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
                auto pyTimeline = PyObjectRef(
                    PyObject_CallObject(pyReadFromFile, pyReadFromFileArgs));

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
#else  // TLRENDER_PYTHON
            out = dynamic_cast<otio::Timeline*>(
                otio::Timeline::from_json_file(fileName, errorStatus));
#endif // TLRENDER_PYTHON
            return out;
        }

        //! Creaate a timeline from a list of filenames.
        otio::SerializableObject::Retainer<otio::Timeline> create(
            const std::vector<std::shared_ptr<mrv::FilesModelItem> >& fileItems,
            const std::shared_ptr<system::Context>& context,
            const bool relative, const std::string& otioFilename,
            const Options& options, const std::shared_ptr<ReadCache>& readCache)
        {
            otio::SerializableObject::Retainer<otio::Timeline> out;
            otio::SerializableObject::Retainer<otio::Timeline> timeline;
            bool isSequence = false;
            auto otioStack = new otio::Stack;
            otime::RationalTime startTime = time::invalidTime;
            otio::Track* videoTrack = nullptr;
            otio::Track* audioTrack = nullptr;
            file::Path path;
            std::string error;
            file::Path audioPath;
            fs::path   otioPath( otioFilename );

            for (const auto& fileItem : fileItems)
            {
                path = fileItem->path;
                try
                {
                    auto ioSystem = context->getSystem<io::System>();
                    if (auto read = ioSystem->read(path, options.ioOptions))
                    {
                        const auto info = read->getInfo().get();
                        if (readCache)
                        {
                            ReadCacheItem item;
                            item.read = read;
                            item.ioInfo = info;
                            readCache->add(item);
                        }

                        otio::ErrorStatus errorStatus;
                        if (!info.video.empty())
                        {
                            if (!time::isValid(startTime))
                                startTime = info.videoTime.start_time();
                            auto videoClip = new otio::Clip;
                            // videoClip->set_source_range(info.videoTime);
                            videoClip->set_source_range(fileItem->inOutRange);
                            isSequence = io::FileType::Sequence ==
                                             ioSystem->getFileType(
                                                 path.getExtension()) &&
                                         !path.getNumber().empty();
                            if (isSequence)
                            {
                                videoClip->set_media_reference(
                                    new otio::ImageSequenceReference(
                                        std::string(), path.getBaseName(),
                                        path.getExtension(),
                                        fileItem->inOutRange.start_time()
                                            .value(),
                                        // info.videoTime.start_time().value(),
                                        1,
                                        fileItem->inOutRange.duration().rate(),
                                        // info.videoTime.duration().rate(),
                                        path.getPadding()));
                            }
                            else
                            {
                                std::string file;
                                if ( relative )
                                    file = _getRelativePath(path, otioPath);
                                else
                                    file = _getAbsolutePath(path);
                                videoClip->set_media_reference(
                                    new otio::ExternalReference(file));
                            }
                            if (!videoTrack)
                                videoTrack = new otio::Track(
                                    "Video", otio::nullopt,
                                    otio::Track::Kind::video);
                            videoTrack->append_child(videoClip, &errorStatus);
                            if (otio::is_error(errorStatus))
                            {
                                throw std::runtime_error("Cannot append child");
                            }

                            if (isSequence)
                            {
                                audioPath = _getAudioPath(
                                    path, options.fileSequenceAudio,
                                    options.fileSequenceAudioFileName,
                                    options.fileSequenceAudioDirectory,
                                    options.pathOptions, context);
                                if (!audioPath.isEmpty())
                                {
                                    if (auto audioRead = ioSystem->read(
                                            audioPath, options.ioOptions))
                                    {
                                        const auto audioInfo =
                                            audioRead->getInfo().get();
                                        if (readCache)
                                        {
                                            ReadCacheItem item;
                                            item.read = audioRead;
                                            item.ioInfo = audioInfo;
                                            readCache->add(item);
                                        }

                                        auto audioClip = new otio::Clip;
                                        audioClip->set_source_range(
                                            fileItem->inOutRange);
                                        // audioClip->set_source_range(audioInfo.audioTime);
                                        std::string file;
                                        if ( relative )
                                            file = _getRelativePath(audioPath,
                                                                    otioPath);
                                        else
                                            file = _getAbsolutePath(audioPath);
                                        audioClip->set_media_reference(
                                            new otio::ExternalReference(file));

                                        if (!audioTrack)
                                            audioTrack = new otio::Track(
                                                "Audio", otio::nullopt,
                                                otio::Track::Kind::audio);
                                        audioTrack->append_child(
                                            audioClip, &errorStatus);
                                        if (otio::is_error(errorStatus))
                                        {
                                            throw std::runtime_error(
                                                "Cannot append child");
                                        }
                                    }
                                }
                            }
                        }

                        if (info.audio.isValid())
                        {
                            auto audioClip = new otio::Clip;
                            audioClip->set_source_range(fileItem->inOutRange);
                            // audioClip->set_source_range(info.audioTime);
                            std::string file;
                            if ( relative )
                                file = _getRelativePath(path, otioPath);
                            else
                                file = _getAbsolutePath(path);
                            audioClip->set_media_reference(
                                new otio::ExternalReference(file));

                            if (!audioTrack)
                                audioTrack = new otio::Track(
                                    "Audio", otio::nullopt,
                                    otio::Track::Kind::audio);
                            audioTrack->append_child(audioClip, &errorStatus);
                            if (otio::is_error(errorStatus))
                            {
                                throw std::runtime_error("Cannot append child");
                            }
                        }

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

                        if (!out)
                        {
                            out = new otio::Timeline("EDL");
                            out->set_tracks(otioStack);
                            if (time::isValid(startTime))
                            {
                                out->set_global_start_time(startTime);
                            }
                        }
                    }
                    else
                    {
                        // Possible timeline (otio or python plugin)
                        otio::ErrorStatus errorStatus;
                        timeline = timeline::read(path.get(), &errorStatus);
                        if (timeline)
                        {
                            auto timelineStack = timeline->tracks();
                            if (!videoTrack)
                                videoTrack = new otio::Track(
                                    "Video", otio::nullopt,
                                    otio::Track::Kind::video);
                            videoTrack->append_child(
                                timelineStack, &errorStatus);
                            if (otio::is_error(errorStatus))
                            {
                                throw std::runtime_error(
                                    "Cannot append stack child");
                            }
                            if (!audioTrack)
                                audioTrack = new otio::Track(
                                    "Audio", otio::nullopt,
                                    otio::Track::Kind::audio);
                            audioTrack->append_child(
                                timelineStack, &errorStatus);

                            if (otio::is_error(errorStatus))
                            {
                                throw std::runtime_error(
                                    "Cannot append stack child");
                            }
                            if (!out)
                            {
                                out = new otio::Timeline("EDL");
                                out->set_tracks(otioStack);
                                if (time::isValid(startTime))
                                {
                                    out->set_global_start_time(startTime);
                                }
                            }
                        }
                        if (!out)
                        {
                            throw std::runtime_error(error);
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
            }

            return out;
        }

    } // namespace timeline
} // namespace tl
