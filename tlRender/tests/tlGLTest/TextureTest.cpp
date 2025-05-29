// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlGLTest/TextureTest.h>

#include <tlGL/GLFWWindow.h>
#include <tlGL/GL.h>
#include <tlGL/Texture.h>
#include <tlGL/TextureAtlas.h>

#include <tlCore/StringFormat.h>

using namespace tl::gl;

namespace tl
{
    namespace gl_tests
    {
        TextureTest::TextureTest(
            const std::shared_ptr<system::Context>& context) :
            ITest("gl_tests::TextureTest", context)
        {
        }

        std::shared_ptr<TextureTest>
        TextureTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<TextureTest>(new TextureTest(context));
        }

        void TextureTest::run()
        {
            std::shared_ptr<GLFWWindow> window;
            try
            {
                window = GLFWWindow::create(
                    "TextureTest", math::Size2i(1, 1), _context,
                    static_cast<int>(GLFWWindowOptions::MakeCurrent));
            }
            catch (const std::exception& e)
            {
                _printError(e.what());
            }
            if (window)
            {
                _texture();
                _textureAtlas();
            }
        }

        void TextureTest::_texture()
        {
            {
                TextureOptions options;
                options.pbo = true;
                TLRENDER_ASSERT(options == options);
                TLRENDER_ASSERT(options != TextureOptions());
            }
            struct TestData
            {
                image::Size size;
                image::PixelType pixelType = image::PixelType::None;
                bool pbo = false;
            };
            const std::vector<TestData> dataList = {
                {image::Size(0, 0), image::PixelType::None, false},
                {image::Size(0, 0), image::PixelType::None, true},
                {image::Size(100, 200), image::PixelType::RGBA_U8, false},
                {image::Size(100, 200), image::PixelType::RGBA_U8, true}};
            for (const auto& data : dataList)
            {
                try
                {
                    const image::Info info(data.size, data.pixelType);
                    TextureOptions options;
                    options.pbo = data.pbo;
                    auto texture = Texture::create(info, options);
                    TLRENDER_ASSERT(texture->getID());
                    TLRENDER_ASSERT(texture->getInfo() == info);
                    TLRENDER_ASSERT(texture->getSize() == info.size);
                    TLRENDER_ASSERT(texture->getWidth() == info.size.w);
                    TLRENDER_ASSERT(texture->getHeight() == info.size.h);
                    TLRENDER_ASSERT(texture->getPixelType() == info.pixelType);
                    auto image = image::Image::create(info);
                    texture->copy(image);
                    auto image2 = image::Image::create(
                        data.size.w / 2, data.size.h / 2, data.pixelType);
                    texture->copy(image2, 0, 0);
                    texture->copy(image->getData(), info);
                    texture->bind();
                }
                catch (const std::exception& e)
                {
                    _printError(e.what());
                }
            }
        }

        void TextureTest::_textureAtlas()
        {
            struct TestData
            {
                size_t textureCount = 0;
                int textureSize = 0;
                image::PixelType textureType = image::PixelType::None;
            };
            const std::vector<TestData> dataList = {
                {0, 0, image::PixelType::None},
                {1, 0, image::PixelType::None},
                {1, 104, image::PixelType::RGBA_U8}};
            for (const auto& data : dataList)
            {
                try
                {
                    auto atlas = TextureAtlas::create(
                        data.textureCount, data.textureSize, data.textureType);
                    TLRENDER_ASSERT(
                        atlas->getTextureCount() == data.textureCount);
                    TLRENDER_ASSERT(
                        atlas->getTextureSize() == data.textureSize);
                    TLRENDER_ASSERT(
                        atlas->getTextureType() == data.textureType);
                    TLRENDER_ASSERT(
                        atlas->getTextures().size() == data.textureCount);
                    std::vector<TextureAtlasID> ids;
                    auto image =
                        image::Image::create(50, 50, image::PixelType::RGBA_U8);
                    for (size_t i = 0; i < 8; ++i)
                    {
                        TextureAtlasItem item;
                        ids.push_back(atlas->addItem(image, item));
                    }
                    for (auto id : ids)
                    {
                        TextureAtlasItem item;
                        atlas->getItem(id, item);
                    }
                    _print(string::Format("Texture atlas: {0}%")
                               .arg(atlas->getPercentageUsed()));
                }
                catch (const std::exception& e)
                {
                    _printError(e.what());
                }
            }
        }
    } // namespace gl_tests
} // namespace tl
