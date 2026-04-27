
#include <pxr/usd/usdShade/materialBindingAPI.h>
#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/usdShade/tokens.h>

namespace tl
{
    namespace usd
    {
        inline pxr::UsdShadeMaterial GetMaterial(pxr::UsdShadeMaterialBindingAPI& api)
        {
            pxr::UsdShadeMaterial out;

            // If we don't specify a purpose, we get a barebone scenes with almost no textures (ALab scene for example)
            // In preview mode, the ALab scene has all sort of UV texturing
            // issues.
            for (auto purpose : {
                    pxr::UsdShadeTokens->preview,  // preview returns all preview textures for Storm (ALab has 974 textures)
                    pxr::UsdShadeTokens->full,  // (ALab 29 textures on my small scene as I have not downloaded the high-res texture pack)
                }
                )
            {
                out = api.ComputeBoundMaterial(purpose);
                if (out)
                    break;
            }
            return out;
        }
    }
}
