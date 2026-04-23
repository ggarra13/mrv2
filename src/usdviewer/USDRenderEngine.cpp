
#define PRINT_STATS 1
#define BAKE_JOINTS 1

// #include "USDProcessSkeletonRoot.h"  // \@todo: do deformation in compute shader

#include "USDCollectTextures.h"
#include "USDGetMaterials.h"
#include "USDRender.h"
#include "USDRenderShadersBinary.h"
#include "USDTextureSlots.h"
#include "USDTransparentPrimitive.h"

#include "usd/material.h"
#include "usd/primvars.h"
#include "usd/primvarSampler.h"

#include "USDRenderEngine.h"

#include <tlCore/Context.h>

#include <pxr/pxr.h>

// math primitives
#include <pxr/base/gf/camera.h>
#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/matrix4d.h>
#include <pxr/base/gf/range3d.h>
#include <pxr/base/gf/vec3f.h>

// diagnostics
#include <pxr/base/tf/diagnostic.h>
#include <pxr/base/tf/token.h>
#include <pxr/base/tf/stringUtils.h>

#include <pxr/usd/usd/stage.h>
#include <pxr/usd/usd/timeCode.h>
#include <pxr/usd/usd/primRange.h>

#include <pxr/usd/usdUtils/pipeline.h>

#include <pxr/imaging/hd/renderBuffer.h>
#include <pxr/imaging/hdSt/hioConversions.h>
#include <pxr/imaging/hdSt/textureUtils.h>
#include <pxr/imaging/hdx/tokens.h>
#include <pxr/imaging/hdx/types.h>

// Primitive types (refactor all these for scene traversal)
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/camera.h>
#include <pxr/usd/usdGeom/capsule.h>
#include <pxr/usd/usdGeom/cone.h>
#include <pxr/usd/usdGeom/cube.h>
#include <pxr/usd/usdGeom/cylinder.h>
#include <pxr/usd/usdGeom/metrics.h>
#include <pxr/usd/usdGeom/nurbsCurves.h>
#include <pxr/usd/usdGeom/nurbsPatch.h>
#include <pxr/usd/usdGeom/sphere.h>
#include <pxr/usd/usdGeom/xform.h>
#include <pxr/usd/usdGeom/xformCache.h>
#include <pxr/usd/usdGeom/primvarsAPI.h>

// Skeleton types
#include <pxr/usd/usdSkel/animQuery.h>
#include <pxr/usd/usdSkel/bakeSkinning.h>
#include <pxr/usd/usdSkel/bindingAPI.h>
#include <pxr/usd/usdSkel/cache.h>
#include <pxr/usd/usdSkel/root.h>
#include <pxr/usd/usdSkel/skeletonQuery.h>
#include <pxr/usd/usdSkel/utils.h>

// Material and Shaders
#include <pxr/usd/usdShade/material.h>
#include <pxr/usd/usdShade/materialBindingAPI.h>
#include <pxr/usd/usdShade/shader.h>

// Not sure about these
#include <pxr/imaging/hdx/tokens.h>
#include <pxr/imaging/hdx/types.h>

#include <tlTimelineVk/RenderShadersBinary.h>

#include <tlVk/OffscreenBuffer.h>
#include <tlVk/Shader.h>

#include <tlCore/Image.h>
#include <tlCore/Mesh.h>
#include <tlCore/Path.h>

#include <OpenEXR/ImfRgbaFile.h>
#include <OpenEXR/ImfArray.h>
#include <OpenEXR/ImfHeader.h>

#include <FL/platform.H>
#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/math.h>

#include <FL/Fl_Vk_Window.H>
#include <FL/Fl_Vk_Utils.H>

#include <chrono>
#include <iostream>
#include <memory>
#include <regex>
#include <string>
#include <unordered_map>

using namespace PXR_NS;

namespace
{
    
    UsdGeomCamera UsdGetCameraAtPath(
        const UsdStageRefPtr& stage,
        const SdfPath path)
    {
        UsdGeomCamera out;
        for (const auto& prim : stage->Traverse())
        {
            if (prim.IsA<UsdGeomCamera>())
            {
                if (prim.GetPath() == path)
                    out = UsdGeomCamera(prim);
                break;
            }
        }
        return out;
    }

