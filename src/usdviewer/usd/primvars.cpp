#include "usd/material.h"
#include "usd/primvars.h"

#include <iostream>

namespace tl
{
    namespace usd
    {
        using namespace pxr;

        struct PrimvarRequest {
            TfToken varname;
            TfToken readerType;  // e.g. "UsdPrimvarReader_float2"
        };
        
        void CollectPrimvarReadersRecursive(
            const UsdShadeShader& shader,
            std::vector<PrimvarRequest>& requests)
        {
            if (!shader) return;

            TfToken shaderId;
            shader.GetShaderId(&shaderId);

            // Check if this node IS a primvar reader
            if (shaderId == TfToken("UsdPrimvarReader_float2") ||
                shaderId == TfToken("UsdPrimvarReader_float")  ||
                shaderId == TfToken("UsdPrimvarReader_float3"))
            {
                UsdShadeInput varnameInput = shader.GetInput(TfToken("varname"));
                if (varnameInput)
                {
                    VtValue val;
                    if (varnameInput.Get(&val) && val.IsHolding<TfToken>())
                        requests.push_back({val.UncheckedGet<TfToken>(), shaderId});
                    else if (val.IsHolding<std::string>())
                        requests.push_back({TfToken(val.UncheckedGet<std::string>()), shaderId});
                }
                return; // readers have no upstream shaders to recurse into
            }

            // Recurse into all connected inputs
            for (const UsdShadeInput& input : shader.GetInputs())
            {
                UsdShadeConnectableAPI source;
                TfToken sourceName;
                UsdShadeAttributeType sourceType;
                if (UsdShadeConnectableAPI::GetConnectedSource(
                        input, &source, &sourceName, &sourceType))
                {
                    CollectPrimvarReadersRecursive(UsdShadeShader(source.GetPrim()), requests);
                }
            }
        }

        std::vector<PrimvarAndType> GetPrimvars(UsdPrim prim)
        {
            std::vector<PrimvarAndType> out;
            
            // 1. Resolve the bound material for the purpose
            UsdShadeMaterialBindingAPI bindingAPI(prim);
            UsdShadeMaterial material = usd::GetMaterial(bindingAPI);

            if (material)
            {
                // 2. Get the surface terminal output
                UsdShadeShader surface = material.ComputeSurfaceSource(
                    {UsdShadeTokens->preview}
                    );

                if (surface)
                {
                    // 3. Walk the network collecting PrimvarReader varnames
                    std::vector<PrimvarRequest> requests;
                    CollectPrimvarReadersRecursive(surface, requests);

                    // 4. Look up only what the material needs
                    UsdGeomPrimvarsAPI primvarsAPI(prim);
                    for (const auto& request : requests)
                    {
                        UsdGeomPrimvar pv = primvarsAPI.FindPrimvarWithInheritance(request.varname);
                        if (pv && pv.IsDefined())
                        {
                            PrimvarAndType pvt;
                            pvt.pv = pv;
                            pvt.name = pv.GetName();
                            pvt.type = ClassifyPrimvar(pv, request.readerType);
                            out.emplace_back(pvt);
                        }
                    }
                }
            }

            if (out.empty())
            {
                UsdGeomPrimvarsAPI primvarsAPI(prim);
                for (const UsdGeomPrimvar& pv : primvarsAPI.FindPrimvarsWithInheritance())
                {
                    if (!pv.IsDefined()) continue;

                    TfToken role = pv.GetTypeName().GetRole();

                    if (role == SdfValueRoleNames->TextureCoordinate ||
                        role == SdfValueRoleNames->Normal            ||
                        role == SdfValueRoleNames->Color             ||
                        role == SdfValueRoleNames->Point)
                    {
                        PrimvarAndType pvt;
                        pvt.pv = pv;
                        pvt.name = pv.GetName();
                        pvt.type = ClassifyPrimvar(pv, TfToken());
                        out.emplace_back(pvt);
                    }
                }
            }

            std::sort(out.begin(), out.end(),
                      [](const PrimvarAndType& a, const PrimvarAndType& b){
                          return a.name > b.name;
                      });
            out.erase(
                std::unique(out.begin(), out.end(),
                            [](const PrimvarAndType& a, const PrimvarAndType& b){
                                return a.name == b.name;
                            }),
                out.end());
            return out;
        }
        
    }  // namespace usd
}  // namespace tl

