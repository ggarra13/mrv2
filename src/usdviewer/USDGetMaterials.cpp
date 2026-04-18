#include "USDGetMaterials.h"

#include <pxr/usd/usd/primRange.h>

#include <pxr/usd/usdGeom/imageable.h>
#include <pxr/usd/usdGeom/subset.h>
#include <pxr/usd/usdShade/materialBindingAPI.h>
#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/usdShade/shader.h>
#include <pxr/usd/usdShade/tokens.h>

#include <iostream>

namespace tl
{
    namespace usd
    {
        using namespace PXR_NS;
        
        namespace
        {
            vlk::TextureBorder getBorder(const TfToken& borderToken)
            {
                vlk::TextureBorder out = vlk::TextureBorder::ClampToEdge;
                
                const std::string border = borderToken.GetText();
                if (border == "clamp")
                    return out;
                else if (border == "repeat")
                    out = vlk::TextureBorder::Repeat;
                else if (border == "mirror")
                    out = vlk::TextureBorder::MirroredRepeat;
                else if (border == "black")
                    out = vlk::TextureBorder::ClampToBorder;
                return out;
            }
            
            void FillEmptyShaderInputResult(ShaderInputResult& result,
                                            const TfToken& inputName)
            {
                result.hasValue = true;
                
                if (inputName == TfToken("diffuseColor") || // ok
                    inputName == TfToken("opacity") ||   // ok
                    inputName == TfToken("occlusion"))      // ok
                {
                    result.texturePath = "*solid";
                    result.value    = { 1.F, 1.F, 1.F, 1.F };
                }
                else if (inputName == TfToken("metallic") ||  // ok 
                         inputName == TfToken("displacement") ||
                         inputName == TfToken("emissiveColor"))     // ok
                {
                    result.texturePath = "*empty";
                    result.value    = { 0.F, 0.F, 0.F, 0.F };
                }
                else if (inputName == TfToken("roughness"))
                {
                    result.texturePath = "*middle";
                    result.value    = { 0.5F, 0.5F, 0.5F, 0.5F };
                }
                else if(inputName == TfToken("normal"))  // ok
                {
                    // These values make sure the normal is not perturbed.
                    result.texturePath = "*normal";
                    result.value    = { 0.5F, 0.5F, 1.0F, 1.0F };
                }
            }
        }

