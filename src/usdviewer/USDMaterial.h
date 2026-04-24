
#pragma once

#include <tlVk/Texture.h>

#include <pxr/base/tf/token.h>

#include <ostream>

namespace tl
{
    namespace usd
    {
        //! Inputs to a material (hold a texture or a value).
        struct ShaderInputResult
        {
            std::string           texturePath;          // non-empty when a texture is found
            std::array<float, 4>  value  = {1.F, 1.F, 1.F, 1.F};          // constant value when no texture
            bool                  hasValue = false;     // true when value is populated
            vlk::TextureBorder    borderU = vlk::TextureBorder::ClampToEdge;
            vlk::TextureBorder    borderV = vlk::TextureBorder::ClampToEdge;
            std::string           channel = "rgba";
            std::string        colorSpace = "srgb";
            pxr::TfToken          primvar = pxr::TfToken("st");
            bool                 animated = false;
            
            std::string getConnection() const
                {
                    if (texturePath.empty())
                        return texturePath;
                    else
                        return texturePath + ":" + channel;
                }
            
            bool operator==(const ShaderInputResult& b) const
                {
                    return (animated == b.animated &&
                            hasValue == b.hasValue &&
                            value == b.value &&
                            texturePath == b.texturePath &&
                            channel == b.channel &&
                            borderU == b.borderU &&
                            borderV == b.borderV &&
                            colorSpace == b.colorSpace);
                }
            
            bool operator!=(const ShaderInputResult& b) const
                {
                    return !(*this == b);
                }
        };
        

        inline
        std::ostream& operator<<(std::ostream& o, const ShaderInputResult& b)
        {
            if (!b.texturePath.empty() && b.texturePath[0] != '*')
                return o << b.texturePath << " primvar=" << b.primvar.GetString() << " channel=" << b.channel
                         << " borderU=" << b.borderU << " borderV=" << b.borderV;
            else
                return o << " hasValue=" << b.hasValue << " " << b.value[0] << ", "
                         << b.value[1] << ", " << b.value[2] << " " << b.value[3];
        }

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
            ShaderInputResult     ior;
            ShaderInputResult     opacityThreshold;

            bool                  transparent = false;
            
            bool operator==(const Material& b) const
                {
                    return (
                        diffuseColor == b.diffuseColor &&
                        metallic == b.metallic &&
                        roughness == b.roughness &&
                        occlusion == b.occlusion &&
                        opacity == b.opacity &&
                        normal == b.normal &&
                        transparent == b.transparent &&
                        emissiveColor == b.emissiveColor &&
                        opacityThreshold == b.opacityThreshold &&
                        ior == b.ior);
                }
            
            bool operator!=(const Material& b) const
                {
                    return !(*this == b);
                }
        };
    }
}
