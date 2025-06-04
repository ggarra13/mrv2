// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlCoreTest/ImageTest.h>

#include <tlCore/Assert.h>
#include <tlCore/Image.h>
#include <tlCore/StringFormat.h>

using namespace tl::image;

namespace tl
{
    namespace core_tests
    {
        ImageTest::ImageTest(const std::shared_ptr<system::Context>& context) :
            ITest("core_tests::ImageTest", context)
        {
        }

        std::shared_ptr<ImageTest>
        ImageTest::create(const std::shared_ptr<system::Context>& context)
        {
            return std::shared_ptr<ImageTest>(new ImageTest(context));
        }

        void ImageTest::run()
        {
            _size();
            _enums();
            _util();
            _info();
            _image();
            _serialize();
        }

        void ImageTest::_size()
        {
            {
                const Size size;
                TLRENDER_ASSERT(0 == size.w);
                TLRENDER_ASSERT(0 == size.h);
                TLRENDER_ASSERT(1.F == size.pixelAspectRatio);
                TLRENDER_ASSERT(!size.isValid());
                TLRENDER_ASSERT(0.F == size.getAspect());
            }
            {
                const Size size(1, 2);
                TLRENDER_ASSERT(1 == size.w);
                TLRENDER_ASSERT(2 == size.h);
                TLRENDER_ASSERT(1.F == size.pixelAspectRatio);
                TLRENDER_ASSERT(size.isValid());
                TLRENDER_ASSERT(.5F == size.getAspect());
            }
            {
                TLRENDER_ASSERT(Size(1, 2) == Size(1, 2));
                TLRENDER_ASSERT(Size(1, 2) != Size(1, 3));
                TLRENDER_ASSERT(Size(1, 2) < Size(1, 3));
            }
            {
                const Size size(1, 2, 2.F);
                std::stringstream ss;
                ss << size;
                Size size2;
                ss >> size2;
                TLRENDER_ASSERT(size == size2);
            }
            {
                TLRENDER_ASSERT(
                    math::Box2i(0, 0, 100, 100) ==
                    getBox(1.F, math::Box2i(0, 0, 100, 100)));
                TLRENDER_ASSERT(
                    math::Box2i(50, 0, 100, 100) ==
                    getBox(1.F, math::Box2i(0, 0, 200, 100)));
                TLRENDER_ASSERT(
                    math::Box2i(0, 50, 100, 100) ==
                    getBox(1.F, math::Box2i(0, 0, 100, 200)));
            }
        }

        void ImageTest::_enums()
        {
            _enum<PixelType>("PixelType", getPixelTypeEnums);
            _enum<VideoLevels>("VideoLevels", getVideoLevelsEnums);
            _enum<YUVCoefficients>("YUVCoefficients", getYUVCoefficientsEnums);
            for (auto i : getYUVCoefficientsEnums())
            {
                _print(string::Format("%0: %1")
                           .arg(getLabel(i))
                           .arg(getYUVCoefficients(i)));
            }
        }

        void ImageTest::_info()
        {
            {
                const Info info;
                TLRENDER_ASSERT(Size() == info.size);
                TLRENDER_ASSERT(PixelType::None == info.pixelType);
                TLRENDER_ASSERT(!info.isValid());
            }
            {
                const Info info(Size(1, 2), PixelType::L_U8);
                TLRENDER_ASSERT(Size(1, 2) == info.size);
                TLRENDER_ASSERT(PixelType::L_U8 == info.pixelType);
                TLRENDER_ASSERT(info.isValid());
            }
            {
                const Info info(1, 2, PixelType::L_U8);
                TLRENDER_ASSERT(Size(1, 2) == info.size);
                TLRENDER_ASSERT(PixelType::L_U8 == info.pixelType);
                TLRENDER_ASSERT(info.isValid());
            }
            {
                Info info(1, 2, PixelType::L_U8);
                info.layout.alignment = 1;
                TLRENDER_ASSERT(getDataByteCount(info) == 2);
            }
            {
                Info info(1, 2, PixelType::L_U8);
                info.layout.alignment = 2;
                TLRENDER_ASSERT(getDataByteCount(info) == 4);
            }
            {
                Info info(1, 2, PixelType::L_U8);
                info.layout.alignment = 4;
                TLRENDER_ASSERT(getDataByteCount(info) == 8);
            }
            {
                Info info(1, 2, PixelType::L_U16);
                info.layout.alignment = 4;
                TLRENDER_ASSERT(getDataByteCount(info) == 8);
            }
            {
                TLRENDER_ASSERT(
                    Info(1, 2, PixelType::L_U8) == Info(1, 2, PixelType::L_U8));
                TLRENDER_ASSERT(
                    Info(1, 2, PixelType::L_U8) !=
                    Info(1, 2, PixelType::L_U16));
            }
        }