        ShaderInputResult GetTextureOrValue(const UsdShadeMaterial& material,
                                            const TfToken&   inputName,
                                            const bool       debug)
        {
            ShaderInputResult result;
            if (!material)
            {
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
            UsdShadeShader surfaceShader = material.ComputeSurfaceSource(TfToken("glslfx"));
            if (!surfaceShader)
            {
                FillEmptyShaderInputResult(result, inputName);
                return result;
            }

            // 3. Look for the requested input on the shader
            UsdShadeInput shaderInput = surfaceShader.GetInput(inputName);
            if (!shaderInput)
            {
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
                        // Access the wrapS and wrapT inputs
                        UsdShadeInput wrapSInput = textureShader.GetInput(TfToken("wrapS"));
                        UsdShadeInput wrapTInput = textureShader.GetInput(TfToken("wrapT"));

                        // Get S and T border wrapping (clamp, repeat, black, mirror, etc)
                        TfToken sVal, tVal;
                        if (wrapSInput && wrapSInput.Get(&sVal)) {
                            result.borderU = getBorder(sVal);
                        }
                        if (wrapTInput && wrapTInput.Get(&tVal)) {
                            result.borderV = getBorder(sVal);
                        }

                        // Get the channel of the connection
                        result.channel = sourceName.GetString();
    
                        result.texturePath = assetPath.GetResolvedPath();
                        if (debug && !result.texturePath.empty())
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

        float GetShaderFloatValue(const UsdShadeMaterial& material,
                                  const TfToken&   inputName,
                                  const bool       debug)
        {
            float out = 0.F;
            UsdShadeShader surfaceShader = material.ComputeSurfaceSource(TfToken("glslfx"));
            if (!surfaceShader)
            {
                return out;
            }

            UsdShadeInput shaderInput = surfaceShader.GetInput(inputName);
            if (!shaderInput)
            {
                return out;
            }
            
            shaderInput.Get(&out);
            return out;
        }

        
        Material ParseMaterial(const UsdShadeMaterial& material,
                               const bool debug)
        {
            Material out;
            
            out.diffuseColor = GetTextureOrValue(material, TfToken("diffuseColor"), debug);
            out.emissiveColor = GetTextureOrValue(material, TfToken("emissiveColor"), debug);
            out.opacity = GetTextureOrValue(material, TfToken("opacity"), debug);
            out.metallic = GetTextureOrValue(material, TfToken("metallic"), debug);
            out.roughness = GetTextureOrValue(material, TfToken("roughness"), debug);
            out.normal = GetTextureOrValue(material, TfToken("normal"), debug);
            out.occlusion = GetTextureOrValue(material, TfToken("occlusion"), debug);
            out.displacement = GetTextureOrValue(material, TfToken("displacement"), debug);

            out.opacityThreshold = GetShaderFloatValue(material,
                                                       TfToken("opacityThreshold"),
                                                       debug);
            if (out.opacityThreshold > 0.F)
                out.transparent = true;
            if (!out.opacity.texturePath.empty() ||
                (out.opacity.hasValue &&
                 (out.opacity.value[0] < 0.95F ||
                  out.opacity.value[1] < 0.95F ||
                  out.opacity.value[2] < 0.95F)))
                out.transparent = true;
            
            return out;
        }

        
        void CollectMaterials(Fl_Vk_Context& ctx,
                              const pxr::UsdStageRefPtr stage,
                              std::unordered_map<std::string, usd::Material >& out)
        {
            UsdPrimRange range(stage->GetPseudoRoot(),
                               UsdTraverseInstanceProxies());
            std::cout << "Started Reading materials..." << std::endl;

            bool debug = true;
#if 1
            for (auto it = range.begin(); it != range.end(); ++it) {

                if (!UsdShadeMaterialBindingAPI::CanApply(*it))
                    continue;

                UsdShadeMaterialBindingAPI bindingApi(*it);
                // UsdShadeMaterial material =
                //     bindingApi.ComputeBoundMaterial(UsdShadeTokens->preview);  
                UsdShadeMaterial material =
                    bindingApi.ComputeBoundMaterial(UsdShadeTokens->full);  
                Material result = ParseMaterial(material, debug);
                out[material.GetPath().GetString()] = result;
            }

#elif 0
            
            for (auto it = range.begin(); it != range.end(); ++it) {

                //
                // Ignore hidden geometry
                //
                if (!it->IsA<UsdShadeMaterial>())
                    continue;

                UsdShadeMaterial material(*it);
                
                Material result = ParseMaterial(material, debug);
                out[material.GetPath().GetString()] = result;
            }
#else
            std::unordered_map<std::string, UsdShadeMaterial> usedMaterials;

            for (const UsdPrim& prim : range)
            {
                if (!UsdShadeMaterialBindingAPI::CanApply(prim))
                    continue;

                UsdShadeMaterialBindingAPI api(prim);

                for (auto purpose : {
                        UsdShadeTokens->full,
                        UsdShadeTokens->preview })
                {
                    UsdShadeMaterial mat = api.ComputeBoundMaterial(purpose);
                    if (mat)
                        usedMaterials[mat.GetPath().GetString()] = mat;
                }

                // 🔥 IMPORTANT: subsets
                if (prim.IsA<UsdGeomImageable>())
                {
                    auto subsets =
                        UsdGeomSubset::GetAllGeomSubsets(
                            UsdGeomImageable(prim));

                    for (const auto& subset : subsets)
                    {
                        UsdShadeMaterialBindingAPI subApi(subset.GetPrim());
                        UsdShadeMaterial mat = subApi.ComputeBoundMaterial();

                        if (mat)
                            usedMaterials[mat.GetPath().GetString()] = mat;
                    }
                }
            }

            for (auto& [path, material] : usedMaterials)
            {
                Material result = ParseMaterial(material, debug);
                out[path] = result;
            }
#endif
            std::cout << "Finished Reading " << out.size()
                      << " materials..." << std::endl;
        }
    }
}
