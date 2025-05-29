// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlGLTest/OffscreenBufferTest.h>

#include <tlGL/GLFWWindow.h>
#include <tlGL/GL.h>
#include <tlGL/OffscreenBuffer.h>

#include <tlCore/StringFormat.h>

using namespace tl::gl;

namespace tl
{
    namespace gl_tests
    {
        OffscreenBufferTest::OffscreenBufferTest(
            const std::shared_ptr<system::Context>& context) :
            ITest("gl_tests::OffscreenBufferTest", context)
        {
        }

        std::shared_ptr<OffscreenBufferTest> OffscreenBufferTest::create(
            const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<OffscreenBufferTest>(
                new OffscreenBufferTest(context));
        }

        void OffscreenBufferTest::run()
        {
            std::shared_ptr<GLFWWindow> window;
            try
            {
                window = GLFWWindow::create(
                    "OffscreenBufferTest", math::Size2i(1, 1), _context,
                    static_cast<int>(GLFWWindowOptions::MakeCurrent));
            }
            catch (const std::exception& e)
            {
                _printError(e.what());
            }
            if (window)
            {
                _enums();
                _buffer();
            }
        }

        void OffscreenBufferTest::_enums()
        {
            _enum<OffscreenDepth>("OffscreenDepth", getOffscreenDepthEnums);
            _enum<OffscreenStencil>(
                "OffscreenStencil", getOffscreenStencilEnums);
            _enum<OffscreenSampling>(
                "OffscreenSampling", getOffscreenSamplingEnums);
        }

        void OffscreenBufferTest::_buffer()
        {
            {
                OffscreenBufferOptions options;
                options.colorType = offscreenColorDefault;
                TLRENDER_ASSERT(options == options);
                TLRENDER_ASSERT(options != OffscreenBufferOptions());
            }
            struct TestData
            {
                math::Size2i size;
                image::PixelType colorType = image::PixelType::None;
                OffscreenDepth depth = OffscreenDepth::None;
                OffscreenStencil stencil = OffscreenStencil::None;
                OffscreenSampling sampling = OffscreenSampling::None;
            };
            const std::vector<TestData> dataList = {
                {math::Size2i(0, 0), image::PixelType::None,
                 OffscreenDepth::None, OffscreenStencil::None,
                 OffscreenSampling::None},
                {math::Size2i(100, 200), offscreenColorDefault,
                 OffscreenDepth::None, OffscreenStencil::None,
                 OffscreenSampling::None},
                {math::Size2i(100, 200), offscreenColorDefault,
                 OffscreenDepth::_16, OffscreenStencil::None,
                 OffscreenSampling::None},
                {math::Size2i(100, 200), offscreenColorDefault,
                 OffscreenDepth::_24, OffscreenStencil::None,
                 OffscreenSampling::None},
                {math::Size2i(100, 200), offscreenColorDefault,
                 OffscreenDepth::_32, OffscreenStencil::None,
                 OffscreenSampling::None},
                {math::Size2i(100, 200), offscreenColorDefault,
                 OffscreenDepth::None, OffscreenStencil::_8,
                 OffscreenSampling::None},
                {math::Size2i(100, 200), offscreenColorDefault,
                 OffscreenDepth::_24, OffscreenStencil::_8,
                 OffscreenSampling::None},
                {math::Size2i(100, 200), offscreenColorDefault,
                 OffscreenDepth::_32, OffscreenStencil::_8,
                 OffscreenSampling::None},
                {math::Size2i(100, 200), offscreenColorDefault,
                 OffscreenDepth::None, OffscreenStencil::None,
                 OffscreenSampling::_2},
                {math::Size2i(100, 200), offscreenColorDefault,
                 OffscreenDepth::None, OffscreenStencil::None,
                 OffscreenSampling::_4},
                {math::Size2i(100, 200), offscreenColorDefault,
                 OffscreenDepth::None, OffscreenStencil::None,
                 OffscreenSampling::_8},
                {math::Size2i(100, 200), offscreenColorDefault,
                 OffscreenDepth::None, OffscreenStencil::None,
                 OffscreenSampling::_16}};
            for (const auto& data : dataList)
            {
                try
                {
                    OffscreenBufferOptions options;
                    options.colorType = data.colorType;
                    options.depth = data.depth;
                    options.stencil = data.stencil;
                    options.sampling = data.sampling;
                    auto buffer = OffscreenBuffer::create(data.size, options);
                    TLRENDER_ASSERT(buffer->getSize() == data.size);
                    TLRENDER_ASSERT(buffer->getWidth() == data.size.w);
                    TLRENDER_ASSERT(buffer->getHeight() == data.size.h);
                    TLRENDER_ASSERT(buffer->getOptions() == options);
                    TLRENDER_ASSERT(buffer->getID());
                    TLRENDER_ASSERT(buffer->getColorID());
                    buffer->bind();
                    TLRENDER_ASSERT(!doCreate(buffer, data.size, options));
                }
                catch (const std::exception& e)
                {
                    _printError(e.what());
                }
            }
            for (auto depth : getOffscreenDepthEnums())
            {
                try
                {
                    OffscreenBufferOptions options;
                    options.colorType = offscreenColorDefault;
                    options.depth = depth;
                    const math::Size2i size(100, 200);
                    auto buffer = OffscreenBuffer::create(size, options);
                    TLRENDER_ASSERT(buffer->getSize() == size);
                    TLRENDER_ASSERT(buffer->getWidth() == size.w);
                    TLRENDER_ASSERT(buffer->getHeight() == size.h);
                    TLRENDER_ASSERT(buffer->getOptions() == options);
                    TLRENDER_ASSERT(buffer->getID());
                    TLRENDER_ASSERT(buffer->getColorID());
                    buffer->bind();
                }
                catch (const std::exception& e)
                {
                    _printError(e.what());
                }
            }
            for (auto sampling : getOffscreenSamplingEnums())
            {
                try
                {
                    OffscreenBufferOptions options;
                    options.colorType = offscreenColorDefault;
                    options.sampling = sampling;
                    const math::Size2i size(100, 200);
                    auto buffer = OffscreenBuffer::create(size, options);
                    TLRENDER_ASSERT(buffer->getSize() == size);
                    TLRENDER_ASSERT(buffer->getWidth() == size.w);
                    TLRENDER_ASSERT(buffer->getHeight() == size.h);
                    TLRENDER_ASSERT(buffer->getOptions() == options);
                    TLRENDER_ASSERT(buffer->getID());
                    TLRENDER_ASSERT(buffer->getColorID());
                    buffer->bind();
                }
                catch (const std::exception& e)
                {
                    _printError(e.what());
                }
            }
        }
    } // namespace gl_tests
} // namespace tl
