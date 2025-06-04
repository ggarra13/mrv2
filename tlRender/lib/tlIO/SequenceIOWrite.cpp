// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/SequenceIO.h>

#include <tlCore/Assert.h>
#include <tlCore/File.h>
#include <tlCore/LogSystem.h>
#include <tlCore/StringFormat.h>

#include <cstring>
#include <sstream>

namespace tl
{
    namespace io
    {
        struct ISequenceWrite::Private
        {
            std::string path;
            std::string baseName;
            std::string number;
            int pad = 0;
            std::string extension;

            float defaultSpeed = sequenceDefaultSpeed;
        };

        void ISequenceWrite::_init(
            const file::Path& path, const Info& info, const Options& options,
            const std::weak_ptr<log::System>& logSystem)
        {
            IWrite::_init(path, options, info, logSystem);

            TLRENDER_P();

            const auto i = options.find("SequenceIO/DefaultSpeed");
            if (i != options.end())
            {
                std::stringstream ss(i->second);
                ss >> p.defaultSpeed;
            }
        }

        ISequenceWrite::ISequenceWrite() :
            _p(new Private)
        {
        }

        ISequenceWrite::~ISequenceWrite() {}

        void ISequenceWrite::writeVideo(
            const otime::RationalTime& time,
            const std::shared_ptr<image::Image>& image, const Options& options)
        {
            _writeVideo(
                _path.get(static_cast<int>(time.value())), time, image,
                merge(options, _options));
        }
    } // namespace io
} // namespace tl
