
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
                HioFormat hioFormat = image->GetFormat();
                image::PixelType pixelType;
                
                // Map HioFormat to bit depth
                switch (hioFormat) {
                case HioFormatUNorm8:
                case HioFormatUNorm8srgb:
                    pixelType = image::PixelType::L_U8;
                    break;
                case HioFormatUNorm8Vec2:
                case HioFormatUNorm8Vec2srgb:
                    pixelType = image::PixelType::LA_U8;
                    break;
                case HioFormatUNorm8Vec3:
                case HioFormatUNorm8Vec3srgb:
                    pixelType = image::PixelType::RGB_U8;
                    break;
                case HioFormatUNorm8Vec4:
                case HioFormatUNorm8Vec4srgb:
                    pixelType = image::PixelType::RGBA_U8;
                    break;
                case HioFormatSNorm8:
                    pixelType = image::PixelType::L_U8;
                    break;
                case HioFormatSNorm8Vec2:
                    pixelType = image::PixelType::LA_U8;
                    break;
                case HioFormatSNorm8Vec3:
                    pixelType = image::PixelType::RGB_U8;
                    break;
                case HioFormatSNorm8Vec4:
                    pixelType = image::PixelType::RGBA_U8;
                    break;
                case HioFormatUInt16:
                    pixelType = image::PixelType::L_U16;
                    break;
                case HioFormatUInt16Vec2:
                    pixelType = image::PixelType::LA_U16;
                    break;
                case HioFormatUInt16Vec3:
                    pixelType = image::PixelType::RGB_U16;
                    break;
                case HioFormatUInt16Vec4:
                    pixelType = image::PixelType::RGBA_U16;
                    break;
                case HioFormatInt16:
                    pixelType = image::PixelType::L_U16;
                    break;
                case HioFormatInt16Vec2:
                    pixelType = image::PixelType::LA_U16;
                    break;
                case HioFormatInt16Vec3:
                    pixelType = image::PixelType::RGB_U16;
                    break;
                case HioFormatInt16Vec4:
                    pixelType = image::PixelType::RGBA_U16;
                    break;
                case HioFormatUInt32:
                    pixelType = image::PixelType::L_U32;
                    break;
                case HioFormatUInt32Vec2:
                    pixelType = image::PixelType::LA_U32;
                    break;
                case HioFormatUInt32Vec3:
                    pixelType = image::PixelType::RGB_U32;
                    break;
                case HioFormatUInt32Vec4:
                    pixelType = image::PixelType::RGBA_U32;
                    break;
                case HioFormatInt32:
                    pixelType = image::PixelType::L_U32;
                    break;
                case HioFormatInt32Vec2:
                    pixelType = image::PixelType::LA_U32;
                    break;
                case HioFormatInt32Vec3:
                    pixelType = image::PixelType::RGB_U32;
                    break;
                case HioFormatInt32Vec4:
                    pixelType = image::PixelType::RGBA_U32;
                    break;
                case HioFormatFloat16:
                    pixelType = image::PixelType::L_F16;
                    break;
                case HioFormatFloat16Vec2:
                    pixelType = image::PixelType::LA_F16;
                    break;
                case HioFormatFloat16Vec3:
                    pixelType = image::PixelType::RGB_F16;
                    break;
                case HioFormatFloat16Vec4:
                    pixelType = image::PixelType::RGBA_F16;
                    break;
                case HioFormatFloat32:
                    pixelType = image::PixelType::L_F32;
                    break;
                case HioFormatFloat32Vec2:
                    pixelType = image::PixelType::LA_F32;
                    break;
                case HioFormatFloat32Vec3:
                    pixelType = image::PixelType::RGB_F32;
                    break;
                case HioFormatFloat32Vec4:
                    pixelType = image::PixelType::RGBA_F32;
                    break;
                case HioFormatDouble64:
                case HioFormatDouble64Vec2:
                case HioFormatDouble64Vec3:
                case HioFormatDouble64Vec4:
                default:
                    std::cerr << "Format: Other/Unsupported ("
                              << static_cast<int>(hioFormat)
                              << ")"
                              << std::endl;
                    return out;
                }

                const image::Info info(width, height, pixelType);
                std::shared_ptr<image::Image> img = image::Image::create(info); 

                HioImage::StorageSpec spec;
                spec.width = width;
                spec.height = height;
                spec.format = hioFormat;
                spec.data = img->getData();

                if (image->Read(spec))
                {
                    out = vlk::Texture::create(ctx, info);
                    out->copy(img);
                }
            }

            return out;
        }

    }
}
