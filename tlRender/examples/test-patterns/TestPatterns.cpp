// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "TestPatterns.h"

#include <tlCore/Math.h>
#include <tlCore/StringFormat.h>

#include <cmath>

namespace tl
{
    namespace examples
    {
        namespace test_patterns
        {
            void ITestPattern::_init(
                const std::string& name, const math::Size2i& size,
                const std::shared_ptr<system::Context>& context)
            {
                _context = context;
                _name = name;
                _size = size;
            }

            ITestPattern::ITestPattern() {}

            ITestPattern::~ITestPattern() {}

            const std::string& ITestPattern::getName() const
            {
                return _name;
            }

            void CountTestPattern::_init(
                const math::Size2i& size,
                const std::shared_ptr<system::Context>& context)
            {
                ITestPattern::_init(getClassName(), size, context);

                auto fontSystem = context->getSystem<image::FontSystem>();
                _secondsFontInfo =
                    image::FontInfo("NotoMono-Regular", _size.h / 2.F);
                _secondsFontMetrics = fontSystem->getMetrics(_secondsFontInfo);

                _framesFontInfo = image::FontInfo(
                    "NotoMono-Regular", _secondsFontInfo.size / 4.F);
                _framesFontMetrics = fontSystem->getMetrics(_framesFontInfo);
            }

            CountTestPattern::~CountTestPattern() {}

            std::string CountTestPattern::getClassName()
            {
                return "Count";
            }

            std::shared_ptr<CountTestPattern> CountTestPattern::create(
                const math::Size2i& size,
                const std::shared_ptr<system::Context>& context)
            {
                auto out =
                    std::shared_ptr<CountTestPattern>(new CountTestPattern);
                out->_init(size, context);
                return out;
            }

