
#define PRINT_TEXTURES 0

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
                    texture = vlk::ResolveTexture(ctx, result);
                    texture->transitionToShaderRead(cmd);
                    if (!result.texturePath.empty())
                        textureCache[result.texturePath] = texture;
                    textures[USD_DiffuseMap] = texture;
                }
                else
                {
                    textures[USD_DiffuseMap] = i->second;
                }
                
                result = usd::GetTextureOrValue(*it, TfToken("opacity"));
                i = textureCache.find(result.texturePath);
                if (i == textureCache.end())
                {
                    texture = vlk::ResolveTexture(ctx, result);
                    texture->transitionToShaderRead(cmd);
                    if (!result.texturePath.empty())
                        textureCache[result.texturePath] = texture;
                    textures[USD_OpacityMap] = texture;
                }
                else
                {
                    textures[USD_OpacityMap] = i->second;
                }
                
                result = usd::GetTextureOrValue(*it, TfToken("metallic"));
                i = textureCache.find(result.texturePath);
                if (i == textureCache.end())
                {
                    texture = vlk::ResolveTexture(ctx, result);
                    texture->transitionToShaderRead(cmd);
                    if (!result.texturePath.empty())
                        textureCache[result.texturePath] = texture;
                    textures[USD_MetallicMap] = texture;
                }
                else
                {
                    textures[USD_MetallicMap] = i->second;
                }

                result = usd::GetTextureOrValue(*it, TfToken("roughness"));
                i = textureCache.find(result.texturePath);
                if (i == textureCache.end())
                {
                    texture = vlk::ResolveTexture(ctx, result);
                    texture->transitionToShaderRead(cmd);
                    if (!result.texturePath.empty())
                        textureCache[result.texturePath] = texture;
                    textures[USD_RoughnessMap] = texture;
                }
                else
                {
                    textures[USD_RoughnessMap] = i->second;
                }

                result = usd::GetTextureOrValue(*it, TfToken("normal"));
                i = textureCache.find(result.texturePath);
                if (i == textureCache.end())
                {
                    texture = vlk::ResolveTexture(ctx, result);
                    texture->transitionToShaderRead(cmd);
                    if (!result.texturePath.empty())
                        textureCache[result.texturePath] = texture;
                    textures[USD_NormalMap] = texture;
                }
                else
                {
                    textures[USD_NormalMap] = i->second;
                }
                
                result = usd::GetTextureOrValue(*it, TfToken("occlusion"));
                i = textureCache.find(result.texturePath);
                if (i == textureCache.end())
                {
                    texture = vlk::ResolveTexture(ctx, result);
                    texture->transitionToShaderRead(cmd);
                    if (!result.texturePath.empty())
                        textureCache[result.texturePath] = texture;
                    textures[USD_AOMap] = texture;
                }
                else
                {
                    textures[USD_AOMap] = i->second;
                }
                
                result = usd::GetTextureOrValue(*it, TfToken("displacement"));
                i = textureCache.find(result.texturePath);
                if (i == textureCache.end())
                {
                    texture = vlk::ResolveTexture(ctx, result);
                    texture->transitionToShaderRead(cmd);
                    if (!result.texturePath.empty())
                        textureCache[result.texturePath] = texture;
                    textures[USD_DisplacementMap] = texture;
                }
                else
                {
                    textures[USD_DisplacementMap] = i->second;
                }

                collectedTextures[primPath] = textures;
            }
        
            std::cout << "Finished Reading Textures..." << std::endl;
        }
        
    }
}

