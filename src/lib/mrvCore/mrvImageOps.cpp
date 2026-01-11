// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvFl/mrvIO.h"

#include "mrvCore/mrvImageOps.h"

namespace
{
    const char* kModule = "img";
}

namespace mrv
{
    void flipImageInY(const std::shared_ptr<image::Image> image)
    {
        const image::PixelType pixelType = image->getPixelType();
        const size_t width = image->getWidth();
        const size_t height = image->getHeight();
        switch(pixelType)
        {
        case image::PixelType::RGBA_U8:
            flipImageInY<uint8_t>(
                reinterpret_cast<uint8_t*>(image->getData()),
                width, height, 4);
            break;
        case image::PixelType::RGB_U16:
            flipImageInY<uint16_t>(
                reinterpret_cast<uint16_t*>(image->getData()),
                width, height, 3);
            break;
        case image::PixelType::RGBA_U16:
            flipImageInY<uint16_t>(
                reinterpret_cast<uint16_t*>(image->getData()),
                width, height, 4);
            break;
        case image::PixelType::RGB_F16:
            flipImageInY<half>(
                reinterpret_cast<half*>(image->getData()),
                width, height, 3);
            break;
        case image::PixelType::RGBA_F16:
            flipImageInY<half>(
                reinterpret_cast<half*>(image->getData()),
                width, height, 4);
            break;
        case image::PixelType::RGB_F32:
            flipImageInY<float>(
                reinterpret_cast<float*>(image->getData()),
                width, height, 3);
            break;
        case image::PixelType::RGBA_F32:
            flipImageInY<float>(
                reinterpret_cast<float*>(image->getData()),
                width, height, 4);
            break;
        default:
            LOG_ERROR("Unknown buffer for flipImageInY info pixel type " << pixelType);
            break;
        }
    }
                        
    void composite_RGBA_U8(std::shared_ptr<image::Image>& dest,
                           std::shared_ptr<image::Image>& src)
    {
        const image::PixelType pixelType = dest->getPixelType();
        const size_t width = dest->getWidth();
        const size_t height = dest->getHeight();
        switch(pixelType)
        {
        case image::PixelType::RGB_U8:
            compositeImageOverNoAlpha<uint8_t>(
                reinterpret_cast<uint8_t*>(
                    dest->getData()),
                src->getData(),
                width, height);
            break;
        case image::PixelType::RGB_U16:
            compositeImageOverNoAlpha<uint16_t>(
                reinterpret_cast<uint16_t*>(
                    dest->getData()),
                src->getData(),
                width, height);
            break;
        case image::PixelType::RGBA_U16:
            compositeImageOver<uint16_t>(
                reinterpret_cast<uint16_t*>(
                    dest->getData()),
                src->getData(),
                width, height);
            break;
        case image::PixelType::RGB_F16:
            compositeImageOverNoAlpha<half>(
                reinterpret_cast<half*>(
                    dest->getData()),
                src->getData(),
                width, height);
            break;
        case image::PixelType::RGBA_F16:
            compositeImageOver<half>(
                reinterpret_cast<half*>(
                    dest->getData()),
                src->getData(),
                width, height);
            break;
        case image::PixelType::RGB_F32:
            compositeImageOverNoAlpha<float>(
                reinterpret_cast<float*>(
                    dest->getData()),
                src->getData(),
                width, height);
            break;
        case image::PixelType::RGBA_F32:
            compositeImageOver<float>(
                reinterpret_cast<float*>(
                    dest->getData()),
                src->getData(),
                width, height);
            break;
        case image::PixelType::RGBA_U8:
            compositeImageOver<uint8_t>(
                reinterpret_cast<uint8_t*>(dest->getData()),
                src->getData(),
                width, height);
            break;
        default:
            LOG_ERROR("Unknown buffer for composite info pixel type " << pixelType);
            break;
        }
    }
    
    void convert_RGBA_to_RGB_U8(std::shared_ptr<image::Image>& dest,
                                   std::shared_ptr<image::Image>& src)
    {
        if (dest->getPixelType() != image::PixelType::RGB_U8)
        {
            LOG_ERROR("convert_RGBA_U8_to_RGB_U8: dest image not RGB_U8");
            return;
        }
        switch(src->getPixelType())
        {
        case image::PixelType::RGB_U8:
        {
            uint8_t* d  = dest->getData();
            const uint8_t* s  = src->getData();
            memcpy(d, s, src->getDataByteCount());
            break;
        }
        case image::PixelType::RGB_U16:
        {
            uint8_t* d  = dest->getData();
            const uint16_t* s = reinterpret_cast<uint16_t*>(src->getData());
            size_t size = dest->getWidth() * dest->getHeight();
            for (size_t i = 0; i < size; ++i)
            {
                *d++ = *s / 65535; ++s;
                *d++ = *s / 65535; ++s;
                *d++ = *s / 65535; ++s;
            }
            break;
        }
        case image::PixelType::RGB_F16:
        {
            uint8_t* d  = dest->getData();
            const half* s = reinterpret_cast<half*>(src->getData());
            size_t size = dest->getWidth() * dest->getHeight();
            for (size_t i = 0; i < size; ++i)
            {
                *d++ = *s * 255.F; ++s;
                *d++ = *s * 255.F; ++s;
                *d++ = *s * 255.F; ++s;
            }
            break;
        }
        case image::PixelType::RGB_F32:
        {
            uint8_t* d  = dest->getData();
            const float* s = reinterpret_cast<float*>(src->getData());
            size_t size = dest->getWidth() * dest->getHeight();
            for (size_t i = 0; i < size; ++i)
            {
                *d++ = *s * 255.F; ++s;
                *d++ = *s * 255.F; ++s;
                *d++ = *s * 255.F; ++s;
            }
            break;
        }
        case image::PixelType::RGBA_U8:
        {
            uint8_t* d  = dest->getData();
            const uint8_t* s  = src->getData();
            size_t size = dest->getWidth() * dest->getHeight();
            for (size_t i = 0; i < size; ++i)
            {
                *d++ = *s++;
                *d++ = *s++;
                *d++ = *s++;
                s++;
            }
            break;
        }
        case image::PixelType::RGBA_U16:
        {
            uint8_t* d  = dest->getData();
            const uint16_t* s = reinterpret_cast<uint16_t*>(src->getData());
            size_t size = dest->getWidth() * dest->getHeight();
            for (size_t i = 0; i < size; ++i)
            {
                *d++ = *s / 65535; ++s;
                *d++ = *s / 65535; ++s;
                *d++ = *s / 65535; ++s;
                s++;
            }
            break;
        }
        case image::PixelType::RGBA_F16:
        {
            uint8_t* d  = dest->getData();
            const half* s = reinterpret_cast<half*>(src->getData());
            size_t size = dest->getWidth() * dest->getHeight();
            for (size_t i = 0; i < size; ++i)
            {
                *d++ = *s * 255.F; ++s;
                *d++ = *s * 255.F; ++s;
                *d++ = *s * 255.F; ++s;
                s++;
            }
            break;
        }
        case image::PixelType::RGBA_F32:
        {
            uint8_t* d  = dest->getData();
            const float* s = reinterpret_cast<float*>(src->getData());
            size_t size = dest->getWidth() * dest->getHeight();
            for (size_t i = 0; i < size; ++i)
            {
                *d++ = *s * 255.F; ++s;
                *d++ = *s * 255.F; ++s;
                *d++ = *s * 255.F; ++s;
                s++;
            }
            break;
        }
        default:
            LOG_ERROR("convert_RGBA_U8_to_RGB_U8: src image not RGBA");
            break;
        }
    }
}
