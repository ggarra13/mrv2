
#include "USDCollectTextures.h"
#include "USDGetMaterials.h"
#include "USDResolveTexture.h"
#include "USDTextureSlots.h"

#include <tlVk/Texture.h>

#include <tlCore/Image.h>

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
                             const std::string& materialName,
                             const usd::Material& material,
                             std::unordered_map<std::string, std::shared_ptr<vlk::Texture > >&
                             textureCache,
                             std::unordered_map<std::string,
                             ShaderTextures >& collectedTextures)
        {        
            std::cout << "Started Reading textures for material " << materialName << "..." << std::endl;
                   
            std::unordered_map<int, std::shared_ptr<vlk::Texture > > textures;
            std::shared_ptr<vlk::Texture > texture;

            {
                const usd::ShaderInputResult& slot = material.diffuseColor;
                std::string texturePath = slot.texturePath;
                auto i = textureCache.find(texturePath);
                if (i == textureCache.end())
                {
                    texture = vlk::ResolveTexture(ctx, slot);
                    if (!slot.texturePath.empty())
                        textureCache[texturePath] = texture;
                    textures[USD_DiffuseMap] = texture;
                }
                else
                {
                    textures[USD_DiffuseMap] = i->second;
                }
            }

            {
                const usd::ShaderInputResult& slot = material.opacity;
                std::string texturePath = slot.texturePath;
                auto i = textureCache.find(texturePath);
                i = textureCache.find(texturePath);
                if (i == textureCache.end())
                {
                    texture = vlk::ResolveTexture(ctx, slot);
                    if (!texturePath.empty())
                        textureCache[texturePath] = texture;
                    textures[USD_OpacityMap] = texture;
                }
                else
                {
                    textures[USD_OpacityMap] = i->second;
                }
            }
                    
            {       
                const usd::ShaderInputResult& slot = material.metallic;
                std::string texturePath = slot.texturePath;
                auto i = textureCache.find(texturePath);
                if (i == textureCache.end())
                {
                    texture = vlk::ResolveTexture(ctx, slot);
                    if (!texturePath.empty())
                        textureCache[texturePath] = texture;
                    textures[USD_MetallicMap] = texture;
                }
                else
                {
                    textures[USD_MetallicMap] = i->second;
                }
            }

            {
                const usd::ShaderInputResult& slot = material.roughness;
                std::string texturePath = slot.texturePath;
                auto i = textureCache.find(texturePath);
                if (i == textureCache.end())
                {
                    texture = vlk::ResolveTexture(ctx, slot);
                    if (!texturePath.empty())
                        textureCache[texturePath] = texture;
                    textures[USD_RoughnessMap] = texture;
                }
                else
                {
                    textures[USD_RoughnessMap] = i->second;
                }
            }

            {
                const usd::ShaderInputResult& slot = material.normal;
                std::string texturePath = slot.texturePath;
                auto i = textureCache.find(texturePath);
                if (i == textureCache.end())
                {
                    texture = vlk::ResolveTexture(ctx, slot);
                    if (!texturePath.empty())
                        textureCache[texturePath] = texture;
                    textures[USD_NormalMap] = texture;
                }
                else
                {
                    textures[USD_NormalMap] = i->second;
                }
            }

            {
                const usd::ShaderInputResult& slot = material.occlusion;
                std::string texturePath = slot.texturePath;
                auto i = textureCache.find(texturePath);
                if (i == textureCache.end())
                {
                    texture = vlk::ResolveTexture(ctx, slot);
                    if (!texturePath.empty())
                        textureCache[texturePath] = texture;
                    textures[USD_OcclusionMap] = texture;
                }
                else
                {
                    textures[USD_OcclusionMap] = i->second;
                }
            }

            {
                const usd::ShaderInputResult& slot = material.emissiveColor;
                std::string texturePath = slot.texturePath;
                auto i = textureCache.find(texturePath);
                if (i == textureCache.end())
                {
                    texture = vlk::ResolveTexture(ctx, slot);
                    if (!texturePath.empty())
                        textureCache[texturePath] = texture;
                    textures[USD_EmissiveMap] = texture;
                }
                else
                {
                    textures[USD_EmissiveMap] = i->second;
                }
            }
            
            {
                const usd::ShaderInputResult& slot = material.displacement;
                std::string texturePath = slot.texturePath;
                auto i = textureCache.find(texturePath);
                if (i == textureCache.end())
                {
                    texture = vlk::ResolveTexture(ctx, slot);
                    if (!texturePath.empty())
                        textureCache[texturePath] = texture;
                    textures[USD_DisplacementMap] = texture;
                }
                else
                {
                    textures[USD_DisplacementMap] = i->second;
                }
            }
                    
            collectedTextures[materialName] = textures;
            
            std::cout << "Finished Reading Textures for " << materialName << "." << std::endl;
        }
                        
    }
}

