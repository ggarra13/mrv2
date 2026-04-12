#pragma once

#include <pxr/usd/usd/prim.h>

#include <array>
#include <string>

namespace tl
{
    namespace usd
    {
        struct ShaderInputResult
        {
            std::string           texturePath;          // non-empty when a texture is found
            std::array<float, 4>  value  = {};          // constant value when no texture
            bool                  hasValue = false;     // true when value is populated
        };

        ShaderInputResult GetTextureOrValue(const pxr::UsdPrim&   prim,
                                            const pxr::TfToken&   inputName,
                                            const bool            debug = true);
    }
}