    UsdGeomCamera getCamera(
        const UsdStageRefPtr& stage,
        const std::string& name = std::string())
    {
        UsdGeomCamera out;
        if (!name.empty())
        {
            out = UsdGetCameraAtPath(stage, SdfPath(name));
        }
        if (!out)
        {
            const TfToken primaryCameraName =
                UsdUtilsGetPrimaryCameraName();
            out = UsdGetCameraAtPath(
                stage, SdfPath(primaryCameraName));
        }
        if (!out)
        {
            for (const auto& prim : stage->Traverse())
            {
                if (prim.IsA<UsdGeomCamera>())
                {
                    out = UsdGeomCamera(prim);
                    break;
                }
            }
        }
        return out;
    }

    GfCamera getCameraToFrameStage(
        const UsdStagePtr& stage, UsdTimeCode timeCode,
        const TfTokenVector& includedPurposes)
    {
        GfCamera gfCamera;
        UsdGeomBBoxCache bboxCache(timeCode, includedPurposes, true);
        const GfBBox3d bbox =
            bboxCache.ComputeWorldBound(stage->GetPseudoRoot());
        const GfVec3d center = bbox.ComputeCentroid();
        const GfRange3d range = bbox.ComputeAlignedRange();
        const GfVec3d dim = range.GetSize();
        const TfToken upAxis = UsdGeomGetStageUpAxis(stage);

        GfVec2d planeCorner;
        if (upAxis == UsdGeomTokens->y)
        {
            planeCorner = GfVec2d(dim[0], dim[1]) / 2;
        }
        else
        {
            planeCorner = GfVec2d(dim[0], dim[2]) / 2;
        }
        const float planeRadius = sqrt(GfDot(planeCorner, planeCorner));

        const float halfFov =
            gfCamera.GetFieldOfView(GfCamera::FOVHorizontal) / 2.0;
        float distance = planeRadius / tan(GfDegreesToRadians(halfFov));

        if (upAxis == UsdGeomTokens->y)
        {
            distance += dim[2] / 2;
        }
        else
        {
            distance += dim[1] / 2;
        }
        distance *= 1.15;  // <--- fudge factor

        GfMatrix4d xf;
        if (upAxis == UsdGeomTokens->y)
        {
            xf.SetTranslate(center + GfVec3d(0, 0, distance));
        }
        else
        {
            xf.SetRotate(GfRotation(GfVec3d(1, 0, 0), 90));
            xf.SetTranslateOnly(center + GfVec3d(0, -distance, 0));
        }
        gfCamera.SetTransform(xf);
        return gfCamera;
    }

} // namespace


using namespace tl;


namespace tl
{
    namespace usd
    {
        struct RenderEngine::Private
        {    
            // USD information
            UsdStageRefPtr stage = nullptr;

            // Timeline
            double startTimeCode = 0.F;
            double endTimeCode = 100.F;
            double timeCodesPerSecond = 24.0F;
            UsdTimeCode time = 0;

            // Vulkan information.
            bool collectMaterials = true;
            bool collectTextures = true;
            std::unordered_map<std::string, usd::Material > materials;
            std::unordered_map<std::string, usd::ShaderTextures > textures;

            // Renderer information
            std::vector<TransparentPrimitive> transparentPrims;

            struct Stats
            {
                std::chrono::steady_clock::time_point timer;
                
                // Primitive counts
                std::size_t total  = 0;
                std::size_t opaque = 0;
                std::size_t transparent = 0;

                // Total scene triangles
                std::size_t triangles = 0;
                
                // Main Primitive types
                std::size_t meshes = 0;
                std::size_t subdivs = 0;
                std::size_t nurbs = 0;
                std::size_t nurbsCurves = 0;
                std::size_t basisCurves = 0;
                std::size_t points = 0;

                // Not common primitives
                std::size_t capsules = 0;
                std::size_t cones = 0;
                std::size_t cubes = 0;
                std::size_t cylinders = 0;
                std::size_t spheres = 0;
                
                std::size_t textures = 0;
                
                std::size_t skeletons = 0;

