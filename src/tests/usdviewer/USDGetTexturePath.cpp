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

        std::string GetTexturePath(const pxr::UsdPrim& prim, const pxr::TfToken& inputName, const bool debug) {
            using namespace PXR_NS;
            
            // 1. Try to get the Material bound directly to the Prim
            UsdShadeMaterialBindingAPI bindingApi(prim);
            UsdShadeMaterial material = bindingApi.ComputeBoundMaterial();
            
            // 1b. If no direct material is found, check for GeomSubsets (per-face bindings)
            if (!material) {
                UsdGeomImageable imageable(prim);
                if (imageable) {
                    std::vector<UsdGeomSubset> subsets = UsdGeomSubset::GetAllGeomSubsets(imageable);
                    for (const UsdGeomSubset& subset : subsets) {
                        UsdShadeMaterialBindingAPI subsetBinding(subset.GetPrim());
                        material = subsetBinding.ComputeBoundMaterial();
                        if (material) break; 
                    }
                }
            }

            if (!material) {
                if (debug) std::cerr << "Did not find material for " << prim.GetPath().GetString() << std::endl;
                return "";
            }

            // DISPLACEMENT EXCEPTION:
            // If we are looking for displacement, we look at the Material's output, not the surface shader.
            if (inputName == TfToken("displacement")) {
                UsdShadeOutput dispOutput = material.GetDisplacementOutput();
                UsdShadeConnectableAPI source;
                TfToken sourceName;
                UsdShadeAttributeType sourceType;
                
                if (UsdShadeConnectableAPI::GetConnectedSource(dispOutput, &source, &sourceName, &sourceType)) {
                    UsdShadeShader textureShader(source.GetPrim());
                    UsdShadeInput fileInput = textureShader.GetInput(TfToken("file"));
                    if (fileInput) {
                        SdfAssetPath assetPath;
                        if (fileInput.Get(&assetPath)) return assetPath.GetResolvedPath();
                    }
                }
                return "";
            }

            // 2. For all other textures, get the Surface Shader
            UsdShadeShader surfaceShader = material.ComputeSurfaceSource(TfToken("universal"));
            if (!surfaceShader) return "";

            // 3. Look for the requested input on the shader
            UsdShadeInput shaderInput = surfaceShader.GetInput(inputName);
            if (!shaderInput) {
                if (debug) std::cerr << "Did not find " << inputName.GetString() << " input." << std::endl;
                return "";
            }

            // 4. Trace the connection to the texture node
            UsdShadeConnectableAPI source;
            TfToken sourceName;
            UsdShadeAttributeType sourceType;

            if (UsdShadeConnectableAPI::GetConnectedSource(shaderInput.GetAttr(), &source, &sourceName, &sourceType)) {
                UsdShadeShader textureShader(source.GetPrim());
        
                // 5. Get the 'file' input from the texture shader
                UsdShadeInput fileInput = textureShader.GetInput(TfToken("file"));
                if (fileInput) {
                    SdfAssetPath assetPath;
                    if (fileInput.Get(&assetPath)) {
                        return assetPath.GetResolvedPath(); // Returns absolute path
                    }
                }
            }

            return "";
        }

    }
}