            void CountTestPattern::render(
                const std::shared_ptr<timeline::IRender>& render,
                const otime::RationalTime& time)
            {
                if (auto context = _context.lock())
                {
                    const otime::RationalTime seconds = time.rescaled_to(1.0);
                    const int wholeSeconds = static_cast<int>(seconds.value());
                    const int frames = static_cast<int>(time.value()) %
                                       static_cast<int>(time.rate());

                    const std::string secondsString =
                        string::Format("{0}").arg(wholeSeconds);
                    auto fontSystem = context->getSystem<image::FontSystem>();
                    const math::Size2i secondsSize =
                        fontSystem->getSize(secondsString, _secondsFontInfo);
                    const math::Vector2i secondsPos(
                        _size.w / 2.F - secondsSize.w / 2.F,
                        _size.h / 2.F - secondsSize.h / 2.F);

                    const std::string framesString =
                        string::Format("{0}").arg(frames);
                    const math::Size2i framesSize =
                        fontSystem->getSize(framesString, _framesFontInfo);
                    const math::Vector2i framesPos(
                        _size.w / 2.F - framesSize.w / 2.F,
                        secondsPos.y + secondsSize.h);

                    render->drawRect(
                        math::Box2i(0, 0, _size.w, _size.h),
                        image::Color4f(.1F, .1F, .1F));

                    /*render->drawRect(
                        math::Box2i(secondsPos.x, secondsPos.y, secondsSize.x,
                    secondsSize.y), image::Color4f(.5F, 0.F, 0.F));
                    render->drawRect(
                        math::Box2i(framesPos.x, framesPos.y, framesSize.x,
                    framesSize.y), image::Color4f(0.F, .5F, 0.F));*/

                    const size_t resolution = 100;
                    geom::TriangleMesh2 mesh;
                    mesh.v.push_back(
                        math::Vector2f(_size.w / 2.F, _size.h / 2.F));
                    for (int i = 0; i < resolution; ++i)
                    {
                        const float f = i / static_cast<float>(resolution - 1);
                        const float a = f * math::pi2;
                        const float r =
                            secondsSize.h / 2.F + framesSize.h + 10.F;
                        mesh.v.push_back(math::Vector2f(
                            _size.w / 2.F + std::cos(a) * r,
                            _size.h / 2.F + std::sin(a) * r));
                    }
                    for (int i = 1; i < resolution; ++i)
                    {
                        mesh.triangles.push_back(geom::Triangle2(
                            {geom::Vertex2(1), geom::Vertex2(i + 1),
                             geom::Vertex2(i - 1 + 1)}));
                    }
                    mesh.triangles.push_back(geom::Triangle2(
                        {geom::Vertex2(1), geom::Vertex2(1 + resolution - 1),
                         geom::Vertex2(1 + 1)}));
                    render->drawMesh(
                        mesh, math::Vector2i(), image::Color4f(.2F, .2F, .2F));

                    mesh.v.clear();
                    mesh.triangles.clear();
                    mesh.v.push_back(
                        math::Vector2f(_size.w / 2.F, _size.h / 2.F));
                    for (int i = 0; i < resolution; ++i)
                    {
                        const float v = frames / time.rate();
                        const float f = i / static_cast<float>(resolution - 1);
                        const float a = v * f * math::pi2 - math::pi / 2.F;
                        const float r =
                            secondsSize.h / 2.F + framesSize.h + 10.F;
                        mesh.v.push_back(math::Vector2f(
                            _size.w / 2.F + std::cos(a) * r,
                            _size.h / 2.F + std::sin(a) * r));
                    }
                    for (int i = 1; i < resolution; ++i)
                    {
                        mesh.triangles.push_back(geom::Triangle2(
                            {geom::Vertex2(1), geom::Vertex2(i + 1),
                             geom::Vertex2((i - 1) + 1)}));
                    }
                    render->drawMesh(
                        mesh, math::Vector2i(), image::Color4f(.3F, .3F, .3F));

                    render->drawText(
                        fontSystem->getGlyphs(secondsString, _secondsFontInfo),
                        math::Vector2i(
                            secondsPos.x,
                            secondsPos.y + _secondsFontMetrics.ascender),
                        image::Color4f(1.F, 1.F, 1.F));

                    render->drawText(
                        fontSystem->getGlyphs(framesString, _framesFontInfo),
                        math::Vector2i(
                            framesPos.x,
                            framesPos.y + _framesFontMetrics.ascender),
                        image::Color4f(1.F, 1.F, 1.F));
                }
            }

            void SwatchesTestPattern::_init(
                const math::Size2i& size,
                const std::shared_ptr<system::Context>& context)
            {
                ITestPattern::_init(getClassName(), size, context);

                const image::Info info(_size.w, 1, image::PixelType::L_F32);
                _gradient = image::Image::create(info);
                float* data = reinterpret_cast<float*>(_gradient->getData());
                for (float *p = data, v = 0.F; p < data + _size.w;
                     ++p, v += 1.F / _size.w)
                {
                    *p = v;
                }
            }

            SwatchesTestPattern::~SwatchesTestPattern() {}

            std::string SwatchesTestPattern::getClassName()
            {
                return "Swatches";
            }

            std::shared_ptr<SwatchesTestPattern> SwatchesTestPattern::create(
                const math::Size2i& size,
                const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<SwatchesTestPattern>(
                    new SwatchesTestPattern);
                out->_init(size, context);
                return out;
            }

            void SwatchesTestPattern::render(
                const std::shared_ptr<timeline::IRender>& render,
                const otime::RationalTime& time)
            {
                const std::array<image::Color4f, 8> colors = {
                    image::Color4f(0.F, 0.F, 0.F),
                    image::Color4f(1.F, 0.F, 0.F),
                    image::Color4f(1.F, 1.F, 0.F),
                    image::Color4f(0.F, 1.F, 0.F),
                    image::Color4f(0.F, 1.F, 1.F),
                    image::Color4f(0.F, 0.F, 1.F),
                    image::Color4f(1.F, 0.F, 1.F),
                    image::Color4f(1.F, 1.F, 1.F)};
                const int swatchWidth = _size.w / colors.size();
                for (int x = 0, i = 0; x < _size.w; x += swatchWidth, ++i)
                {
                    render->drawRect(
                        math::Box2i(x, 0, swatchWidth, _size.h / 2), colors[i]);
                }
                render->drawImage(
                    _gradient,
                    math::Box2i(0, _size.h / 2, _size.w, _size.h / 2));
            }