                void reset()
                    {
                        opaque = transparent = total = 0;
                        triangles = meshes = subdivs = nurbs = 0;
                        nurbsCurves = basisCurves = 0;
                        points = 0;
                        capsules = cones = cubes = cylinders = spheres = 0;
                        skeletons = 0;
                    }

                void print(std::ostream& o)
                    {
                        o << "---------------------------------------------------"
                          << std::endl
                          << "    Triangles = " << triangles
                          << std::endl
                          << "    Textures  = " << textures
                          << std::endl
                          << std::endl
                          << "        Total = " << total << std::endl
                          << "       Opaque = " << opaque << std::endl
                          << "  Transparent = " << transparent
                          << std::endl
                          << std::endl
                          << "       Meshes = " << meshes << std::endl
                          << "      Subdivs = " << subdivs << std::endl
                          << "Nurbs Patches = " << nurbs << std::endl
                          << " Nurbs Curves = " << nurbsCurves
                          << std::endl
                          << " Basis Curves = " << basisCurves
                          << std::endl
                          << "       Points = " << points << std::endl
                          << "    Skeletons = " << skeletons
                          << std::endl
                          << "     Capsules = " << capsules << std::endl
                          << "        Cones = " << cones << std::endl
                          << "        Cubes = " << cubes << std::endl
                          << "    Cylinders = " << cylinders << std::endl
                          << "      Spheres = " << spheres << std::endl
                          << std::endl;
                    }
            };
            Stats stats;
            
            //! tlRender context
            std::shared_ptr<system::Context> context;


            //! Offscreen renderer.
            std::shared_ptr<usd::Render> render;
    
            //! Offscreen buffer.
            std::shared_ptr<tl::vlk::OffscreenBuffer> buffer;
        };



        void RenderEngine::_init()
        {
            TLRENDER_P();
            
            p.context = system::Context::create();
        }

        RenderEngine::RenderEngine(Fl_Vk_Context& vulkanCtx) :
            ctx(vulkanCtx),
            _p(new Private)
        {
        }

        RenderEngine::~RenderEngine()
        {
        }

        std::shared_ptr<RenderEngine>
        RenderEngine::create(Fl_Vk_Context& ctx)
        {
            auto out = std::shared_ptr<RenderEngine>(new RenderEngine(ctx));
            out->_init();
            return out;
        }


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

