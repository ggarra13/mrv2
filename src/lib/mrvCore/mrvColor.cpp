
#include "mrvCore/mrvColor.h"

namespace mrv
{
    namespace color
    {
        using namespace tl;
        
        image::Color4f fromVoidPtr(const void* ptr, const image::PixelType pixelType)
        {
            image::Color4f out(0.F, 0.F, 0.F, 1.F);
            switch (pixelType)
            {
            case image::PixelType::RGB_F32:
            {
                const float* pixels = reinterpret_cast<const float*>(ptr);
                out.r = pixels[0];
                out.g = pixels[1];
                out.b = pixels[2];
                break;
            }
            case image::PixelType::RGBA_F32:
            {
                const float* pixels = reinterpret_cast<const float*>(ptr);
                out.r = pixels[0];
                out.g = pixels[1];
                out.b = pixels[2];
                out.a = pixels[3];
                break;
            }
            case image::PixelType::RGB_F16:
            {
                const half* pixels = reinterpret_cast<const half*>(ptr);
                out.r = pixels[0];
                out.g = pixels[1];
                out.b = pixels[2];
                break;
            }
            case image::PixelType::RGBA_F16:
            {
                const half* pixels = reinterpret_cast<const half*>(ptr);
                out.r = pixels[0];
                out.g = pixels[1];
                out.b = pixels[2];
                out.a = pixels[3];
                break;
            }
            case image::PixelType::RGB_U8:
            {
                const uint8_t* pixels = reinterpret_cast<const uint8_t*>(ptr);
                out.r = static_cast<float>(pixels[0]) / 255.F;
                out.g = static_cast<float>(pixels[1]) / 255.F;
                out.b = static_cast<float>(pixels[2]) / 255.F;
                break;
            }
            case image::PixelType::RGBA_U8:
            {
                const uint8_t* pixels = reinterpret_cast<const uint8_t*>(ptr);
                out.r = static_cast<float>(pixels[0]) / 255.F;
                out.g = static_cast<float>(pixels[1]) / 255.F;
                out.b = static_cast<float>(pixels[2]) / 255.F;
                out.a = static_cast<float>(pixels[3]) / 255.F;
                break;
            }
            case image::PixelType::RGB_U16:
            {
                const uint16_t* pixels = reinterpret_cast<const uint16_t*>(ptr);
                out.r = static_cast<float>(pixels[0]) / 65535.F;
                out.g = static_cast<float>(pixels[1]) / 65535.F;
                out.b = static_cast<float>(pixels[2]) / 65535.F;
                break;
            }
            case image::PixelType::RGBA_U16:
            {
                const uint16_t* pixels = reinterpret_cast<const uint16_t*>(ptr);
                out.r = static_cast<float>(pixels[0]) / 65535.F;
                out.g = static_cast<float>(pixels[1]) / 65535.F;
                out.b = static_cast<float>(pixels[2]) / 65535.F;
                out.a = static_cast<float>(pixels[3]) / 65535.F;
                break;
            }
            case image::PixelType::L_F32:
            {
                const float* pixels = reinterpret_cast<const float*>(ptr);
                out.r = static_cast<float>(pixels[0]) / 65535.F;
                out.g = static_cast<float>(pixels[0]) / 65535.F;
                out.b = static_cast<float>(pixels[0]) / 65535.F;
                break;
            }
            case image::PixelType::LA_F32:
            {
                const float* pixels = reinterpret_cast<const float*>(ptr);
                out.r = static_cast<float>(pixels[0]) / 65535.F;
                out.g = static_cast<float>(pixels[0]) / 65535.F;
                out.b = static_cast<float>(pixels[0]) / 65535.F;
                out.a = static_cast<float>(pixels[1]) / 65535.F;
                break;
            }
            case image::PixelType::L_F16:
            {
                const half* pixels = reinterpret_cast<const half*>(ptr);
                out.r = pixels[0];
                out.g = pixels[0];
                out.b = pixels[0];
                break;
            }
            case image::PixelType::LA_F16:
            {
                const half* pixels = reinterpret_cast<const half*>(ptr);
                out.r = pixels[0];
                out.g = pixels[0];
                out.b = pixels[0];
                out.a = pixels[1];
                break;
            }
            case image::PixelType::L_U8:
            {
                const uint16_t* pixels = reinterpret_cast<const uint16_t*>(ptr);
                out.r = static_cast<float>(pixels[0]) / 255.F;
                out.g = static_cast<float>(pixels[0]) / 255.F;
                out.b = static_cast<float>(pixels[0]) / 255.F;
                break;
            }
            case image::PixelType::LA_U8:
            {
                const uint8_t* pixels = reinterpret_cast<const uint8_t*>(ptr);
                out.r = static_cast<float>(pixels[0]) / 255.F;
                out.g = static_cast<float>(pixels[0]) / 255.F;
                out.b = static_cast<float>(pixels[0]) / 255.F;
                out.a = static_cast<float>(pixels[1]) / 255.F;
                break;
            }
            case image::PixelType::L_U16:
            {
                const uint16_t* pixels = reinterpret_cast<const uint16_t*>(ptr);
                out.r = static_cast<float>(pixels[0]) / 65535.F;
                out.g = static_cast<float>(pixels[0]) / 65535.F;
                out.b = static_cast<float>(pixels[0]) / 65535.F;
                break;
            }
            case image::PixelType::LA_U16:
            {
                const uint16_t* pixels = reinterpret_cast<const uint16_t*>(ptr);
                out.r = static_cast<float>(pixels[0]) / 65535.F;
                out.g = static_cast<float>(pixels[0]) / 65535.F;
                out.b = static_cast<float>(pixels[0]) / 65535.F;
                out.a = static_cast<float>(pixels[1]) / 65535.F;
                break;
            }
            default:
                break;
            }
            return out;
        }
    }  // namespace color
} // namespace mrv
