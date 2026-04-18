
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
#if 1
            for (auto purpose : {
                    pxr::UsdShadeTokens->full,
                    pxr::UsdShadeTokens->preview })
            {
                out = api.ComputeBoundMaterial(purpose);
                if (out)
                    break;
            }
#else
            /// If we don't specify a purpose, we get a barebone scenes with almost no textures
            out = api.ComputeBoundMaterial();
#endif
            return out;
        }
    }
}
