
#include <pxr/imaging/hio/image.h>

#include "USDResolveTexture.h"

#include <tlVk/Texture.h>
#include <tlCore/Image.h>

#include <cmath>
#include <vector>

namespace tl
{
    namespace vlk
    {
        using namespace tl::image;

        namespace {
            // Template helper to process the raw buffer
            template <typename T>
            void processFill(uint8_t* data, size_t pixelCount, int channelCount, int sourceIndex) {
                T* p = reinterpret_cast<T*>(data);
                for (size_t i = 0; i < pixelCount; ++i, p += channelCount) {
                    // Grab the value of the source channel (e.g., R)
                    const T value = p[sourceIndex];
                    // Fill all other channels in this pixel with that value
                    for (int c = 0; c < channelCount; ++c) {
                        p[c] = value;
                    }
                }
            }
        }

        void fillChannelsFromSource(std::shared_ptr<Image>& image, int sourceChannelIndex) {
            const auto& info = image->getInfo();
            const int channelCount = getChannelCount(info.pixelType);
    
            // If it's a single channel image, there's nothing to fill
            if (channelCount <= 1 || sourceChannelIndex >= channelCount) return;

            const size_t pixelCount = info.size.w * info.size.h;
            uint8_t* data = image->getData();

            // Determine bit depth/type and process
            switch (info.pixelType) {
            case PixelType::L_U8:
            case PixelType::LA_U8:
            case PixelType::RGB_U8:
            case PixelType::RGBA_U8:
                processFill<uint8_t>(data, pixelCount, channelCount, sourceChannelIndex);
                break;

            case PixelType::L_U16:
            case PixelType::LA_U16:
            case PixelType::RGB_U16:
            case PixelType::RGBA_U16:
                processFill<uint16_t>(data, pixelCount, channelCount, sourceChannelIndex);
                break;

            case PixelType::L_F16:
            case PixelType::LA_F16:
            case PixelType::RGB_F16:
            case PixelType::RGBA_F16:
                processFill<uint16_t>(data, pixelCount, channelCount, sourceChannelIndex);
                break;

            case PixelType::L_F32:
            case PixelType::LA_F32:
            case PixelType::RGB_F32:
            case PixelType::RGBA_F32:
                processFill<float>(data, pixelCount, channelCount, sourceChannelIndex);
                break;

            default:
                // Handle specialized or None types if necessary
                break;
            }
        }

        // void ProcessSRGBToLinear(unsigned char* data, int width, int height, int numChannels) {
        //     int totalPixels = width * height * numChannels;
        //     int numChannelsNoAlpha = (numChannels == 4 ? 3 : numChannels == 2 ? 1 : numChannels);
    
        //     for (int i = 0; i < totalPixels; i += numChannels) {
        //         // We only process RGB; Alpha is usually already linear
        //         for (int j = 0; j < numChannelsNoAlpha; ++j) {
        //             // 1. Normalize to 0.0 - 1.0
        //             float normalized = data[i + j] / 255.0f;
            
        //             // 2. Apply Gamma (Approximation of 2.2)
        //             float linearized = std::pow(normalized, 2.2f);
            
        //             // 3. Scale back to 0-255 (if staying in 8-bit) 
        //             // Note: Doing math in 8-bit causes heavy precision loss.
        //             data[i + j] = static_cast<unsigned char>(std::round(linearized * 255.0f));
        //         }
        //     }
        // }
        
        std::shared_ptr<vlk::Texture> ResolveTexture(
            Fl_Vk_Context& ctx,
            const usd::ShaderInputResult& result)
        {
            using namespace PXR_NS;

            std::shared_ptr<vlk::Texture> out;

            if (!result.texturePath.empty() && result.texturePath[0] != '*')
            {
                // Hio::Image opens the file (or the file inside the .usdz) and reads the header
                HioImageSharedPtr image = HioImage::OpenForReading(result.texturePath);
                
                if (image)
                {
                    int width  = image->GetWidth();
                    int height = image->GetHeight();
                    HioFormat hioFormat = image->GetFormat();
                    image::PixelType pixelType;
                
                    // Map HioFormat to bit depth
                    switch (hioFormat) {
                    case HioFormatUNorm8srgb:
                        pixelType = image::PixelType::L_U8;
                        break;
                    case HioFormatUNorm8Vec2srgb:
                        pixelType = image::PixelType::LA_U8;
                        break;
                    case HioFormatUNorm8Vec3srgb:
                        pixelType = image::PixelType::RGB_U8;
                        break;
                    case HioFormatUNorm8Vec4srgb:
                        pixelType = image::PixelType::RGBA_U8;
                        break;
                    case HioFormatUNorm8:
                        pixelType = image::PixelType::L_U8;
                        break;
                    case HioFormatUNorm8Vec2:
                        pixelType = image::PixelType::LA_U8;
                        break;
                    case HioFormatUNorm8Vec3:
                        pixelType = image::PixelType::RGB_U8;
                        break;
                    case HioFormatUNorm8Vec4:
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
                        if (image->IsColorSpaceSRGB())
                        {
                            uint8_t* data = img->getData();
                            int numChannels = image::getChannelCount(img->getPixelType());
                        
                            // ProcessSRGBToLinear(data, width, height, numChannels);
                        }
                    
                        TextureOptions options;
                        options.filters.minify = timeline::ImageFilter::Linear;
                        options.filters.magnify = timeline::ImageFilter::Linear;
                        options.borders.U = result.borderU;
                        options.borders.V = result.borderV;

                        // Check the channel of the connection and if single channel,
                        // copy its contents to the full RGB, RGBA or LA texture.
                        if (result.channel == "r")
                            fillChannelsFromSource(img, 0);
                        else if (result.channel == "g")
                            fillChannelsFromSource(img, 1);
                        else if (result.channel == "b")
                            fillChannelsFromSource(img, 2);
                        else if (result.channel == "a")
                            fillChannelsFromSource(img, 3);
                        
                        out = vlk::Texture::create(ctx, info, options);
                        out->copy(img);
                    }
                }
            }
            else
            {
                // Create a 1x1 RGBA texture with the values of result.
                const image::Info info(1, 1, image::PixelType::RGBA_F32);
                std::shared_ptr<image::Image> img = image::Image::create(info); 
                std::memcpy(img->getData(), &result.value[0], sizeof(float)*4);
                
                TextureOptions options;
                options.filters.minify = timeline::ImageFilter::Nearest;
                options.filters.magnify = timeline::ImageFilter::Nearest;
                        
                out = vlk::Texture::create(ctx, info, options);
                out->copy(img);
            }

            return out;
        }
    }
}
