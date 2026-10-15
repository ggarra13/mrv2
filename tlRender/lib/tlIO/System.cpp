// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the tlRender project.

#include <tlIO/System.h>


#include <tlIO/Cineon.h>
#include <tlIO/DPX.h>
#include <tlIO/PPM.h>
#include <tlIO/SGI.h>
#include <tlIO/SVG.h>
#if defined(TLRENDER_STB)
#    include <tlIO/STB.h>
#endif
#if defined(TLRENDER_FFMPEG)
#    include <tlIO/FFmpeg.h>
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_JPEG)
#    include <tlIO/JPEG.h>
#endif // TLRENDER_JPEG
#if defined(TLRENDER_EXR)
#    include <tlIO/OpenEXR.h>
#endif // TLRENDER_EXR
#if defined(TLRENDER_NDI)
#    include <tlIO/NDI.h>
#endif // TLRENDER_NDI
#if defined(TLRENDER_PNG)
#    include <tlIO/PNG.h>
#endif // TLRENDER_PNG
#if defined(TLRENDER_TIFF)
#    include <tlIO/TIFF.h>
#endif // TLRENDER_TIFF
#if defined(TLRENDER_USD)
#    include <tlIO/USDPlugin.h>
#    include <tlIO/USD.h>
#endif // TLRENDER_USD
#if defined(TLRENDER_RAW)
#    include <tlIO/RAW.h>
#endif // TLRENDER_RAW
#if defined(TLRENDER_ZFILE)
#    include <tlIO/ZFile.h>
#endif

#include <tlCore/Context.h>
#include <tlCore/String.h>

#include <iomanip>
#include <sstream>

namespace tl
{
    namespace io
    {
        struct ReadSystem::Private
        {
            std::vector<std::string> names;
        };

        ReadSystem::ReadSystem(const std::shared_ptr<system::Context>& context) :
            system::ISystem(context, "tl::ReadSystem"),
            _p(new Private)
        {
            TLRENDER_P();

            auto logSystem = context->getLogSystem();
            if (auto context = _context.lock())
            {
                _plugins.push_back(cineon::ReadPlugin::create(logSystem));
                _plugins.push_back(dpx::ReadPlugin::create(logSystem));
                _plugins.push_back(ppm::ReadPlugin::create(logSystem));
                _plugins.push_back(sgi::ReadPlugin::create(logSystem));
                _plugins.push_back(svg::ReadPlugin::create(logSystem));
#if defined(TLRENDER_EXR)
                _plugins.push_back(exr::ReadPlugin::create(logSystem));
#endif // TLRENDER_EXR
#if defined(TLRENDER_JPEG)
                _plugins.push_back(jpeg::ReadPlugin::create(logSystem));
#endif
#if defined(TLRENDER_FFMPEG)
                _plugins.push_back(ffmpeg::ReadPlugin::create(logSystem));
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_NDI)
            _plugins.push_back(ndi::ReadPlugin::create(logSystem));
#endif // TLRENDER_NDI
#if defined(TLRENDER_PNG)
                _plugins.push_back(png::ReadPlugin::create(logSystem));
#endif // TLRENDER_PNG
#if defined(TLRENDER_STB)
                _plugins.push_back(stb::ReadPlugin::create(logSystem));
#endif // TLRENDER_STB
#if defined(TLRENDER_TIFF)
                _plugins.push_back(tiff::ReadPlugin::create(logSystem));
#endif // TLRENDER_TIFF
#if defined(TLRENDER_USD) && !defined(VULKAN_BACKEND)
                _plugins.push_back(usd::ReadPlugin::create(logSystem));
#endif // TLRENDER_USD
#if defined(TLRENDER_RAW)
                _plugins.push_back(raw::ReadPlugin::create(logSystem));
#endif // TLRENDER_RAW
#if defined(TLRENDER_ZFILE)
                _plugins.push_back(zfile::ReadPlugin::create(logSystem));
#endif // TLRENDER_ZFILE
            }

            for (const auto& plugin : _plugins)
            {
                p.names.push_back(plugin->getPluginName());
            }
            logSystem->print("tl::ReadSystem", "Plugins: " +
                             string::join(p.names, ", "));
        }

        ReadSystem::~ReadSystem()
        {}

