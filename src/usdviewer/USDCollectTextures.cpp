
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
                             const std::unordered_map<std::string, Material >& materials,
                             std::unordered_map<std::string, std::shared_ptr<vlk::Texture > >&
                             textureCache,
                             std::unordered_map<std::string,
                             ShaderTextures >& collectedTextures)
        {
            
            std::cout << "Started Reading textures..." << std::endl;
            std::shared_ptr<vlk::Texture > texture;
            std::string connection;
            std::size_t numTextures = 0;
            for (const auto& [materialName, material] : materials)
            {
                   
                std::unordered_map<int, std::shared_ptr<vlk::Texture > > textures;
                {
                    const usd::ShaderInputResult& slot = material.diffuseColor;
                    connection = slot.getConnection();
                    auto i = textureCache.find(connection);
                    if (i == textureCache.end())
                    {
                        texture = vlk::ResolveTexture(ctx, slot);
                        if (!connection.empty())
                        {
                            textureCache[connection] = texture;
                            if (!slot.hasValue)
                                ++numTextures;
                        }
                        textures[USD_DiffuseMap] = texture;
                    }
                    else
                    {
                        textures[USD_DiffuseMap] = i->second;
                    }
                }

                {
                    const usd::ShaderInputResult& slot = material.opacity;
                    connection = slot.getConnection();
                    auto i = textureCache.find(connection);
                    i = textureCache.find(connection);
                    if (i == textureCache.end())
                    {
                        texture = vlk::ResolveTexture(ctx, slot);
                        if (!connection.empty())
                        {
                            textureCache[connection] = texture;
                            if (!slot.hasValue)
                                ++numTextures;
                        }
                        textures[USD_OpacityMap] = texture;
                    }
                    else
                    {
                        textures[USD_OpacityMap] = i->second;
                    }
                }
                    
                {       
                    const usd::ShaderInputResult& slot = material.metallic;
                    connection = slot.getConnection();
                    auto i = textureCache.find(connection);
                    if (i == textureCache.end())
                    {
                        texture = vlk::ResolveTexture(ctx, slot);
                        if (!connection.empty())
                        {
                            textureCache[connection] = texture;
                            if (!slot.hasValue)
                                ++numTextures;
                        }
                        textures[USD_MetallicMap] = texture;
                    }
                    else
                    {
                        textures[USD_MetallicMap] = i->second;
                    }
                }

                {
                    const usd::ShaderInputResult& slot = material.roughness;
                    connection = slot.getConnection();
                    auto i = textureCache.find(connection);
                    if (i == textureCache.end())
                    {
                        texture = vlk::ResolveTexture(ctx, slot);
                        if (!connection.empty())
                        {
                            textureCache[connection] = texture;
                            if (!slot.hasValue)
                                ++numTextures;
                        }
                        textures[USD_RoughnessMap] = texture;
                    }
                    else
                    {
                        textures[USD_RoughnessMap] = i->second;
                    }
                }

                {
                    const usd::ShaderInputResult& slot = material.normal;
                    connection = slot.getConnection();
                    auto i = textureCache.find(connection);
                    if (i == textureCache.end())
                    {
                        texture = vlk::ResolveTexture(ctx, slot);
                        if (!connection.empty())
                        {
                            textureCache[connection] = texture;
                            if (!slot.hasValue)
                                ++numTextures;
                        }
                        textures[USD_NormalMap] = texture;
                    }
                    else
                    {
                        textures[USD_NormalMap] = i->second;
                    }
                }

                {
                    const usd::ShaderInputResult& slot = material.occlusion;
                    connection = slot.getConnection();
                    auto i = textureCache.find(connection);
                    if (i == textureCache.end())
                    {
                        texture = vlk::ResolveTexture(ctx, slot);
                        if (!connection.empty())
                        {
                            textureCache[connection] = texture;
                            if (!slot.hasValue)
                                ++numTextures;
                        }
                        textures[USD_OcclusionMap] = texture;
                    }
                    else
                    {
                        textures[USD_OcclusionMap] = i->second;
                    }
                }

                {
                    const usd::ShaderInputResult& slot = material.emissiveColor;
                    connection = slot.getConnection();
                    auto i = textureCache.find(connection);
                    if (i == textureCache.end())
                    {
                        texture = vlk::ResolveTexture(ctx, slot);
                        if (!connection.empty())
                        {
                            textureCache[connection] = texture;
                            if (!slot.hasValue)
                                ++numTextures;
                        }
                        textures[USD_EmissiveMap] = texture;
                    }
                    else
                    {
                        textures[USD_EmissiveMap] = i->second;
                    }
                }
            
                {
                    const usd::ShaderInputResult& slot = material.displacement;
                    connection = slot.getConnection();
                    auto i = textureCache.find(connection);
                    if (i == textureCache.end())
                    {
                        texture = vlk::ResolveTexture(ctx, slot);
                        if (!connection.empty())
                        {
                            textureCache[connection] = texture;
                            if (!slot.hasValue)
                                ++numTextures;
                        }
                        textures[USD_DisplacementMap] = texture;
                    }
                    else
                    {
                        textures[USD_DisplacementMap] = i->second;
                    }
                }
                    
                collectedTextures[materialName] = textures;
            }
            
            std::cout << "Finished Reading " << numTextures << " Textures." << std::endl;

            
        }
                        
    }
}

