
#pragma once

#include <tlVk/Texture.h>

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
            ShaderInputResult     emissiveColor;
            ShaderInputResult     opacity;
            ShaderInputResult     metallic;
            ShaderInputResult     roughness;
            ShaderInputResult     normal;
            ShaderInputResult     occlusion;
            ShaderInputResult     displacement;

            float                 opacityThreshold = 0.F;
        };
    }
}
