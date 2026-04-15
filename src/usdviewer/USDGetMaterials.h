#pragma once

#include "USDRenderOptions.h"

#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdShade/material.h>

#include <tlVk/Texture.h>

#include <array>
#include <string>
#include <unordered_map>

namespace tl
{
    namespace usd
    {
        //! Inputs to a material (hold a texture or a value).
        struct ShaderInputResult
        {
            std::string           texturePath;          // non-empty when a texture is found
            std::array<float, 4>  value  = {};          // constant value when no texture
            bool                  hasValue = false;     // true when value is populated
            vlk::TextureBorder    borderU = vlk::TextureBorder::ClampToEdge;
            vlk::TextureBorder    borderV = vlk::TextureBorder::ClampToEdge;
        };

        //! Material.
        struct Material
        {
            ShaderInputResult     diffuseColor;
            ShaderInputResult     opacity;
            ShaderInputResult     metallic;
            ShaderInputResult     roughness;
            ShaderInputResult     normal;
            ShaderInputResult     occlusion;
            ShaderInputResult     displacement;
        };

        // Get a list of textures and/or values for each material in a primitive
        ShaderInputResult GetTextureOrValue(
            const pxr::UsdPrim& prim,
            const pxr::UsdShadeMaterial& material,
            const pxr::TfToken& inputName,
            const bool          debug = DEBUG_TEXTURES);
        
        std::unordered_map<std::string, Material>
        GetMaterials(const pxr::UsdPrim& prim,
                     const bool          debug = DEBUG_MATERIALS);
    }
}