        std::vector<UsdGeomMesh> RenderEngine::_SplitMeshBySubsets(
            const std::string& primPath,
            const UsdGeomMesh& mesh,
            const VtArray<GfVec3f>& points,
            const VtArray<int>& faceCounts,
            const VtArray<int> faceIndices,
            const std::vector<UsdGeomSubset>& subsets)
        {
            TLRENDER_P();

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
        
                UsdGeomMesh newMesh = UsdGeomMesh::Define(p.stage, newPath);
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
                UsdShadeMaterial material = GetMaterial(subsetBinding);
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

        void RenderEngine::_drawMesh(const std::string& primPath,
                                     const UsdGeomMesh& usdMesh,
                                     const math::Matrix4x4f& modelMatrix,
                                     std::string shaderId,
                                     const image::Color4f& color,
                                     UsdGeomBBoxCache& bboxCache)
        {
            TLRENDER_P();
            
            // 1. Try to get the Material bound directly to the Prim
            UsdPrim prim = usdMesh.GetPrim();  // \@todo: remove to use just prim not usdMesh
            UsdTimeCode time = p.time;
            
            // -------------------------
            // 1. VERTICES (Points)
            // -------------------------
            VtArray<GfVec3f> points;
            usdMesh.GetPointsAttr().Get(&points, time);

            // Faces vertex counts: number vertices per face.
            VtArray<int> faceVertexCounts;
            usdMesh.GetFaceVertexCountsAttr().Get(&faceVertexCounts, time);
            
            // faceVertexIndices: flat list of vertex indices for all faces
            VtArray<int> faceVertexIndices;
            usdMesh.GetFaceVertexIndicesAttr().Get(&faceVertexIndices, time);

            const TfToken materialBindFamily("materialBind");
            std::vector<UsdGeomSubset> subsets =
                UsdGeomSubset::GetGeomSubsets(usdMesh,
                                              UsdGeomTokens->face,
                                              materialBindFamily);
            // If only one subset, no need to split the mesh
            if (subsets.size() > 1)
            {
                const std::vector<UsdGeomMesh>& usdMeshes = _SplitMeshBySubsets(
                    primPath,
                    usdMesh,
                    points,
                    faceVertexCounts,
                    faceVertexIndices,
                    subsets);

                for (const auto& usdMesh : usdMeshes)
                {
                    const std::string primPath = usdMesh.GetPath().GetString();
                    _drawMesh(primPath, usdMesh, modelMatrix, shaderId, color,
                              bboxCache);
                }

                return;
            }
            
            
            std::shared_ptr<geom::TriangleMesh3> geom(new geom::TriangleMesh3);
            MeshOptimization meshOptimization;

            std::vector<usd::PrimvarAndType> primvarsAndType = usd::GetPrimvars(usdMesh.GetPrim());

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
                    std::cerr << primPath << " (among others?) " << std::endl
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
                                
                                geom->t.emplace_back(math::Vector2f(uv[0], 1.0F - uv[1]));
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
                            
                                geom->n.emplace_back(math::Vector3f(n[0], n[1], n[2]));
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
                            
                                geom->c.emplace_back(
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
            geom->v.reserve(points.size());
            for (int i = 0; i < points.size(); ++i)
            {
                const auto& p = points[i];
                geom->v.emplace_back(math::Vector3f(p[0], p[1], p[2]));
            }
            
            const bool hasUVs = !geom->t.empty();
            const bool hasNormals = !geom->n.empty();
            const bool hasColors = !geom->c.empty();

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
                    
                    geom->triangles.emplace_back(triangle);
                }
                indexOffset += vertCount;
            }

                
            //
            // Find the textures if the mesh has UVs 
            //
            usd::Material material;
            UsdShadeMaterial usdMaterial;
            std::unordered_map<int, std::shared_ptr<vlk::Texture > > textures;
            if (hasUVs)
            {
                UsdShadeMaterialBindingAPI api(prim);
                usdMaterial = usd::GetMaterial(api);
                if (!usdMaterial)
                {
                    UsdGeomImageable imageable(prim);
                    if (imageable)
                    {
                        for (const UsdGeomSubset& subset : UsdGeomSubset::GetAllGeomSubsets(imageable))
                        {
                            UsdShadeMaterialBindingAPI subsetBinding(subset.GetPrim());
                            usdMaterial = usd::GetMaterial(subsetBinding);
                            if (usdMaterial)
                                break;
                        }
                    }
                }
            }
            
            shaderId = "dummy";
            std::string materialKey = "unassigned";
            if (usdMaterial)
            {
                std::string materialKey = usdMaterial.GetPrim().GetPath().GetString();
                
                auto i = p.textures.find(materialKey);
                if (i != p.textures.end())
                {
                    textures = i->second;
                    auto j = p.materials.find(materialKey);
                    if (j != p.materials.end())
                    {
                        material = j->second;
                        shaderId = "usd";
                    }
                }
            }

            p.stats.textures = p.textures.size();
            p.stats.triangles += geom->triangles.size();

            if (!material.transparent)
            {
                // Object is opaque.  Render it without blending.
                p.stats.opaque++;

                p.render->drawMesh(*geom, meshOptimization, modelMatrix, color,
                                   shaderId, textures, material);
            }
            else
            {
                // Object is transparent.  Store it for later drawing.
                p.stats.transparent++;
                
                TransparentPrimitive object;
                object.geom = geom;
                object.optimization = meshOptimization;
                object.modelMatrix = modelMatrix;
                object.color = color;
                object.shaderId = shaderId;
                object.material = material;
                object.textures = textures;

                p.transparentPrims.emplace_back(object);
            }
        }

