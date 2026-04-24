

#include "usdgeom/mesh.h"

#include "usd/primvars.h"
#include "usd/primvarSampler.h"

#include "USDMeshOptimization.h"

#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/primvarsAPI.h>

#include <tlCore/Mesh.h>

namespace tl
{
    namespace usdgeom
    {
        using namespace PXR_NS;

        std::shared_ptr<geom::TriangleMesh3>
        Mesh::convert(usd::MeshOptimization& meshOptimization)
        {
            std::shared_ptr<geom::TriangleMesh3> out(new geom::TriangleMesh3);

            UsdGeomMesh usdMesh(prim);
            
            std::vector<usd::PrimvarAndType> primvarsAndType = usd::GetPrimvars(prim);

            // \@bug: OpenUSD supports multiple uv sets per geometry (actually,
            //        per texture).
            //        If we find that, then we will use the last one and
            //        spit out a warning.
            int numUVSets = 0;
            int uvCount = 0;
            for (const auto& pvt : primvarsAndType)
            {
                if (pvt.type == TfToken("st"))
                {
                    ++numUVSets;
                }
            }
            if (numUVSets > 1)
            {
                static bool warned = false;
                if (!warned)
                {
                    std::cerr << prim.GetPath().GetString() << " (among others?) " << std::endl
                              << "has more than one set of uv coordinates. " 
                              << "Not yet supported. Picking last one." << std::endl;
                    warned = true;
                    for (const auto& pvt : primvarsAndType)
                    {
                        if (pvt.type == TfToken("st"))
                        {
                            std::cout << "\t\t" << pvt.name << std::endl;
                        }
                    }
                }
            }
            
            
            for (const auto& pvt : primvarsAndType)
            {
                const UsdGeomPrimvar& pv = pvt.pv;
                const TfToken type = pvt.type;

                if (type == TfToken("st"))
                {
                    // We'll use whatever UV comes last
                    ++uvCount;
                    if (uvCount < numUVSets)
                        continue;
                    
                    // Get STs if any.
                    usd::PrimvarSampler<GfVec2f> sampler(pv, time);
                    if (sampler.IsValid())
                    {
                        int faceCornerIdx = 0; 
                        for (size_t faceIdx = 0; faceIdx < faceVertexCounts.size();
                             ++faceIdx) {
                            int vertCount = faceVertexCounts[faceIdx];
                            
                            for (int i = 0; i < vertCount; ++i) {
                                int faceVertexIdx = faceCornerIdx + i;
                                int pointIdx = faceVertexIndices[faceVertexIdx];
                            
                                GfVec2f uv = sampler.Sample(faceIdx,
                                                            faceVertexIdx,
                                                            pointIdx);
                                
                                if (uv[0] < 0.F || uv[0] > 1.F ||
                                    uv[1] < 0.F || uv[1] > 1.F)
                                    meshOptimization.floatUVs = true;
                                
                                out->t.emplace_back(math::Vector2f(uv[0], 1.0F - uv[1]));
                            }
                            faceCornerIdx += vertCount;
                        }
                    }
                }
                else if (type == TfToken("normal"))
                {
                    // Get Normals if any.
                    usd::PrimvarSampler<GfVec3f> sampler(pv, time);
                    if (sampler.IsValid()) {
                        int faceCornerIdx = 0; 
                        for (size_t faceIdx = 0;
                             faceIdx < faceVertexCounts.size();
                             ++faceIdx) {
                            int vertCount = faceVertexCounts[faceIdx];
                        
                            for (int i = 0; i < vertCount; ++i) {
                                int faceVertexIdx = faceCornerIdx + i;
                                int pointIdx = faceVertexIndices[faceVertexIdx];
                                
                                GfVec3f n = sampler.Sample(faceIdx,
                                                           faceVertexIdx,
                                                           pointIdx);
                            
                                if (n[0] < -1.F || n[0] > 1.F ||
                                    n[1] < -1.F || n[1] > 1.F ||
                                    n[2] < -1.F || n[2] > 1.F)
                                    meshOptimization.floatNormals = true;
                            
                                out->n.emplace_back(math::Vector3f(n[0], n[1], n[2]));
                            }
                            faceCornerIdx += vertCount;
                        }
                    }
                }
                else if (type == TfToken("color"))
                {
                    usd::PrimvarSampler<GfVec4f> sampler(pv, time);
                    if (sampler.IsValid()) {
                        int faceCornerIdx = 0; 
                        for (size_t faceIdx = 0; faceIdx < faceVertexCounts.size();
                             ++faceIdx) {
                            int vertCount = faceVertexCounts[faceIdx];
                        
                            for (int i = 0; i < vertCount; ++i) {
                                int faceVertexIdx = faceCornerIdx + i;
                                int pointIdx = faceVertexIndices[faceVertexIdx];

                                GfVec4f c = sampler.Sample(faceIdx,
                                                           faceVertexIdx,
                                                           pointIdx);
                            
                                if (c[0] < 0.F || c[0] > 1.F ||
                                    c[1] < 0.F || c[1] > 1.F ||
                                    c[2] < 0.F || c[2] > 1.F ||
                                    c[3] < 0.F || c[3] > 1.F)
                                    meshOptimization.floatColors = true;
                            
                                out->c.emplace_back(
                                    math::Vector4f(c[0], c[1], c[2], c[3]));
                            }
                            faceCornerIdx += vertCount;
                        }
                    }
                }
                // else
                // {
                //     std::cerr << "Unknown type for primvar type="
                //               << type.GetString()
                //               << std::endl;
                // }
            }

            // Get points.
            out->v.reserve(points.size());
            for (int i = 0; i < points.size(); ++i)
            {
                const auto& p = points[i];
                out->v.emplace_back(math::Vector3f(p[0], p[1], p[2]));
            }
            
            const bool hasUVs = !out->t.empty();
            const bool hasNormals = !out->n.empty();
            const bool hasColors = !out->c.empty();

            // Create triangles.
            int indexOffset = 0;
            geom::Triangle3 triangle;
            for (int vertCount : faceVertexCounts)
            {
                // Fan triangulation: anchor at faceVertexIndices[indexOffset]
                // e.g. a quad [A, B, C, D] → (A,B,C), (A,C,D)
                for (int i = 1; i < vertCount - 1; ++i)
                {
                    const int i0 = faceVertexIndices[indexOffset];
                    const int i1 = faceVertexIndices[indexOffset + i];
                    const int i2 = faceVertexIndices[indexOffset + i + 1];
                    
                    triangle.v[0].v = i0 + 1;
                    triangle.v[1].v = i1 + 1;
                    triangle.v[2].v = i2 + 1;
                    
                    if (hasUVs)
                    {
                        triangle.v[0].t = indexOffset + 1;
                        triangle.v[1].t = indexOffset + i + 1;
                        triangle.v[2].t = indexOffset + i + 2;
                    }
                    
                    if (hasNormals)
                    {
                        triangle.v[0].n = indexOffset + 1;
                        triangle.v[1].n = indexOffset + i + 1;
                        triangle.v[2].n = indexOffset + i + 2;
                    }

                    if (hasColors)
                    {
                        triangle.v[0].c = indexOffset + 1;
                        triangle.v[1].c = indexOffset + i + 1;
                        triangle.v[2].c = indexOffset + i + 2;
                    }
                    
                    out->triangles.emplace_back(triangle);
                }
                indexOffset += vertCount;
            }

            return out;
        }
    }
}
