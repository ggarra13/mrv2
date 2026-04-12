#include "USDGetTextureOrValue.h"

#include <pxr/usd/usdGeom/imageable.h>
#include <pxr/usd/usdGeom/subset.h>
#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/usdShade/materialBindingAPI.h>
#include <pxr/usd/usdShade/shader.h>

#include <iostream>

namespace tl
{
    namespace usd
    {
        using namespace PXR_NS;
        
        namespace
        {
            void FillEmptyShaderInputResult(ShaderInputResult& result,
                                            const TfToken& inputName)
            {
                result.hasValue = true;
                
                if (inputName == TfToken("diffuseColor") ||
                    inputName == TfToken("opacity") ||
                    inputName == TfToken("occlusion"))
                {
                    result.texturePath = "*solid";
                    result.value    = { 1.F, 1.F, 1.F, 1.F };
                }
                else if (inputName == TfToken("displacement") ||
                         inputName == TfToken("roughness") ||
                         inputName == TfToken("metallic"))
                {
                    result.texturePath = "*empty";
                    result.value    = { 0.F, 0.F, 0.F, 0.F };
                }
                else if (inputName == TfToken("normal"))
                {
                    result.texturePath = "*middle";
                    result.value    = { 0.5F, 0.5F, 0.5F, 0.5F };
                }
            }
        }

        ShaderInputResult GetTextureOrValue(const UsdPrim&   prim,
                                            const TfToken&   inputName,
                                            const bool            debug)
        {
            ShaderInputResult result;

            // ------------------------------------------------------------------
            // Helper: read a constant scalar or vector input into result.value.
            // Handles color3f/float3/normal3f → [r,g,b,1], color4f/float4 → [r,g,b,a],
            // and float → [v,v,v,v].
            // ------------------------------------------------------------------
            auto readConstantValue = [&](const UsdShadeInput& input)
            {
                if (!input) return;

                const SdfValueTypeName typeName = input.GetAttr().GetTypeName();

                if (typeName == SdfValueTypeNames->Color3f  ||
                    typeName == SdfValueTypeNames->Float3   ||
                    typeName == SdfValueTypeNames->Normal3f)
                {
                    GfVec3f v;
                    if (input.Get(&v))
                    {
                        if (debug) std::cerr << "got 3 values " << v[0] << " " << v[1] << " " << v[2] 
                                             << std::endl;
                        result.value    = { v[0], v[1], v[2], 1.0f };
                        result.hasValue = true;
                    }
                }
                else if (typeName == SdfValueTypeNames->Color4f ||
                         typeName == SdfValueTypeNames->Float4)
                {
                    GfVec4f v;
                    if (input.Get(&v))
                    {
                        if (debug) std::cerr << "got 4 values " << v[0] << " " << v[1] << " " << v[2] << " " << v[3]
                                             << std::endl;
                        result.value    = { v[0], v[1], v[2], v[3] };
                        result.hasValue = true;
                    }
                }
                else if (typeName == SdfValueTypeNames->Float)
                {
                    float v = 0.f;
                    if (input.Get(&v))
                    {
                        if (debug) std::cerr << "got 1 value " << v 
                                             << std::endl;
                        result.value    = { v, v, v, v };
                        result.hasValue = true;
                    }
                }
                else if (debug)
                {
                    std::cerr << "Unhandled input type for fallback value: "
                              << typeName.GetAsToken().GetString() << std::endl;
                }
            };

            // 1. Try to get the Material bound directly to the Prim
            UsdShadeMaterialBindingAPI bindingApi(prim);
            UsdShadeMaterial material = bindingApi.ComputeBoundMaterial();

            // 1b. If no direct material is found, check for GeomSubsets (per-face bindings)
            if (!material)
            {
                UsdGeomImageable imageable(prim);
                if (imageable)
                {
                    for (const UsdGeomSubset& subset : UsdGeomSubset::GetAllGeomSubsets(imageable))
                    {
                        UsdShadeMaterialBindingAPI subsetBinding(subset.GetPrim());
                        material = subsetBinding.ComputeBoundMaterial();
                        if (material) break;
                    }
                }
            }
            if (!material)
            {
                if (debug) std::cerr << "Did not find material for "
                                     << prim.GetPath().GetString() << std::endl;
                FillEmptyShaderInputResult(result, inputName);
                return result;
            }

            // DISPLACEMENT EXCEPTION:
            // Look at the Material's displacement output, not the surface shader.
            // No constant-value fallback is meaningful for displacement, so return as-is.
            if (inputName == TfToken("displacement"))
            {
                UsdShadeOutput       dispOutput = material.GetDisplacementOutput();
                UsdShadeConnectableAPI source;
                TfToken              sourceName;
                UsdShadeAttributeType sourceType;

                if (UsdShadeConnectableAPI::GetConnectedSource(
                        dispOutput, &source, &sourceName, &sourceType))
                {
                    UsdShadeShader textureShader(source.GetPrim());
                    UsdShadeInput  fileInput = textureShader.GetInput(TfToken("file"));
                    if (fileInput)
                    {
                        SdfAssetPath assetPath;
                        if (fileInput.Get(&assetPath))
                            result.texturePath = assetPath.GetResolvedPath();
                        else
                            FillEmptyShaderInputResult(result, inputName);
                    }
                }
                else
                {
                    FillEmptyShaderInputResult(result, inputName);
                }
                return result;
            }

            // 2. For all other inputs, get the Surface Shader
            UsdShadeShader surfaceShader = material.ComputeSurfaceSource(TfToken("universal"));
            if (!surfaceShader) return result;

            // 3. Look for the requested input on the shader
            UsdShadeInput shaderInput = surfaceShader.GetInput(inputName);
            if (!shaderInput)
            {
                if (debug) std::cerr << "Did not find "
                                     << inputName.GetString() << " input." << std::endl;
                FillEmptyShaderInputResult(result, inputName);
                return result;
            }

            // 4. Trace the connection to a texture node
            UsdShadeConnectableAPI source;
            TfToken                sourceName;
            UsdShadeAttributeType  sourceType;

            if (UsdShadeConnectableAPI::GetConnectedSource(
                    shaderInput.GetAttr(), &source, &sourceName, &sourceType))
            {
                UsdShadeShader textureShader(source.GetPrim());
                UsdShadeInput  fileInput = textureShader.GetInput(TfToken("file"));
                if (fileInput)
                {
                    SdfAssetPath assetPath;
                    if (fileInput.Get(&assetPath))
                    {
                        result.texturePath = assetPath.GetResolvedPath();
                        return result;                      // texture found — done
                    }
                }
            }

            // 5. No texture connection — read the constant value off the input directly
            readConstantValue(shaderInput);
            return result;
        }
    }
}
