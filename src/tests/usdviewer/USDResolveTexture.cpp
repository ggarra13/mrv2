
#include <pxr/imaging/hio/image.h>

#include "USDResolveTexture.h"

#include <tlVk/Texture.h>
#include <tlCore/Image.h>

namespace tl
{
    namespace vlk
    {
        std::shared_ptr<vlk::Texture> ResolveTexture(Fl_Vk_Context& ctx,
                                                     const std::string& resolvedPath)
        {
            using namespace PXR_NS;

            std::shared_ptr<vlk::Texture> out;

            // Hio::Image opens the file (or the file inside the .usdz) and reads the header
            HioImageSharedPtr image = HioImage::OpenForReading(resolvedPath);

            if (image) {
                int width  = image->GetWidth();
                int height = image->GetHeight();
                int depth  = 1;
        
                HioFormat hioFormat = image->GetFormat();
                VkImageType type = VK_IMAGE_TYPE_2D;
                VkFormat  format = VK_FORMAT_R8G8B8A8_UNORM;
                std::cout << "Resolution: " << width << "x" << height << std::endl;

                image::PixelType pixelType;
                // Map HioFormat to bit depth
                switch (hioFormat) {
                case HioFormatUNorm8:
                case HioFormatUNorm8srgb:
                    type = VK_IMAGE_TYPE_1D;
                    format = VK_FORMAT_R8_UNORM;
                    pixelType = image::PixelType::L_U8;
                    break;
                case HioFormatUNorm8Vec2:
                case HioFormatUNorm8Vec2srgb:
                    format = VK_FORMAT_R8G8_UNORM;
                    pixelType = image::PixelType::LA_U8;
                    break;
                case HioFormatUNorm8Vec3:
                case HioFormatUNorm8Vec3srgb:
                    format = VK_FORMAT_R8G8B8_UNORM;
                    pixelType = image::PixelType::RGB_U8;
                    break;
                case HioFormatUNorm8Vec4:
                case HioFormatUNorm8Vec4srgb:
                    format = VK_FORMAT_R8G8B8A8_UNORM;
                    pixelType = image::PixelType::RGBA_U8;
                    break;
                case HioFormatSNorm8:
                    type = VK_IMAGE_TYPE_1D;
                    format = VK_FORMAT_R8_SNORM;
                    pixelType = image::PixelType::L_U8;
                    break;
                case HioFormatSNorm8Vec2:
                    format = VK_FORMAT_R8G8_SNORM;
                    pixelType = image::PixelType::LA_U8;
                    break;
                case HioFormatSNorm8Vec3:
                    format = VK_FORMAT_R8G8B8_SNORM;
                    pixelType = image::PixelType::RGB_U8;
                    break;
                case HioFormatSNorm8Vec4:
                    format = VK_FORMAT_R8G8B8A8_SNORM;
                    pixelType = image::PixelType::RGBA_U8;
                    break;
                case HioFormatUInt16:
                    type = VK_IMAGE_TYPE_1D;
                    format = VK_FORMAT_R16_UINT;
                    pixelType = image::PixelType::L_U16;
                    break;
                case HioFormatUInt16Vec2:
                    type = VK_IMAGE_TYPE_2D;
                    format = VK_FORMAT_R16G16_UINT;
                    pixelType = image::PixelType::LA_U16;
                    break;
                case HioFormatUInt16Vec3:
                    format = VK_FORMAT_R16G16B16_UINT;
                    pixelType = image::PixelType::RGB_U16;
                    break;
                case HioFormatUInt16Vec4:
                    format = VK_FORMAT_R16G16B16A16_UINT;
                    pixelType = image::PixelType::RGBA_U16;
                    break;
                case HioFormatInt16:
                    type = VK_IMAGE_TYPE_1D;
                    format = VK_FORMAT_R16_SINT;
                    pixelType = image::PixelType::L_U16;
                    break;
                case HioFormatInt16Vec2:
                    format = VK_FORMAT_R16G16_SINT;
                    pixelType = image::PixelType::LA_U16;
                    break;
                case HioFormatInt16Vec3:
                    format = VK_FORMAT_R16G16B16_SINT;
                    pixelType = image::PixelType::RGB_U16;
                    break;
                case HioFormatInt16Vec4:
                    format = VK_FORMAT_R16G16B16A16_SINT;
                    pixelType = image::PixelType::RGBA_U16;
                    break;
                case HioFormatUInt32:
                    type = VK_IMAGE_TYPE_1D;
                    format = VK_FORMAT_R32_UINT;
                    pixelType = image::PixelType::L_U32;
                    break;
                case HioFormatUInt32Vec2:
                    format = VK_FORMAT_R32G32_UINT;
                    pixelType = image::PixelType::LA_U32;
                    break;
                case HioFormatUInt32Vec3:
                    format = VK_FORMAT_R32G32B32_UINT;
                    pixelType = image::PixelType::RGB_U32;
                    break;
                case HioFormatUInt32Vec4:
                    format = VK_FORMAT_R32G32B32A32_UINT;
                    pixelType = image::PixelType::RGBA_U32;
                    break;
                case HioFormatInt32:
                    type = VK_IMAGE_TYPE_1D;
                    format = VK_FORMAT_R32_SINT;
                    pixelType = image::PixelType::L_U32;
                    break;
                case HioFormatInt32Vec2:
                    format = VK_FORMAT_R32G32_SINT;
                    pixelType = image::PixelType::LA_U32;
                    break;
                case HioFormatInt32Vec3:
                    format = VK_FORMAT_R32G32B32_SINT;
                    pixelType = image::PixelType::RGB_U32;
                    break;
                case HioFormatInt32Vec4:
                    format = VK_FORMAT_R32G32B32A32_SINT;
                    pixelType = image::PixelType::RGBA_U32;
                    break;
                case HioFormatFloat16:
                    type = VK_IMAGE_TYPE_1D;
                    format = VK_FORMAT_R16_SFLOAT;
                    pixelType = image::PixelType::L_F16;
                    break;
                case HioFormatFloat16Vec2:
                    format = VK_FORMAT_R16G16_SFLOAT;
                    pixelType = image::PixelType::LA_F16;
                    break;
                case HioFormatFloat16Vec3:
                    format = VK_FORMAT_R16G16B16_SFLOAT;
                    pixelType = image::PixelType::RGB_F16;
                    break;
                case HioFormatFloat16Vec4:
                    format = VK_FORMAT_R16G16B16A16_SFLOAT;
                    pixelType = image::PixelType::RGBA_F16;
                    break;
                case HioFormatFloat32:
                    type = VK_IMAGE_TYPE_1D;
                    format = VK_FORMAT_R32_SFLOAT;
                    pixelType = image::PixelType::L_F32;
                    break;
                case HioFormatFloat32Vec2:
                    format = VK_FORMAT_R32G32_SFLOAT;
                    pixelType = image::PixelType::LA_F32;
                    break;
                case HioFormatFloat32Vec3:
                    format = VK_FORMAT_R32G32B32_SFLOAT;
                    pixelType = image::PixelType::RGB_F32;
                    break;
                case HioFormatFloat32Vec4:
                    format = VK_FORMAT_R32G32B32A32_SFLOAT;
                    pixelType = image::PixelType::RGBA_F32;
                    break;
                case HioFormatDouble64:
                case HioFormatDouble64Vec2:
                case HioFormatDouble64Vec3:
                case HioFormatDouble64Vec4:
                default:
                    std::cout << "Format: Other/Unsupported ("
                              << static_cast<int>(format)
                              << ")"
                              << std::endl;
                    break;
                }

                std::shared_ptr<image::Image> img =
                    image::Image::create(width, height, pixelType); 

                HioImage::StorageSpec spec;
                spec.width = width;
                spec.height = height;
                spec.format = hioFormat;
                spec.data = img->getData();

                if (image->Read(spec))
                {
                    std::cout << "read image" << std::endl;
                    out = vlk::Texture::create(ctx, type, width, height, depth, format);
                    out->copy(img);
                }
                else
                {
                    std::cerr << "read image failed" << std::endl;
                }
            }

            return out;
        }

    }
}