        void ImageTest::_util()
        {
            for (auto i : getPixelTypeEnums())
            {
                std::stringstream ss;
                ss << i << " channel count: " << getChannelCount(i);
                _print(ss.str());
            }
            for (auto i : getPixelTypeEnums())
            {
                std::stringstream ss;
                ss << i << " bit depth: " << getBitDepth(i);
                _print(ss.str());
            }
            for (size_t c : {1, 2, 3, 4})
            {
                for (size_t b : {8, 10, 16, 32})
                {
                    std::stringstream ss;
                    ss << c << "/" << b << " int type: " << getIntType(c, b);
                    _print(ss.str());
                }
            }
            for (size_t c : {1, 2, 3, 4})
            {
                for (size_t b : {16, 32})
                {
                    std::stringstream ss;
                    ss << c << "/" << b
                       << " float type: " << getFloatType(c, b);
                    _print(ss.str());
                }
            }
            {
                TLRENDER_ASSERT(
                    getClosest(PixelType::None, {}) == PixelType::None);
                TLRENDER_ASSERT(
                    getClosest(PixelType::L_U16, {PixelType::L_U8}) ==
                    PixelType::L_U8);
                TLRENDER_ASSERT(
                    getClosest(
                        PixelType::L_U16,
                        {PixelType::L_U8, PixelType::L_U16}) ==
                    PixelType::L_U16);
                TLRENDER_ASSERT(
                    getClosest(
                        PixelType::L_U16, {PixelType::L_U8, PixelType::L_U16,
                                           PixelType::L_U32}) ==
                    PixelType::L_U16);
                TLRENDER_ASSERT(
                    getClosest(
                        PixelType::RGB_U16, {PixelType::L_U8, PixelType::L_U16,
                                             PixelType::L_U32}) ==
                    PixelType::L_U16);
                TLRENDER_ASSERT(
                    getClosest(
                        PixelType::L_U16,
                        {PixelType::RGB_U8, PixelType::RGB_U16,
                         PixelType::RGB_U32}) == PixelType::RGB_U16);
            }
            for (auto i : getPixelTypeEnums())
            {
                const Info info(1, 2, i);
                std::stringstream ss;
                ss << info.size << " " << info.pixelType
                   << " data byte count: " << getDataByteCount(info);
                _print(ss.str());
            }
        }

        void ImageTest::_image()
        {
            {
                const Info info(1, 2, PixelType::L_U8);
                auto image = Image::create(info);
                image->zero();
                TLRENDER_ASSERT(image->getInfo() == info);
                TLRENDER_ASSERT(image->getSize() == info.size);
                TLRENDER_ASSERT(image->getWidth() == info.size.w);
                TLRENDER_ASSERT(image->getHeight() == info.size.h);
                TLRENDER_ASSERT(image->getAspect() == .5F);
                TLRENDER_ASSERT(image->getPixelType() == info.pixelType);
                TLRENDER_ASSERT(image->isValid());
                TLRENDER_ASSERT(image->getData());
                TLRENDER_ASSERT(
                    static_cast<const Image*>(image.get())->getData());
            }
            {
                auto image = Image::create(Size(1, 2), PixelType::L_U8);
                TLRENDER_ASSERT(image->getWidth() == 1);
                TLRENDER_ASSERT(image->getHeight() == 2);
                TLRENDER_ASSERT(image->getPixelType() == PixelType::L_U8);
            }
            {
                auto image = Image::create(1, 2, PixelType::L_U8);
                TLRENDER_ASSERT(image->getWidth() == 1);
                TLRENDER_ASSERT(image->getHeight() == 2);
                TLRENDER_ASSERT(image->getPixelType() == PixelType::L_U8);
            }
        }

        void ImageTest::_serialize()
        {
            {
                const Size s(1, 2);
                nlohmann::json json;
                to_json(json, s);
                Size s2;
                from_json(json, s2);
                TLRENDER_ASSERT(s == s2);
            }
            {
                const Size s(1, 2);
                std::stringstream ss;
                ss << s;
                Size s2;
                ss >> s2;
                TLRENDER_ASSERT(s == s2);
            }
            {
                const Size s(1, 2, 2.F);
                std::stringstream ss;
                ss << s;
                Size s2;
                ss >> s2;
                TLRENDER_ASSERT(s == s2);
            }
            try
            {
                Size s;
                std::stringstream ss;
                ss >> s;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {
            }
            try
            {
                Size s;
                std::stringstream ss("...");
                ss >> s;
                TLRENDER_ASSERT(false);
            }
            catch (const std::exception&)
            {
            }
        }
    } // namespace core_tests
} // namespace tl
