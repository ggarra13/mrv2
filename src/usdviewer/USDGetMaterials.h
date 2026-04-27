#pragma once

#include "engine/options.h"

#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/timeCode.h>

#include <tlVk/Texture.h>

#include <array>
#include <string>
#include <unordered_map>

#include "USDMaterial.h"

namespace tl
{
    namespace usd
    {

        // Get a list of textures and/or values for each material in a primitive
        ShaderInputResult GetTextureOrValue(
            const pxr::UsdPrim& prim,
            const pxr::UsdShadeMaterial& material,
            const pxr::TfToken& inputName,
            const bool          debug = DEBUG_TEXTURES);
        
        void
        GetMaterials(const pxr::UsdPrim& prim,
                     std::unordered_map<std::string, Material >& out,
                     const bool          debug = DEBUG_MATERIALS);

        void CollectMaterials(Fl_Vk_Context& ctx,
                              const pxr::UsdStageRefPtr stage,
                              const pxr::UsdTimeCode time,
                              std::unordered_map<std::string,
                              usd::Material >& materials);
    }
}
