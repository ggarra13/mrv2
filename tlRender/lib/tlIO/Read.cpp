// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlIO/Read.h>

#include <tlCore/LogSystem.h>

namespace tl
{
    namespace io
    {
        void IRead::_init(
            const file::Path& path,
            const std::vector<file::MemoryRead>& mem,
            const Options& options,
            const std::shared_ptr<log::System>& logSystem)
        {
            IIO::_init(path, options, logSystem);
            _mem = mem;
        }

        IRead::IRead()
        {}

        IRead::~IRead()
        {}

        std::string IRead::getError() const
        {
            return std::string();
        }

        size_t IRead::getErrorCount() const
        {
            return 0;
        }

        IVideoRead::~IVideoRead()
        {}

        IAudioRead::~IAudioRead()
        {}

        struct IReadPlugin::Private
        {
        };

        void IReadPlugin::_init(
            const std::string& name,
            const std::map<std::string, FileType>& extensions,
            const std::shared_ptr<log::System>& logSystem)
        {
            IIOPlugin::_init(name, extensions, logSystem);
        }

        IReadPlugin::IReadPlugin() :
            _p(new Private)
        {}

        IReadPlugin::~IReadPlugin()
        {}

        std::shared_ptr<IVideoRead> IReadPlugin::videoRead(
            const file::Path& path,
            const Options& options)
        {
            return videoRead(path, {}, options);
        }

        std::shared_ptr<IVideoRead> IReadPlugin::videoRead(
            const file::Path&,
            const std::vector<file::MemoryRead>&,
            const Options&)
        {
            return nullptr;
        }

        std::shared_ptr<IAudioRead> IReadPlugin::audioRead(
            const file::Path& path,
            const Options& options)
        {
            return audioRead(path, {}, options);
        }

        std::shared_ptr<IAudioRead> IReadPlugin::audioRead(
            const file::Path&,
            const std::vector<file::MemoryRead>&,
            const Options&)
        {
            return nullptr;
        }

        std::shared_ptr<IDecode> IReadPlugin::decode(const Options&)
        {
            return nullptr;
        }
    }
}
