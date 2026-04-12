
#define PRINT_TEXTURES 1

#include "USDGetTextureOrValue.h"
#include "USDResolveTexture.h"

#include <tlTimelineVk/USDTextureSlots.h>

#include <tlVk/Texture.h>

#include <tlCore/Image.h>
#include "USDCollectTextures.h"

#include <pxr/usd/usd/timeCode.h>
#include <pxr/usd/usd/primRange.h>
#include <pxr/usd/usdGeom/imageable.h>

#include <FL/Fl_Vk_Context.H>

#include <iostream>

namespace tl
{
    namespace usd
    {
        using namespace PXR_NS;

        void CollectTextures(Fl_Vk_Context& ctx,
                             const VkCommandBuffer cmd,
                             const UsdStageRefPtr stage,
                             const UsdTimeCode time,
                             std::unordered_map<std::string,
                             ShaderTextures >& collectedTextures)
        {
            // Create an empty texture (L_U8 is okay)
            image::Info info(1, 1, image::PixelType::L_U8);
            std::shared_ptr<image::Image> img = image::Image::create(info);
            memset(img->getData(), 0, img->getDataByteCount());
        
            auto emptyTexture = vlk::Texture::create(ctx, info);
            emptyTexture->copy(img);
            emptyTexture->transitionToShaderRead(cmd);

            // Create a gray texture
            info = image::Info(1, 1, image::PixelType::RGBA_U8);
            memset(img->getData(), 128, img->getDataByteCount());
        
            auto middleTexture = vlk::Texture::create(ctx, info);
            middleTexture->copy(img);
            middleTexture->transitionToShaderRead(cmd);
        
            // Create a filled (RGBA) texture
            img = image::Image::create(info);
            memset(img->getData(), 255, img->getDataByteCount());
        
            auto filledTexture = vlk::Texture::create(ctx, info);
            filledTexture->copy(img);
            filledTexture->transitionToShaderRead(cmd);
        
            // Collect textures.
            UsdPrimRange range(stage->GetPseudoRoot(),
                               UsdTraverseInstanceProxies());

            std::cout << "Started Reading textures..." << std::endl;
            std::unordered_map<std::string, std::shared_ptr<vlk::Texture > >
                textureCache;
        
            for (auto it = range.begin(); it != range.end(); ++it) {

                //
                // Ignore hidden geometry
                //
                if (!it->IsA<UsdGeomImageable>())
                    continue;
                
                UsdGeomImageable imageable(*it);
            
                if (imageable.ComputeVisibility(time) ==
                    UsdGeomTokens->invisible) {
                    // If this prim is invisible, its entire subtree is invisible.
                    // Prune the traversal to skip all children.
                    it.PruneChildren();
                    continue;
                }

                // If purpose is not default or not render, don't use this
                // geometry.
                TfToken purpose = imageable.ComputePurpose();
                if (purpose != UsdGeomTokens->default_ &&
                    purpose != UsdGeomTokens->render)
                    continue;
                
                
                const std::string primPath = it->GetPath().GetString();
#if PRINT_TEXTURES
                std::cout << primPath << std::endl;
#endif             
                std::unordered_map<int, std::shared_ptr<vlk::Texture > > textures;
                std::shared_ptr<vlk::Texture > texture;
                usd::ShaderInputResult result;

                result = usd::GetTextureOrValue(*it, TfToken("diffuseColor"));
                
                auto i = textureCache.find(result.texturePath);
                if (i == textureCache.end())
                {
                    std::cerr << "\tdiffuse=" << result.texturePath << std::endl;
                    texture = vlk::ResolveTexture(ctx, result);
                    texture->transitionToShaderRead(cmd);
                    if (!result.texturePath.empty())
                        textureCache[result.texturePath] = texture;
                    textures[USD_DiffuseMap] = texture;
                }
                else
                {
                    std::cerr << "\tREUSING diffuse=" << result.texturePath << std::endl;
                    textures[USD_DiffuseMap] = i->second;
                }
                
                result = usd::GetTextureOrValue(*it, TfToken("opacity"));
                i = textureCache.find(result.texturePath);
                if (i == textureCache.end())
                {
                    std::cerr << "\topacity=" << result.texturePath << std::endl;
                    texture = vlk::ResolveTexture(ctx, result);
                    texture->transitionToShaderRead(cmd);
                    if (!result.texturePath.empty())
                        textureCache[result.texturePath] = texture;
                    textures[USD_OpacityMap] = texture;
                }
                else
                {
                    std::cerr << "\tREUSING opacity=" << result.texturePath << std::endl;
                    textures[USD_OpacityMap] = i->second;
                }
                
                result = usd::GetTextureOrValue(*it, TfToken("metallic"));
                i = textureCache.find(result.texturePath);
                if (i == textureCache.end())
                {
                    std::cerr << "\tmetallic=" << result.texturePath << std::endl;
                    texture = vlk::ResolveTexture(ctx, result);
                    texture->transitionToShaderRead(cmd);
                    if (!result.texturePath.empty())
                        textureCache[result.texturePath] = texture;
                    textures[USD_MetallicMap] = texture;
                }
                else
                {
                    std::cerr << "\tREUSING metallic=" << result.texturePath << std::endl;
                    textures[USD_MetallicMap] = i->second;
                }

                result = usd::GetTextureOrValue(*it, TfToken("roughness"));
                i = textureCache.find(result.texturePath);
                if (i == textureCache.end())
                {
                    std::cerr << "\troughness=" << result.texturePath << std::endl;
                    texture = vlk::ResolveTexture(ctx, result);
                    texture->transitionToShaderRead(cmd);
                    if (!result.texturePath.empty())
                        textureCache[result.texturePath] = texture;
                    textures[USD_RoughnessMap] = texture;
                }
                else
                {
                    std::cerr << "\tREUSING roughness=" << result.texturePath << std::endl;
                    textures[USD_RoughnessMap] = i->second;
                }

                result = usd::GetTextureOrValue(*it, TfToken("normal"));
                i = textureCache.find(result.texturePath);
                if (i == textureCache.end())
                {
                    std::cerr << "\tnormal=" << result.texturePath << std::endl;
                    texture = vlk::ResolveTexture(ctx, result);
                    texture->transitionToShaderRead(cmd);
                    if (!result.texturePath.empty())
                        textureCache[result.texturePath] = texture;
                    textures[USD_NormalMap] = texture;
                }
                else
                {
                    std::cerr << "\tREUSING normal=" << result.texturePath << std::endl;
                    textures[USD_NormalMap] = i->second;
                }
                
                result = usd::GetTextureOrValue(*it, TfToken("occlusion"));
                i = textureCache.find(result.texturePath);
                if (i == textureCache.end())
                {
                    std::cerr << "\tocclusion=" << result.texturePath << std::endl;
                    texture = vlk::ResolveTexture(ctx, result);
                    texture->transitionToShaderRead(cmd);
                    if (!result.texturePath.empty())
                        textureCache[result.texturePath] = texture;
                    textures[USD_AOMap] = texture;
                }
                else
                {
                    std::cerr << "\tREUSING occlusion=" << result.texturePath << std::endl;
                    textures[USD_AOMap] = i->second;
                }
                
                result = usd::GetTextureOrValue(*it, TfToken("displacement"));
                i = textureCache.find(result.texturePath);
                if (i == textureCache.end())
                {
                    std::cerr << "\tdisplacement=" << result.texturePath << std::endl;
                    texture = vlk::ResolveTexture(ctx, result);
                    texture->transitionToShaderRead(cmd);
                    if (!result.texturePath.empty())
                        textureCache[result.texturePath] = texture;
                    textures[USD_DisplacementMap] = texture;
                }
                else
                {
                    std::cerr << "\tREUSING displacement=" << result.texturePath << std::endl;
                    textures[USD_DisplacementMap] = i->second;
                }

                collectedTextures[primPath] = textures;
            }
        
#if PRINT_TEXTURES
            std::cout << "Finished Reading Textures..." << std::endl;
#endif
        }
        
    }
}

