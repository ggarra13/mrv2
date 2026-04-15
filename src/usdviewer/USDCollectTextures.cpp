
#include "USDGetMaterials.h"
#include "USDResolveTexture.h"
#include "USDTextureSlots.h"

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
                
                 std::unordered_map<std::string, usd::Material> materials;               
                std::unordered_map<int, std::shared_ptr<vlk::Texture > > textures;
                std::shared_ptr<vlk::Texture > texture;
                
                materials = usd::GetMaterials(*it);
                for (auto& [name, material] : materials)
                {
                    usd::ShaderInputResult& slot = material.diffuseColor;
                    std::string texturePath = slot.texturePath;
                    auto i = textureCache.find(texturePath);
                    if (i == textureCache.end())
                    {
                        texture = vlk::ResolveTexture(ctx, slot);
                        texture->transitionToShaderRead(cmd);
                        if (!slot.texturePath.empty())
                            textureCache[texturePath] = texture;
                        textures[USD_DiffuseMap] = texture;
                    }
                    else
                    {
                        textures[USD_DiffuseMap] = i->second;
                    }
                    
                    slot = material.opacity;
                    texturePath = slot.texturePath;
                    i = textureCache.find(texturePath);
                    if (i == textureCache.end())
                    {
                        texture = vlk::ResolveTexture(ctx, slot);
                        texture->transitionToShaderRead(cmd);
                        if (!texturePath.empty())
                            textureCache[texturePath] = texture;
                        textures[USD_OpacityMap] = texture;
                    }
                    else
                    {
                        textures[USD_OpacityMap] = i->second;
                    }
                    
                    
                    slot = material.metallic;
                    texturePath = slot.texturePath;
                    i = textureCache.find(texturePath);
                    if (i == textureCache.end())
                    {
                        texture = vlk::ResolveTexture(ctx, slot);
                        texture->transitionToShaderRead(cmd);
                        if (!texturePath.empty())
                            textureCache[texturePath] = texture;
                        textures[USD_MetallicMap] = texture;
                    }
                    else
                    {
                        textures[USD_MetallicMap] = i->second;
                    }
                    
                    slot = material.roughness;
                    texturePath = slot.texturePath;
                    i = textureCache.find(texturePath);
                    if (i == textureCache.end())
                    {
                        texture = vlk::ResolveTexture(ctx, slot);
                        texture->transitionToShaderRead(cmd);
                        if (!texturePath.empty())
                            textureCache[texturePath] = texture;
                        textures[USD_RoughnessMap] = texture;
                    }
                    else
                    {
                        textures[USD_RoughnessMap] = i->second;
                    }
                    
                    slot = material.normal;
                    texturePath = slot.texturePath;
                    i = textureCache.find(texturePath);
                    if (i == textureCache.end())
                    {
                        texture = vlk::ResolveTexture(ctx, slot);
                        texture->transitionToShaderRead(cmd);
                        if (!texturePath.empty())
                            textureCache[texturePath] = texture;
                        textures[USD_NormalMap] = texture;
                    }
                    else
                    {
                        textures[USD_NormalMap] = i->second;
                    }
                    
                    slot = material.occlusion;
                    texturePath = slot.texturePath;
                    i = textureCache.find(texturePath);
                    if (i == textureCache.end())
                    {
                        texture = vlk::ResolveTexture(ctx, slot);
                        texture->transitionToShaderRead(cmd);
                        if (!texturePath.empty())
                            textureCache[texturePath] = texture;
                        textures[USD_OcclusionMap] = texture;
                    }
                    else
                    {
                        textures[USD_OcclusionMap] = i->second;
                    }
                    
                    slot = material.displacement;
                    texturePath = slot.texturePath;
                    i = textureCache.find(texturePath);
                    if (i == textureCache.end())
                    {
                        texture = vlk::ResolveTexture(ctx, slot);
                        texture->transitionToShaderRead(cmd);
                        if (!texturePath.empty())
                            textureCache[texturePath] = texture;
                        textures[USD_DisplacementMap] = texture;
                    }
                    else
                    {
                        textures[USD_DisplacementMap] = i->second;
                    }
                    
                    collectedTextures[name] = textures;
                }
                

            }
        
            std::cout << "Finished Reading Textures..." << std::endl;
        }
        
    }
}

