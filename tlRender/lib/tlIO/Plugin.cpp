// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlIO/Plugin.h>

#include <tlCore/LogSystem.h>

namespace tl
{
    namespace io
    {
        void IIO::_init(
            const file::Path& path,
            const io::Options& options,
            const std::shared_ptr<log::System>& logSystem)
        {
            _path = path;
            _options = options;
            _logSystem = logSystem;
        }

        namespace
        {
            std::atomic<size_t> objectCount = 0;
        }

        IIO::IIO()
        {
            ++objectCount;
        }

        IIO::~IIO()
        {
            --objectCount;
        }

        const file::Path& IIO::getPath() const
        {
            return _path;
        }

        size_t IIO::getObjectCount()
        {
            return objectCount;
        }

        struct IIOPlugin::Private
        {
            std::string pluginName;
            std::map<std::string, FileType> exts;
        };

        void IIOPlugin::_init(
            const std::string& name,
            const std::map<std::string, FileType>& exts,
            const std::shared_ptr<log::System>& logSystem)
        {
            TLRENDER_P();
            _logSystem = logSystem;
            p.pluginName = name;
            p.exts = exts;
        }

        IIOPlugin::IIOPlugin() :
            _p(new Private)
        {}

        IIOPlugin::~IIOPlugin()
        {}

        const std::string& IIOPlugin::getPluginName() const
        {
            return _p->pluginName;
        }

        std::string IIOPlugin::getPluginInfo(const io::Options&) const
        {
            return std::string();
        }

        std::set<std::string> IIOPlugin::getExts(int types) const
        {
            std::set<std::string> out;
            for (const auto& i : _p->exts)
            {
                if (static_cast<int>(i.second) & types)
                {
                    out.insert(i.first);
                }
            }
            return out;
        }
    }
}
