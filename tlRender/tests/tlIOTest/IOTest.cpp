// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIOTest/IOTest.h>

#include <tlIO/System.h>

#include <tlCore/Assert.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <sstream>

using namespace tl::io;

namespace tl
{
    namespace io_tests
    {
        IOTest::IOTest(const std::shared_ptr<system::Context>& context) :
            ITest("IOTest::IOTest", context)
        {
        }

        std::shared_ptr<IOTest>
        IOTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<IOTest>(new IOTest(context));
        }

        void IOTest::run()
        {
            _videoData();
            _ioSystem();
        }

        void IOTest::_videoData()
        {
            {
                const VideoData v;
                TLRENDER_ASSERT(!time::isValid(v.time));
                TLRENDER_ASSERT(!v.image);
            }
            {
                const auto time = otime::RationalTime(1.0, 24.0);
                const uint16_t layer = 1;
                const auto image =
                    image::Image::create(160, 80, image::PixelType::L_U8);
                const VideoData v(time, layer, image);
                TLRENDER_ASSERT(time.strictly_equal(v.time));
                TLRENDER_ASSERT(layer == v.layer);
                TLRENDER_ASSERT(image == v.image);
            }
            {
                const auto time = otime::RationalTime(1.0, 24.0);
                const uint16_t layer = 1;
                const auto image =
                    image::Image::create(16, 16, image::PixelType::L_U8);
                const VideoData a(time, layer, image);
                VideoData b(time, layer, image);
                TLRENDER_ASSERT(a == b);
                b.time = otime::RationalTime(2.0, 24.0);
                TLRENDER_ASSERT(a != b);
                TLRENDER_ASSERT(a < b);
            }
        }

        namespace
        {
            class DummyPlugin : public IPlugin
            {
            public:
                std::shared_ptr<IRead>
                read(const file::Path&, const Options& = Options()) override
                {
                    return nullptr;
                }

                image::Info getWriteInfo(
                    const image::Info&,
                    const io::Options& = io::Options()) const override
                {
                    return image::Info();
                }

                std::shared_ptr<IWrite> write(
                    const file::Path&, const Info&,
                    const Options& = Options()) override
                {
                    return nullptr;
                }
            };
        } // namespace

        void IOTest::_ioSystem()
        {
            auto system = _context->getSystem<System>();
            {
                std::vector<std::string> plugins;
                for (const auto& plugin : system->getPlugins())
                {
                    plugins.push_back(plugin->getName());
                }
                std::stringstream ss;
                ss << "Plugins: " << string::join(plugins, ", ");
                _print(ss.str());
            }
            {
                std::map<std::string, std::shared_ptr<IPlugin> > plugins;
                for (const auto& plugin : system->getPlugins())
                {
                    const auto& extensions = plugin->getExtensions();
                    if (!extensions.empty())
                    {
                        plugins[*(extensions.begin())] = plugin;
                    }
                }
                for (const auto& plugin : plugins)
                {
                    TLRENDER_ASSERT(
                        system->getPlugin(file::Path("test" + plugin.first)) ==
                        plugin.second);
                }
                TLRENDER_ASSERT(!system->getPlugin(file::Path()));
                TLRENDER_ASSERT(!system->getPlugin<DummyPlugin>());
            }
            {
                std::vector<std::string> extensions;
                for (const auto& extension : system->getExtensions())
                {
                    extensions.push_back(extension);
                }
                std::stringstream ss;
                ss << "Extensions: " << string::join(extensions, ", ");
                _print(ss.str());
            }
            TLRENDER_ASSERT(!system->read(file::Path()));
            TLRENDER_ASSERT(!system->write(file::Path(), Info()));
        }
    } // namespace io_tests
} // namespace tl
