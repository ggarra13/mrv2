
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
        
#define GET_SLOT_TEXTURE(INPUT, MAPNAME)                        \
        {                                                       \
        const usd::ShaderInputResult  slot = material.INPUT;    \
        connection = slot.getConnection();                      \
        if (connection.empty())                                 \
        {                                                       \
            texture = vlk::ResolveTexture(ctx, slot);           \
            textures[MAPNAME] = texture;                        \
        }                                                       \
        else                                                    \
        {                                                       \
            auto i = textureCache.find(connection);             \
            if (i == textureCache.end())                        \
            {                                                   \
                texture = vlk::ResolveTexture(ctx, slot);       \
                textureCache[connection] = texture;             \
                if (!slot.hasValue)                             \
                    ++numTextures;                              \
                textures[MAPNAME] = texture;                    \
            }                                                   \
            else                                                \
            {                                                   \
                textures[MAPNAME] = i->second;                  \
            }                                                   \
        }                                                       \
    }                                                           \
    

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
#if DEBUG_TEXTURES
                std::cout << "\tReading textures for " << materialName << std::endl;
#endif
                std::unordered_map<int, std::shared_ptr<vlk::Texture > > textures;
                
                GET_SLOT_TEXTURE(diffuseColor, USD_DiffuseMap);
                GET_SLOT_TEXTURE(opacity, USD_OpacityMap);
                GET_SLOT_TEXTURE(metallic, USD_MetallicMap);
                GET_SLOT_TEXTURE(roughness, USD_RoughnessMap);
                GET_SLOT_TEXTURE(normal, USD_NormalMap);
                GET_SLOT_TEXTURE(occlusion, USD_OcclusionMap);
                GET_SLOT_TEXTURE(emissiveColor, USD_EmissiveMap)
                GET_SLOT_TEXTURE(displacement, USD_DisplacementMap);
                GET_SLOT_TEXTURE(opacityThreshold, USD_OpacityThresholdMap);
                GET_SLOT_TEXTURE(ior, USD_IorMap);

                collectedTextures[materialName] = textures;
            }
            
            std::cout << "Finished Reading " << numTextures << " Textures." << std::endl;

            
        }
                        
    }
}