        void RenderEngine::draw(VkCommandBuffer cmd,
                                unsigned frameIndex,
                                unsigned renderWidth)
        {
            TLRENDER_P();

            std::cerr << "draw loop begin" << std::endl;
            p.stats.timer = std::chrono::steady_clock::now();
            
            if (!p.render)
            {
                p.render = usd::Render::create(ctx, p.context);
                if (!p.render)
                {
                    std::cerr << "Could not inititalize vulkan USD render class"
                              << std::endl;
                    return;
                }
            }

            if (p.collectMaterials)
            {
                p.collectMaterials = false;
                usd::CollectMaterials(ctx, p.stage, p.time, p.materials);
            }
            if (p.collectTextures)
            {
                p.collectTextures = false;
                std::unordered_map<std::string, std::shared_ptr<vlk::Texture > > textureCache;
                p.textures.clear();
                usd::CollectTextures(ctx, p.materials, textureCache, p.textures);
                for (auto& [_, texture] : textureCache)
                {
                    texture->transitionToShaderRead(cmd);
                }
                
            }

            std::string cameraName;
            auto usdCamera = getCamera(p.stage, cameraName);

            GfCamera gfCamera;
            if (usdCamera)
            {
                gfCamera = usdCamera.GetCamera(p.time);
            }
            else
            {
                const TfTokenVector purposes(
                    {UsdGeomTokens->default_, UsdGeomTokens->proxy});
                gfCamera = getCameraToFrameStage(p.stage, p.time, purposes);
            }
    
            float aspectRatio = gfCamera.GetAspectRatio();
            if (GfIsClose(aspectRatio, 0.F, 1e-4))
            {
                aspectRatio = 1.F;
            }
            unsigned renderHeight = renderWidth / aspectRatio;

            const GfFrustum frustum = gfCamera.GetFrustum();
            const GfVec3d cameraPos = frustum.GetPosition();
            const math::Vector3f camPos(cameraPos[0], cameraPos[1],
                                        cameraPos[2]);
    
            const GfMatrix4d& gfViewMatrix = frustum.ComputeViewMatrix();
            const GfMatrix4d& gfProjectionMatrix = frustum.ComputeProjectionMatrix();

            const GfMatrix4d& Gfmvp = gfViewMatrix * gfProjectionMatrix;
            const math::Matrix4x4f mvp(
                Gfmvp[0][0], Gfmvp[0][1], Gfmvp[0][2], Gfmvp[0][3],
                Gfmvp[1][0], Gfmvp[1][1], Gfmvp[1][2], Gfmvp[1][3],
                Gfmvp[2][0], Gfmvp[2][1], Gfmvp[2][2], Gfmvp[2][3],
                Gfmvp[3][0], Gfmvp[3][1], Gfmvp[3][2], Gfmvp[3][3]);
            const math::Matrix4x4f viewMatrix(
                gfViewMatrix[0][0], gfViewMatrix[0][1],
                gfViewMatrix[0][2], gfViewMatrix[0][3],
                gfViewMatrix[1][0], gfViewMatrix[1][1],
                gfViewMatrix[1][2], gfViewMatrix[1][3],
                gfViewMatrix[2][0], gfViewMatrix[2][1],
                gfViewMatrix[2][2], gfViewMatrix[2][3],
                gfViewMatrix[3][0], gfViewMatrix[3][1],
                gfViewMatrix[3][2], gfViewMatrix[3][3]);
    
            vlk::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.colorType = image::PixelType::RGBA_F16;
            offscreenBufferOptions.depth = vlk::OffscreenDepth::_32;
            offscreenBufferOptions.stencil = vlk::OffscreenStencil::kNone;
            offscreenBufferOptions.sampling = vlk::OffscreenSampling::kNone;
            offscreenBufferOptions.pbo = true;
            offscreenBufferOptions.storeDepth = true;
            offscreenBufferOptions.colorFilters.minify = timeline::ImageFilter::Linear;
            offscreenBufferOptions.colorFilters.magnify = timeline::ImageFilter::Linear;

            const math::Size2i renderSize(renderWidth, renderHeight);
            if (vlk::doCreate(
                    p.buffer, renderSize, offscreenBufferOptions))
            {
                p.buffer = vlk::OffscreenBuffer::create(
                    ctx, renderSize, offscreenBufferOptions);
            }

            // locale::SetAndRestore saved;
            timeline::RenderOptions renderOptions;
            renderOptions.colorBuffer = image::PixelType::RGBA_F16;
            renderOptions.clear = true;
            renderOptions.clearColor = image::Color4f(0.F, 0.F, 0.F, 0.F);

            
            p.render->begin(cmd, p.buffer, frameIndex, renderSize,
                            renderOptions);
            p.render->setViewMatrix(viewMatrix);
            p.render->applyTransforms();
            p.render->beginLoadRenderPass();
            p.render->setupViewportAndScissor();
            
            auto oldTransform = p.render->getTransform();
            p.render->setTransform(mvp);
            
            TfTokenVector purposes = {UsdGeomTokens->default_};
            UsdGeomBBoxCache bboxCache(p.time, purposes);
    
            UsdPrimRange range(p.stage->GetPseudoRoot(),
                               UsdTraverseInstanceProxies());
            GfMatrix4d matrix;
            UsdSkelCache skelCache;
            UsdGeomXformCache xformCache(p.time);
            std::string primPath;

            //
            // Stats
            //
            p.stats.reset();
            p.transparentPrims.clear();
            
            std::shared_ptr<vlk::Texture> texture;    
            for (auto it = range.begin(); it != range.end(); ++it) {

                // Check if a primitive is visible and its purpose is "render" or "default".
                if (it->IsA<UsdGeomImageable>()) {
                    UsdGeomImageable imageable(*it);
            
                    if (imageable.ComputeVisibility(p.time) ==
                        UsdGeomTokens->invisible) {
                        // If this prim is invisible, its entire subtree is invisible.
                        // Prune the traversal to skip all children.
                        it.PruneChildren();
                        continue;
                    }

                    // If purpose is not default or not render, don't use this
                    // geometry.
                    TfToken purpose = imageable.ComputePurpose();
                    if (purpose != UsdGeomTokens->default_ &&
                        purpose != UsdGeomTokens->render)
                        continue;

                }

                if (!UsdShadeMaterialBindingAPI::CanApply(*it))
                    continue;

                primPath = it->GetPath().GetString();


                matrix = xformCache.GetLocalToWorldTransform(*it);
                const math::Matrix4x4f modelMatrix(matrix[0][0], matrix[0][1],
                                                   matrix[0][2], matrix[0][3],
                                                   matrix[1][0], matrix[1][1],
                                                   matrix[1][2], matrix[1][3],
                                                   matrix[2][0], matrix[2][1],
                                                   matrix[2][2], matrix[2][3],
                                                   matrix[3][0], matrix[3][1],
                                                   matrix[3][2], matrix[3][3]);

                // std::regex re("Leg");
                //std::regex re("REye");
                //std::regex re("(?:Iris|Pupil|Sclera)");  // renders brown as it should
                // std::regex re("Sclera");
                // std::regex re("Cornea");
                // std::regex re("Pupil");  // renders black as it should
                
                // if (!std::regex_search(primPath, re))
                //     continue;
                
                // std::cout << primPath << std::endl;

                image::Color4f color(1, 1, 1);
                
                VtArray<GfVec3f> colors;
                UsdGeomGprim gprim(*it);
                if (gprim)
                    gprim.GetDisplayColorAttr().Get(&colors, p.time);

                if (colors.size() == 1)
                {
                    color.r = colors[0][0];
                    color.g = colors[0][1];
                    color.b = colors[0][2];
                    //color.a = colors[0][3];  // alpha is not used.
                }
                
                
                std::string shaderId;
                if (it->IsA<UsdGeomMesh>())
                {
                    p.stats.total++;
                    p.stats.meshes++;

                    UsdGeomMesh usdMesh = UsdGeomMesh(*it);
                    _drawMesh(primPath, usdMesh, modelMatrix, shaderId, color,
                              bboxCache);
                }
                else if (it->IsA<UsdGeomNurbsPatch>())
                {
                    p.stats.total++;
                    p.stats.nurbs++; 
                    UsdGeomNurbsPatch out = UsdGeomNurbsPatch(*it);
                }
                else if (it->IsA<UsdGeomNurbsCurves>())
                {
                    p.stats.total++;
                    p.stats.nurbsCurves++; 
                    UsdGeomNurbsCurves out = UsdGeomNurbsCurves(*it);
                }
                else if (it->IsA<UsdGeomBasisCurves>())
                {
                    p.stats.total++;
                    p.stats.basisCurves++; 
                    UsdGeomBasisCurves out = UsdGeomBasisCurves(*it);
                }
                else if (it->IsA<UsdGeomSphere>())
                {
                    p.stats.total++;
                    p.stats.spheres++;

                    UsdTimeCode time = p.time;
                    
                    UsdGeomSphere out = UsdGeomSphere(*it);
                    float radius = 1;
                    out.GetRadiusAttr().Get(&radius, time);
                    auto geom = geom::sphere(radius, 16, 16);
                    
                    std::unordered_map<int, std::shared_ptr<vlk::Texture > > textures;
                    UsdShadeMaterialBindingAPI api(*it);
                    UsdShadeMaterial material = usd::GetMaterial(api);

                    const std::string materialKey = material.GetPath().GetString();
                    auto i = p.textures.find(materialKey);
                    if (i != p.textures.end())
                    {
                        textures = i->second;
                        shaderId = "UsdShaderPreview";
                    }

                    MeshOptimization opt;
                    p.render->drawMesh(geom, opt, modelMatrix, color,
                                       shaderId, textures);
                }
                else if (it->IsA<UsdGeomCube>())
                {
                    p.stats.total++;
                    p.stats.cubes++;
                }
                else if (it->IsA<UsdGeomCylinder>())
                {
                    p.stats.total++;
                    p.stats.cylinders++;
                }
                else if (it->IsA<UsdGeomCapsule>())
                {
                    p.stats.total++;
                    p.stats.capsules++;
                }
                else if (it->IsA<UsdGeomCone>())
                {
                    p.stats.total++;
                    p.stats.cones++;
                }
            }
            
            p.render->endRenderPass();

            auto oldRenderPass = p.render->getRenderPass();
            
            p.render->createOIT();
            p.render->beginOITRenderPass();
            
            //
            // Draw transparent primitives.
            //
            for (auto& object : p.transparentPrims)
            {
                std::cerr << "one oit mesh draw" << std::endl;
                p.render->drawMeshOIT(*object.geom, object.optimization,
                                      object.modelMatrix, object.color,
                                      "usd_oit", object.textures,
                                      object.material);
                std::cerr << "one oit mesh drawn" << std::endl;
            }
            
            
            p.render->endOITRenderPass();
            p.render->end();

            p.render->setRenderPass(oldRenderPass);
            p.render->setTransform(oldTransform);

#if PRINT_STATS
            std::ostream& out = std::cout;
            p.stats.print(out);
#endif
            
            if (p.buffer)
            {
                p.buffer->transitionToShaderRead(cmd);
            }
            
            const auto now = std::chrono::steady_clock::now();
            const auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - p.stats.timer);

#if PRINT_STATS
            out << "Render time: " << ( diff.count() / 1000.F) << " seconds" << std::endl;
#endif
        }