            GridTestPattern::~GridTestPattern() {}

            std::string GridTestPattern::getClassName()
            {
                return "Grid";
            }

            std::shared_ptr<GridTestPattern> GridTestPattern::create(
                const math::Size2i& size,
                const std::shared_ptr<system::Context>& context)
            {
                auto out =
                    std::shared_ptr<GridTestPattern>(new GridTestPattern);
                out->_init(getClassName(), size, context);
                return out;
            }

            void GridTestPattern::render(
                const std::shared_ptr<timeline::IRender>& render,
                const otime::RationalTime& time)
            {
                int cellSize = 2;
                switch (static_cast<int>(time.value() / 24.0) % 3)
                {
                case 1:
                    cellSize = 10;
                    break;
                case 2:
                    cellSize = 100;
                    break;
                }
                {
                    geom::TriangleMesh2 mesh;
                    for (int x = 0, i = 0; x < _size.w; x += cellSize, ++i)
                    {
                        mesh.v.push_back(math::Vector2f(x, 0.F));
                        mesh.v.push_back(math::Vector2f(x + 1, 0.F));
                        mesh.v.push_back(math::Vector2f(x + 1, _size.h));
                        mesh.v.push_back(math::Vector2f(x, _size.h));
                        mesh.triangles.push_back(geom::Triangle2(
                            {geom::Vertex2(i * 4 + 1), geom::Vertex2(i * 4 + 2),
                             geom::Vertex2(i * 4 + 3)}));
                        mesh.triangles.push_back(geom::Triangle2(
                            {geom::Vertex2(i * 4 + 3), geom::Vertex2(i * 4 + 4),
                             geom::Vertex2(i * 4 + 1)}));
                    }
                    render->drawMesh(
                        mesh, math::Vector2i(), image::Color4f(1.F, 1.F, 1.F));
                }
                {
                    geom::TriangleMesh2 mesh;
                    for (int y = 0, i = 0; y < _size.h; y += cellSize, ++i)
                    {
                        mesh.v.push_back(math::Vector2f(0.F, y));
                        mesh.v.push_back(math::Vector2f(_size.w, y));
                        mesh.v.push_back(math::Vector2f(_size.w, y + 1));
                        mesh.v.push_back(math::Vector2f(0.F, y + 1));
                        mesh.triangles.push_back(geom::Triangle2(
                            {geom::Vertex2(i * 4 + 1), geom::Vertex2(i * 4 + 2),
                             geom::Vertex2(i * 4 + 3)}));
                        mesh.triangles.push_back(geom::Triangle2(
                            {geom::Vertex2(i * 4 + 3), geom::Vertex2(i * 4 + 4),
                             geom::Vertex2(i * 4 + 1)}));
                    }
                    render->drawMesh(
                        mesh, math::Vector2i(), image::Color4f(1.F, 1.F, 1.F));
                }
            }

            std::shared_ptr<ITestPattern> TestPatternFactory::create(
                const std::string& name, const math::Size2i& size,
                const std::shared_ptr<system::Context>& context)
            {
                std::shared_ptr<ITestPattern> out;
                if (name == CountTestPattern::getClassName())
                {
                    out = CountTestPattern::create(size, context);
                }
                else if (name == SwatchesTestPattern::getClassName())
                {
                    out = SwatchesTestPattern::create(size, context);
                }
                else if (name == GridTestPattern::getClassName())
                {
                    out = GridTestPattern::create(size, context);
                }
                return out;
            };
        } // namespace test_patterns
    } // namespace examples
} // namespace tl
