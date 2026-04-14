#include "USDGetMaterials.h"

#include <pxr/usd/usdGeom/imageable.h>
#include <pxr/usd/usdGeom/subset.h>
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
                                            const UsdPrim&   prim,
                                            const TfToken& inputName)
            {
                result.hasValue = true;
                
                VtArray<GfVec3f> colors;
                UsdGeomGprim gprim(prim);
                if (gprim)
                    gprim.GetDisplayColorAttr().Get(&colors);
                if (inputName == TfToken("diffuseColor") && colors.size() == 1)
                {
                    result.value[0] = colors[0][0];
                    result.value[1] = colors[0][1];
                    result.value[2] = colors[0][2];
                    result.value[3] = colors[0][3];
                    return;
                }
                
                if (inputName == TfToken("diffuseColor") || // ok
                    inputName == TfToken("opacity") ||   // ok
                    inputName == TfToken("occlusion") || // ok
                    inputName == TfToken("normal"))      // ok
                {
                    result.texturePath = "*solid";
                    result.value    = { 1.F, 1.F, 1.F, 1.F };
                }
                else if (inputName == TfToken("metallic") ||  // ok 
                         inputName == TfToken("displacement"))     // ok
                {
                    result.texturePath = "*empty";
                    result.value    = { 0.F, 0.F, 0.F, 0.F };
                }
                else if (inputName == TfToken("roughness"))
                {
                    result.texturePath = "*middle";
                    result.value    = { 0.5F, 0.5F, 0.5F, 0.5F };
                }
            }
        }

        ShaderInputResult GetTextureOrValue(const UsdPrim&   prim,
                                            const UsdShadeMaterial& material,
                                            const TfToken&   inputName,
                                            const bool       debug)
        {
            ShaderInputResult result;
            if (!material)
            {
                FillEmptyShaderInputResult(result, prim, inputName);
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
                            FillEmptyShaderInputResult(result, prim, inputName);
                    }
                }
                else
                {
                    FillEmptyShaderInputResult(result, prim, inputName);
                }
                return result;
            }
            
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
                        result.value    = { v[0], v[1], v[2], v[3] };
                        result.hasValue = true;
                    }
                }
                else if (typeName == SdfValueTypeNames->Float)
                {
                    float v = 0.f;
                    if (input.Get(&v))
                    {
                        result.value    = { v, v, v, v };
                        result.hasValue = true;
                    }
                }
            };
                        
            // 2. For all other inputs, get the Surface Shader
            UsdShadeShader surfaceShader = material.ComputeSurfaceSource(TfToken("universal"));
            if (!surfaceShader)
            {
                FillEmptyShaderInputResult(result, prim, inputName);
                return result;
            }

            // 3. Look for the requested input on the shader
            UsdShadeInput shaderInput = surfaceShader.GetInput(inputName);
            if (!shaderInput)
            {
                FillEmptyShaderInputResult(result, prim, inputName);
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
                        if (debug)
                        {
                            std::cout << "\t" << inputName << " has " << result.texturePath << std::endl;
                        }
                        return result;                      // texture found — done
                    }
                }
            }

            // 5. No texture connection — read the constant value off the input directly
            readConstantValue(shaderInput);

            return result;
        }

        
        Material ParseMaterial(const UsdPrim& prim,
                               const UsdShadeMaterial& material,
                               const bool debug)
        {
            Material out;
            
            out.diffuseColor = GetTextureOrValue(prim, material, TfToken("diffuseColor"), debug);
            out.opacity = GetTextureOrValue(prim, material, TfToken("opacity"), debug);
            out.metallic = GetTextureOrValue(prim, material, TfToken("metallic"), debug);
            out.roughness = GetTextureOrValue(prim, material, TfToken("roughness"), debug);
            out.normal = GetTextureOrValue(prim, material, TfToken("normal"), debug);
            out.occlusion = GetTextureOrValue(prim, material, TfToken("occlusion"), debug);
            out.displacement = GetTextureOrValue(prim, material, TfToken("displacement"), debug);

            return out;
        }
        
        std::unordered_map<std::string, Material >
        GetMaterials(const pxr::UsdPrim& prim,
                     const bool          debug)
        {
            std::unordered_map<std::string, Material > out;
            
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
                        if (material)
                        {
                            Material result = ParseMaterial(prim, material, debug);
                            out[material.GetPath().GetString()] = result;
                        }
                    }
                }
            }
            if (material)
            {
                Material result = ParseMaterial(prim, material, debug);
                out[material.GetPath().GetString()] = result;
            }

            return out;
        }
    }
}
