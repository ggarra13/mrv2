#include <pxr/usd/usd/prim.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/usdShade/materialBindingAPI.h>
#include <pxr/usd/usdShade/shader.h>

#include <iostream>
#include <string>

namespace tl
{
    namespace usd
    {
        

        std::string GetDiffuseTexturePath(const pxr::UsdPrim& prim) {
            using namespace PXR_NS;
        
            // 1. Get the Material bound to the Prim
            UsdShadeMaterialBindingAPI bindingApi(prim);
            UsdShadeMaterial material = bindingApi.ComputeBoundMaterial();

            if (!material) {
                return "";
            }

            // 2. Get the Surface Shader source
            // We use the 'universal' render context for standard UsdPreviewSurface
            UsdShadeShader surfaceShader = material.ComputeSurfaceSource(TfToken("universal"));
    
            if (!surfaceShader) {
                return "";
            }

            // 3. Look for the 'diffuseColor' input on the shader
            UsdShadeInput diffuseInput = surfaceShader.GetInput(TfToken("diffuseColor"));
            if (!diffuseInput) {
                return "";
            }

            // 4. Trace the connection to the texture node
            UsdShadeConnectableAPI source;
            TfToken sourceName;
            UsdShadeAttributeType sourceType;

            if (UsdShadeConnectableAPI::GetConnectedSource(diffuseInput.GetAttr(), &source, &sourceName, &sourceType)) {
                // The source is another shader (the UsdUVTexture node)
                UsdShadeShader textureShader(source.GetPrim());
        
                // 5. Get the 'file' input from the texture shader
                UsdShadeInput fileInput = textureShader.GetInput(TfToken("file"));
                if (fileInput) {
                    SdfAssetPath assetPath;
                    if (fileInput.Get(&assetPath)) {
                        // Return the resolved path (absolute system path)
                        return assetPath.GetResolvedPath();
                    }
                }
            }

            return "";
        }
    }
}