        std::shared_ptr<vlk::OffscreenBuffer> RenderEngine::getFBO()
        {
            return _p->buffer;
        }

        void RenderEngine::setTimeCode(UsdStageRefPtr stage,
                                       const UsdTimeCode& time)
        {
            TLRENDER_P();

            if (p.stage != stage)
            {
                p.stage = stage;
                _bakeJoints();
            }
            
            p.time = time;
        }

        void RenderEngine::_bakeJoints()
        {
            TLRENDER_P();
    
            TfDiagnosticMgr::GetInstance().SetQuiet(true);

            p.startTimeCode = p.stage->GetStartTimeCode();
            p.endTimeCode   = p.stage->GetEndTimeCode();
            p.timeCodesPerSecond = p.stage->GetTimeCodesPerSecond();
    
#if BAKE_JOINTS
            std::cout << "Baking joints..." << std::endl;
            // Bake the all skeletons and bound geometry over the time range.
            // \@todo: this is done on the CPU (slow) and it uses a lot of memory.
            UsdPrimRange range(p.stage->GetPseudoRoot(),
                               UsdTraverseInstanceProxies());
            GfInterval interval(p.startTimeCode, p.endTimeCode);
            UsdSkelBakeSkinning(range, interval);
            std::cout << "Baked joints..." << std::endl;
#endif
        }

    }
}
