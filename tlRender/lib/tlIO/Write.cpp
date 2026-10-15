// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlIO/Write.h>

#include <tlCore/LogSystem.h>

namespace tl
{
    namespace io
    {
        void IWrite::_init(
            const file::Path& path,
            const io::Options& options,
            const io::Info& info,
            const std::shared_ptr<log::System>& logSystem)
        {
            IIO::_init(path, options, logSystem);
            _info = info;
        }

        IWrite::IWrite()
        {}

        IWrite::~IWrite()
        {}

        void IWrite::writeAudio(
            const OTIO_NS::TimeRange&,
            const std::shared_ptr<audio::Audio>&,
            const io::Options&)
        {}

        void IWrite::finish()
        {}

        struct IWritePlugin::Private
        {
        };

        void IWritePlugin::_init(
            const std::string& name,
            const std::map<std::string, FileType>& extensions,
            const std::shared_ptr<log::System>& logSystem)
        {
            IIOPlugin::_init(name, extensions, logSystem);
        }

        IWritePlugin::IWritePlugin() :
            _p(new Private)
        {}

        IWritePlugin::~IWritePlugin()
        {}

        bool IWritePlugin::_isCompatible(const image::Info& info, const io::Options& options) const
        {
            return info.pixelType != image::PixelType::kNone &&
                info == getInfo(info, options);
        }
    }
}
