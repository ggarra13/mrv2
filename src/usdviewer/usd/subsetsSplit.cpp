
#include "usd/subsetsSplit.h"
#include "usd/material.h"

#include <pxr/base/gf/vec2f.h>

// Material and Shaders
#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/usdShade/materialBindingAPI.h>
#include <pxr/usd/usdShade/shader.h>

#include <pxr/usd/usdGeom/primvarsAPI.h>

#include <iostream>

namespace tl
{
    namespace usd
    {
        using namespace PXR_NS;
        
        struct UVData
        {
            VtArray<GfVec2f> values;
            VtArray<int> indices;   // optional
            TfToken interpolation;
            bool indexed = false;
        };

        UVData GetMeshUVs(const UsdGeomMesh& mesh)
        {
            UVData uv;

            UsdGeomPrimvarsAPI primvars(mesh);
            UsdGeomPrimvar st = primvars.GetPrimvar(TfToken("st"));

            if (!st)
                return uv;

            st.Get(&uv.values);
            uv.interpolation = st.GetInterpolation();

            if (st.IsIndexed())
            {
                st.GetIndices(&uv.indices);
                uv.indexed = true;
            }

            return uv;
        }


        std::vector<UsdGeomMesh> subsetsSplit(
            UsdStageRefPtr stage,
            const UsdGeomMesh& mesh,
            const VtArray<GfVec3f>& points,
            const VtArray<int>& faceCounts,
            const VtArray<int> faceIndices,
            const std::vector<UsdGeomSubset>& subsets)
        {
            std::vector<UsdGeomMesh> out;
            out.reserve(subsets.size());            
            
            UVData srcUV = GetMeshUVs(mesh);
            for (const UsdGeomSubset& subset : subsets)
            {
                TfToken elementType;
                subset.GetElementTypeAttr().Get(&elementType);

                if (elementType != UsdGeomTokens->face)
                    continue; // only handling face subsets

                VtArray<GfVec2f> newUVs;

                int faceVertexCursor = 0;

                VtArray<int> subsetFaces;
                subset.GetIndicesAttr().Get(&subsetFaces);
        
                std::unordered_set<int> subsetSet(subsetFaces.begin(), subsetFaces.end());
        
                struct SubsetMeshData
                {
                    VtArray<GfVec3f> points;
                    VtArray<int> faceVertexCounts;
                    VtArray<int> faceVertexIndices;
                };

                SubsetMeshData data;

                std::unordered_map<int, int> oldToNewIndex;

                // --- Iterate faces ---
                int faceStart = 0;
                for (size_t faceId = 0; faceId < faceCounts.size(); ++faceId)
                {
                    int count = faceCounts[faceId];

                    // Check if this face is in the subset
                    if (subsetSet.find(faceId) == subsetSet.end())
                    {
                        faceStart += count;
                        faceVertexCursor += count;
                        continue;
                    }

                    data.faceVertexCounts.push_back(count);

                    for (int i = 0; i < count; ++i)
                    {
                        int oldIndex = faceIndices[faceStart + i];

                        auto it = oldToNewIndex.find(oldIndex);
                        int newIndex;

                        if (it == oldToNewIndex.end())
                        {
                            newIndex = static_cast<int>(data.points.size());
                            oldToNewIndex[oldIndex] = newIndex;
                            data.points.push_back(points[oldIndex]);
                        }
                        else
                        {
                            newIndex = it->second;
                        }

                        data.faceVertexIndices.push_back(newIndex);

                        if (srcUV.values.size() > 0)
                        {
                            if (srcUV.interpolation == UsdGeomTokens->faceVarying)
                            {
                                int uvIdx = faceVertexCursor + i;

                                if (srcUV.indexed)
                                    uvIdx = srcUV.indices[uvIdx];

                                newUVs.push_back(srcUV.values[uvIdx]);
                            }
                            else if (srcUV.interpolation == UsdGeomTokens->vertex)
                            {
                                if (it == oldToNewIndex.end()) 
                                {
                                    int uvIdx = oldIndex;
                        
                                    if (srcUV.indexed)
                                        uvIdx = srcUV.indices[uvIdx];

                                    // remap like points
                                    newUVs.push_back(srcUV.values[uvIdx]);
                                }
                            }
                            else if (srcUV.interpolation == UsdGeomTokens->uniform)
                            {
                            }
                            else if (srcUV.interpolation == UsdGeomTokens->face)
                            {
                            }
                            else if (srcUV.interpolation == UsdGeomTokens->constant)
                            {
                            }
                        }

                    }

                    faceStart += count;
                    faceVertexCursor += count;
                }

                // --- Create new mesh prim ---
                std::string subsetBase = subset.GetPrim().GetName().GetString();
                std::string safeName = TfMakeValidIdentifier(subsetBase);

                if (safeName.empty())
                    safeName = "subset";

                // Add suffix to avoid collisions
                safeName += "_geom";
                UsdPrim parentPrim = mesh.GetPrim().GetParent();
                SdfPath newPath = parentPrim.GetPath().AppendChild(TfToken(safeName));

                if (newPath.IsEmpty()) {
                    std::cerr << "Still failed to create path: " << safeName << std::endl;
                    continue;
                }
        
                UsdGeomMesh newMesh = UsdGeomMesh::Define(stage, newPath);
                if (!newMesh)
                {
                    std::cerr << "Could not create new mesh at "
                              << newPath.GetString() << std::endl;
                    continue;
                }

                newMesh.CreatePointsAttr().Set(data.points);
                newMesh.CreateFaceVertexCountsAttr().Set(data.faceVertexCounts);
                newMesh.CreateFaceVertexIndicesAttr().Set(data.faceVertexIndices);
        
                if (!newUVs.empty())
                {
                    UsdGeomPrimvarsAPI pv(newMesh);
            
                    UsdGeomPrimvar newSt =
                        pv.CreatePrimvar(
                            TfToken("st"),
                            SdfValueTypeNames->TexCoord2fArray,
                            srcUV.interpolation
                            );

                    newSt.Set(newUVs);
                }
        
                UsdShadeMaterialBindingAPI subsetBinding(subset.GetPrim());
                UsdShadeMaterial material = usd::GetMaterial(subsetBinding);
                if (material)
                {
                    // Bind material to the new mesh
                    UsdShadeMaterialBindingAPI newBinding =
                        UsdShadeMaterialBindingAPI::Apply(newMesh.GetPrim());
                    newBinding.Bind(material);
                }
                else
                {
                    // Fallback: try mesh-level material
                    UsdShadeMaterialBindingAPI meshBinding(mesh.GetPrim());
                    material = usd::GetMaterial(meshBinding);
                    if (material)
                    {
                        UsdShadeMaterialBindingAPI newBinding =
                            UsdShadeMaterialBindingAPI::Apply(newMesh.GetPrim());
                        newBinding.Bind(material);
                    }
                    else
                    {
                        // \@note: Attaching no material works fine (renders lambert)
                    }
                }

                out.push_back(newMesh);
            }

            return out;
        }

    }
}