        std::shared_ptr<ReadSystem> ReadSystem::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = context->getSystem<ReadSystem>();
            if (!out)
            {
                out = std::shared_ptr<ReadSystem>(new ReadSystem(context));
                context->addSystem(out);
            }
            return out;
        }

        std::shared_ptr<IReadPlugin> ReadSystem::getPlugin(const file::Path& path) const
        {
            const std::string ext = string::toLower(path.getExtension());
            for (const auto& i : _plugins)
            {
                const auto& exts = i->getExts();
                if (exts.find(ext) != exts.end())
                {
                    return i;
                }
            }
            return nullptr;
        }

        void ReadSystem::addPlugin(const std::shared_ptr<IReadPlugin>& plugin)
        {
            _plugins.push_back(plugin);
        }

        void ReadSystem::removePlugin(const std::shared_ptr<IReadPlugin>& plugin)
        {
            const auto i = std::find(_plugins.begin(), _plugins.end(), plugin);
            if (i != _plugins.end())
            {
                _plugins.erase(i);
            }
        }

        const std::vector<std::string>& ReadSystem::getNames() const
        {
            return _p->names;
        }

        std::set<std::string> ReadSystem::getExts(int types) const
        {
            std::set<std::string> out;
            for (const auto& i : _plugins)
            {
                const auto& exts = i->getExts(types);
                out.insert(exts.begin(), exts.end());
            }
            return out;
        }

        FileType ReadSystem::getFileType(const std::string& ext) const
        {
            FileType out = FileType::Unknown;
            const std::string lower = string::toLower(ext);
            for (const auto& plugin : _plugins)
            {
                for (auto fileType : {
                        FileType::Movie,
                        FileType::Sequence,
                        FileType::Audio })
                {
                    const auto& exts = plugin->getExts(static_cast<int>(fileType));
                    if (const auto i = exts.find(lower); i != exts.end())
                    {
                        out = fileType;
                        break;
                    }
                }
            }
            return out;
        }

        namespace
        {
            // Hand the path to the first plugin that claims its extension. If
            // that plugin throws, keep trying the others and report the last
            // error only when none of them produced a reader.
            template<typename T, typename Create>
            std::shared_ptr<T> createRead(
                const std::vector<std::shared_ptr<IReadPlugin> >& plugins,
                const file::Path& path,
                Create&& create)
            {
                const std::string ext = string::toLower(path.getExtension());
                std::string err;
                for (const auto& i : plugins)
                {
                    try
                    {
                        const auto& exts = i->getExts();
                        if (exts.find(ext) != exts.end())
                        {
                            return create(i);
                        }
                    }
                    catch (const std::exception& e)
                    {
                        err = e.what();
                    }
                }
                if (!err.empty())
                {
                    throw std::runtime_error(err);
                }
                return nullptr;
            }
        }

        std::shared_ptr<io::IVideoRead> ReadSystem::videoRead(
            const file::Path& path,
            const io::Options& options)
        {
            return createRead<io::IVideoRead>(
                _plugins,
                path,
                [&](const std::shared_ptr<IReadPlugin>& plugin)
                    {
                        return plugin->videoRead(path, options);
                    });
        }

        std::shared_ptr<io::IVideoRead> ReadSystem::videoRead(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memory,
            const io::Options& options)
        {
            return createRead<io::IVideoRead>(
                _plugins,
                path,
                [&](const std::shared_ptr<IReadPlugin>& plugin)
                    {
                        return plugin->videoRead(path, memory, options);
                    });
        }

        std::shared_ptr<io::IAudioRead> ReadSystem::audioRead(
            const file::Path& path,
            const io::Options& options)
        {
            return createRead<io::IAudioRead>(
                _plugins,
                path,
                [&](const std::shared_ptr<IReadPlugin>& plugin)
                    {
                        return plugin->audioRead(path, options);
                    });
        }

        std::shared_ptr<io::IAudioRead> ReadSystem::audioRead(
            const file::Path& path,
            const std::vector<file::MemoryRead>& memory,
            const io::Options& options)
        {
            return createRead<io::IAudioRead>(
                _plugins,
                path,
                [&](const std::shared_ptr<IReadPlugin>& plugin)
                    {
                        return plugin->audioRead(path, memory, options);
                    });
        }

        struct WriteSystem::Private
        {
            std::vector<std::string> names;
        };

        WriteSystem::WriteSystem(const std::shared_ptr<system::Context>& context) :
            system::ISystem(context, "tl::WriteSystem"),
            _p(new Private)
        {
            TLRENDER_P();

            auto logSystem = context->getLogSystem();
            if (auto context = _context.lock())
            {
                _plugins.push_back(cineon::WritePlugin::create(logSystem));
                _plugins.push_back(dpx::WritePlugin::create(logSystem));
                _plugins.push_back(ppm::WritePlugin::create(logSystem));
                _plugins.push_back(sgi::WritePlugin::create(logSystem));
#if defined(TLRENDER_EXR)
                _plugins.push_back(exr::WritePlugin::create(logSystem));
#endif // TLRENDER_EXR
#if defined(TLRENDER_JPEG)
                _plugins.push_back(jpeg::WritePlugin::create(logSystem));
#endif
#if defined(TLRENDER_FFMPEG)
                _plugins.push_back(ffmpeg::WritePlugin::create(logSystem));
#endif // TLRENDER_FFMPEG
#if defined(TLRENDER_PNG)
                _plugins.push_back(png::WritePlugin::create(logSystem));
#endif // TLRENDER_PNG
#if defined(TLRENDER_STB)
                _plugins.push_back(stb::WritePlugin::create(logSystem));
#endif // TLRENDER_STB
#if defined(TLRENDER_TIFF)
                _plugins.push_back(tiff::WritePlugin::create(logSystem));
#endif // TLRENDER_TIFF
            }

            for (const auto& plugin : _plugins)
            {
                p.names.push_back(plugin->getPluginName());
            }
            logSystem->print("tl::WriteSystem", "Plugins: " + string::join(p.names, ", "));
        }

        WriteSystem::~WriteSystem()
        {}

        std::shared_ptr<WriteSystem> WriteSystem::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = context->getSystem<WriteSystem>();
            if (!out)
            {
                out = std::shared_ptr<WriteSystem>(new WriteSystem(context));
                context->addSystem(out);
            }
            return out;
        }

        std::shared_ptr<IWritePlugin> WriteSystem::getPlugin(const file::Path& path) const
        {
            const std::string ext = string::toLower(path.getExtension());
            for (const auto& i : _plugins)
            {
                const auto& exts = i->getExts();
                if (exts.find(ext) != exts.end())
                {
                    return i;
                }
            }
            return nullptr;
        }

        void WriteSystem::addPlugin(const std::shared_ptr<IWritePlugin>& plugin)
        {
            _plugins.push_back(plugin);
        }

        void WriteSystem::removePlugin(const std::shared_ptr<IWritePlugin>& plugin)
        {
            const auto i = std::find(_plugins.begin(), _plugins.end(), plugin);
            if (i != _plugins.end())
            {
                _plugins.erase(i);
            }
        }

        const std::vector<std::string>& WriteSystem::getNames() const
        {
            return _p->names;
        }

        std::set<std::string> WriteSystem::getExts(int types) const
        {
            std::set<std::string> out;
            for (const auto& i : _plugins)
            {
                const auto& exts = i->getExts(types);
                out.insert(exts.begin(), exts.end());
            }
            return out;
        }

        FileType WriteSystem::getFileType(const std::string& ext) const
        {
            FileType out = FileType::Unknown;
            const std::string lower = string::toLower(ext);
            for (const auto& plugin : _plugins)
            {
                for (auto fileType : {
                        FileType::Movie,
                        FileType::Sequence,
                        FileType::Audio })
                {
                    const auto& exts = plugin->getExts(static_cast<int>(fileType));
                    if (const auto i = exts.find(lower); i != exts.end())
                    {
                        out = fileType;
                        break;
                    }
                }
            }
            return out;
        }

        std::shared_ptr<IWrite> WriteSystem::write(
            const file::Path& path,
            const io::Info& info,
            const io::Options& options)
        {
            const std::string ext = string::toLower(path.getExtension());
            for (const auto& i : _plugins)
            {
                const auto& exts = i->getExts();
                if (exts.find(ext) != exts.end())
                {
                    return i->write(path, info, options);
                }
            }
            return nullptr;
        }

    }
}
