// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2024-Present Gonzalo Garramuño
// All rights reserved.

#include <tlTimeline/TimelinePrivate.h>

#include <tlTimeline/MemoryReference.h>
#include <tlTimeline/Util.h>

#include <tlIO/System.h>

#include <tlCore/File.h>
#include <tlCore/FileInfo.h>
#include <tlCore/StringFormat.h>
#include <tlCore/URL.h>

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
                    listOptions.maxNumberDigits = pathOptions.seqMaxDigits;
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


        otio::SerializableObject::Retainer<otio::Timeline> create(
            file::Path& path,
            const std::shared_ptr<system::Context>& context,
            const otime::RationalTime& offsetTime, const Options& options)
        {
            return create(path, file::Path(), context, offsetTime, options);
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
            file::Path path(fileName, options.pathOptions);
            auto otioTimeline = timeline::create(
                path, context, offsetTime, options);
            out->_init(otioTimeline, context, options);
            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            file::Path& path,
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
            file::Path path(fileName, options.pathOptions);
            auto otioTimeline = timeline::create(path,
                file::Path(audioFileName, options.pathOptions), context,
                offsetTime, options);
            out->_init(otioTimeline, context, options);
            return out;
        }

        std::shared_ptr<Timeline> Timeline::create(
            file::Path& path, const file::Path& audioPath,
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
