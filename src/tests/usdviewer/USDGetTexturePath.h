#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/imageable.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/subset.h>
#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/usdShade/materialBindingAPI.h>
#include <pxr/usd/usdShade/shader.h>

#include <iostream>
#include <string>
#include <unordered_map>

namespace tl
{
    namespace usd
    {
        std::string GetTexturePath(const pxr::UsdPrim& prim,
                                   const pxr::TfToken& inputName,
                                   const bool debug = false);
    }
}
