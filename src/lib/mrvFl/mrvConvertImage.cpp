// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "mrvFl/mrvConvertImage.h"
#include "mrvFl/mrvIO.h"

#include "mrvCore/mrvPixelConverter.h"

#include <tlCore/StringFormat.h>


namespace
{
    const char* kModule = "pcvt";
}

namespace mrv
{
    
    void convertImage(std::shared_ptr<image::Image>& outputImage,
                      std::shared_ptr<image::Image>& inputImage)
    {
        const size_t numPixels = outputImage->getWidth() * outputImage->getHeight();
        const image::PixelType outputPixelType = outputImage->getPixelType();
        const image::PixelType inputPixelType = inputImage->getPixelType();

        /* xgettext:c++-format */
        const std::string err = string::Format(
            _("Unhandled buffer format: {0} for output: {1}"))
                                .arg(inputPixelType)
                                .arg(outputPixelType);
        
        switch(outputPixelType)
        {
        case image::PixelType::RGB_U8:
        {
            switch(inputPixelType)
            {
            case image::PixelType::RGBA_U8:
                convert_rgba_to_rgb_array<uint8_t, uint8_t>
                    (reinterpret_cast<uint8_t*>(outputImage->getData()),
                     reinterpret_cast<uint8_t*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGBA_U16:
                convert_rgba_to_rgb_array<uint16_t, uint8_t>
                    (reinterpret_cast<uint16_t*>(outputImage->getData()),
                     reinterpret_cast<uint8_t*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGB_F16:
                convert_rgb_array<uint8_t, half>
                    (reinterpret_cast<uint8_t*>(outputImage->getData()),
                     reinterpret_cast<half*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGB_F32:
                convert_rgb_array<uint8_t, float>
                    (reinterpret_cast<uint8_t*>(outputImage->getData()),
                     reinterpret_cast<float*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGBA_F16:
                convert_rgba_to_rgb_array<uint8_t, half>
                    (reinterpret_cast<uint8_t*>(outputImage->getData()),
                     reinterpret_cast<half*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGBA_F32:
                convert_rgba_to_rgb_array<uint8_t, float>
                    (reinterpret_cast<uint8_t*>(outputImage->getData()),
                     reinterpret_cast<float*>(inputImage->getData()),
                     numPixels);
                break;
            default:
                LOG_ERROR(err);
                break;
            }
            break;
        }
        case image::PixelType::RGB_U16:
        {
            switch(inputPixelType)
            {
            case image::PixelType::RGB_U8:
                convert_rgb_array<uint16_t, uint8_t>
                    (reinterpret_cast<uint16_t*>(outputImage->getData()),
                     reinterpret_cast<uint8_t*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGB_U16:
                convert_rgb_array<uint16_t, uint16_t>
                    (reinterpret_cast<uint16_t*>(outputImage->getData()),
                     reinterpret_cast<uint16_t*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGB_F16:
                convert_rgb_array<uint16_t, half>
                    (reinterpret_cast<uint16_t*>(outputImage->getData()),
                     reinterpret_cast<half*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGB_F32:
                convert_rgb_array<uint16_t, float>
                    (reinterpret_cast<uint16_t*>(outputImage->getData()),
                     reinterpret_cast<float*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGBA_U8:
                convert_rgba_to_rgb_array<uint16_t, uint8_t>
                    (reinterpret_cast<uint16_t*>(outputImage->getData()),
                     reinterpret_cast<uint8_t*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGBA_U16:
                convert_rgba_to_rgb_array<uint16_t, uint16_t>
                    (reinterpret_cast<uint16_t*>(outputImage->getData()),
                     reinterpret_cast<uint16_t*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGBA_F16:
                convert_rgba_to_rgb_array<uint16_t, half>
                    (reinterpret_cast<uint16_t*>(outputImage->getData()),
                     reinterpret_cast<half*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGBA_F32:
                convert_rgba_to_rgb_array<uint16_t, float>
                    (reinterpret_cast<uint16_t*>(outputImage->getData()),
                     reinterpret_cast<float*>(inputImage->getData()),
                     numPixels);
                break;
            default:
                LOG_ERROR(err);
                break;
            }
            break;
        }
        case image::PixelType::RGBA_U8:
        {
            switch(inputPixelType)
            {
            case image::PixelType::RGB_U16:
                convert_rgb_to_rgba_array<uint8_t, uint16_t>
                    (reinterpret_cast<uint8_t*>(outputImage->getData()),
                     reinterpret_cast<uint16_t*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGB_F16:
                convert_rgb_to_rgba_array<uint8_t, half>
                    (reinterpret_cast<uint8_t*>(outputImage->getData()),
                     reinterpret_cast<half*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGB_F32:
                convert_rgb_to_rgba_array<uint8_t, float>
                    (reinterpret_cast<uint8_t*>(outputImage->getData()),
                     reinterpret_cast<float*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGBA_U16:
                convert_rgba_array<uint8_t, uint16_t>
                    (reinterpret_cast<uint8_t*>(outputImage->getData()),
                     reinterpret_cast<uint16_t*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGBA_F16:
                convert_rgba_array<uint8_t, half>
                    (reinterpret_cast<uint8_t*>(outputImage->getData()),
                     reinterpret_cast<half*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGBA_F32:
                convert_rgba_array<uint8_t, float>
                    (reinterpret_cast<uint8_t*>(outputImage->getData()),
                     reinterpret_cast<float*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGBA_U8:
                outputImage = inputImage;
                break;
            default:
                LOG_ERROR(err);
                break;
            }
            break;
        }
        case image::PixelType::RGBA_U16:
        {
            switch(inputPixelType)
            {
            case image::PixelType::RGB_U8:
                convert_rgb_to_rgba_array<uint16_t, uint8_t>
                    (reinterpret_cast<uint16_t*>(outputImage->getData()),
                     reinterpret_cast<uint8_t*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGB_F16:
                convert_rgb_to_rgba_array<uint16_t, half>
                    (reinterpret_cast<uint16_t*>(outputImage->getData()),
                     reinterpret_cast<half*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGB_F32:
                convert_rgb_to_rgba_array<uint16_t, float>
                    (reinterpret_cast<uint16_t*>(outputImage->getData()),
                     reinterpret_cast<float*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGBA_U8:
                convert_rgba_array<uint16_t, uint8_t>
                    (reinterpret_cast<uint16_t*>(outputImage->getData()),
                     reinterpret_cast<uint8_t*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGBA_F16:
                convert_rgba_array<uint16_t, half>
                    (reinterpret_cast<uint16_t*>(outputImage->getData()),
                     reinterpret_cast<half*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGBA_F32:
                convert_rgba_array<uint16_t, float>
                    (reinterpret_cast<uint16_t*>(outputImage->getData()),
                     reinterpret_cast<float*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGBA_U16:
                outputImage = inputImage;
                break;
            default:
                LOG_ERROR(err);
                break;
            }
            break;
        }
        case image::PixelType::RGBA_F16:
        {
            switch(inputPixelType)
            {
            case image::PixelType::RGBA_U8:
                convert_rgba_array<half, uint8_t>
                    (reinterpret_cast<half*>(outputImage->getData()),
                     reinterpret_cast<uint8_t*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGBA_U16:
                convert_rgba_array<half, uint16_t>
                    (reinterpret_cast<half*>(outputImage->getData()),
                     reinterpret_cast<uint16_t*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGB_U8:
                convert_rgb_to_rgba_array<half, uint8_t>
                    (reinterpret_cast<half*>(outputImage->getData()),
                     reinterpret_cast<uint8_t*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGB_U16:
                convert_rgb_to_rgba_array<half, uint16_t>
                    (reinterpret_cast<half*>(outputImage->getData()),
                     reinterpret_cast<uint16_t*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGB_F16:
                convert_rgb_to_rgba_array<half, half>
                    (reinterpret_cast<half*>(outputImage->getData()),
                     reinterpret_cast<half*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGB_F32:
                convert_rgb_to_rgba_array<uint16_t, float>
                    (reinterpret_cast<uint16_t*>(outputImage->getData()),
                     reinterpret_cast<float*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGBA_F16:
                outputImage = inputImage;
                break;
            case image::PixelType::RGBA_F32:
                convert_rgba_array<half, float>
                    (reinterpret_cast<half*>(outputImage->getData()),
                     reinterpret_cast<float*>(inputImage->getData()),
                     numPixels);
                break;
            default:
                LOG_ERROR(err);
                break;
            }
            break;
        }
        case image::PixelType::RGBA_F32:
        {
            switch(inputPixelType)
            {
            case image::PixelType::RGBA_U8:
                convert_rgba_array<float, uint8_t>
                    (reinterpret_cast<float*>(outputImage->getData()),
                     reinterpret_cast<uint8_t*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGBA_U16:
                convert_rgba_array<float, uint16_t>
                    (reinterpret_cast<float*>(outputImage->getData()),
                     reinterpret_cast<uint16_t*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGB_U8:
                convert_rgb_to_rgba_array<float, uint8_t>
                    (reinterpret_cast<float*>(outputImage->getData()),
                     reinterpret_cast<uint8_t*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGB_U16:
                convert_rgb_to_rgba_array<float, uint16_t>
                    (reinterpret_cast<float*>(outputImage->getData()),
                     reinterpret_cast<uint16_t*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGB_F16:
                convert_rgb_to_rgba_array<float, half>
                    (reinterpret_cast<float*>(outputImage->getData()),
                     reinterpret_cast<half*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGB_F32:
                convert_rgb_to_rgba_array<float, float>
                    (reinterpret_cast<float*>(outputImage->getData()),
                     reinterpret_cast<float*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGBA_F16:
                convert_rgba_array<float, half>
                    (reinterpret_cast<float*>(outputImage->getData()),
                     reinterpret_cast<half*>(inputImage->getData()),
                     numPixels);
                break;
            case image::PixelType::RGBA_F32:
                outputImage = inputImage;
                break;
            default:
                LOG_ERROR(err);
                break;
            }
            break;
        }
        default:
            if (inputPixelType != outputPixelType)
            {
                LOG_ERROR("Unhandled input format: " << inputPixelType
                          << " for output: " << outputPixelType);
            }
            else
            {
                outputImage = inputImage;
            }
            break;
        }
    }
    
}  // namespace mrv
