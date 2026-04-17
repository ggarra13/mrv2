
#define PRINT_STATS 0
#define BAKE_JOINTS 1

// #include "USDProcessSkeletonRoot.h"  // \@todo: do deformation in compute shader
#include "USDCollectTextures.h"
#include "USDGetMaterials.h"
#include "USDRender.h"
#include "USDRenderShadersBinary.h"
#include "USDTextureSlots.h"

#include "USDRenderEngine.h"

#include <tlCore/Context.h>

#include <pxr/pxr.h>

// math primitives
#include <pxr/base/gf/camera.h>
#include <pxr/base/gf/frustum.h>
#include <pxr/base/gf/matrix4d.h>
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

// Primitive types
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/bboxCache.h>
#include <pxr/usd/usdGeom/camera.h>
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

#include <memory>
#include <iostream>
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

            // Current time
            UsdTimeCode time = 0;

            // Vulkan information.
            bool collectMaterials = true;
            bool collectTextures = true;
            std::unordered_map<std::string, usd::Material > materials;
            std::unordered_map<std::string, usd::ShaderTextures > textures;

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

                UsdShadeMaterial material =
                    subsetBinding.ComputeBoundMaterial();

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

                    UsdShadeMaterial meshMaterial =
                        meshBinding.ComputeBoundMaterial();

                    if (meshMaterial)
                    {
                        UsdShadeMaterialBindingAPI newBinding =
                            UsdShadeMaterialBindingAPI::Apply(newMesh.GetPrim());
                        newBinding.Bind(meshMaterial);
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
                                     std::string shaderName,
                                     const image::Color4f& color)
        {
            TLRENDER_P();
            
            // -------------------------
            // 1. VERTICES (Points)
            // -------------------------
            VtArray<GfVec3f> points;
            usdMesh.GetPointsAttr().Get(&points, p.time);

            // Faces vertex counts: number vertices per face.
            VtArray<int> faceVertexCounts;
            usdMesh.GetFaceVertexCountsAttr().Get(&faceVertexCounts, p.time);
            
            // faceVertexIndices: flat list of vertex indices for all faces
            VtArray<int> faceVertexIndices;
            usdMesh.GetFaceVertexIndicesAttr().Get(&faceVertexIndices, p.time);
    
            const TfToken materialBindFamily("materialBind");
            std::vector<UsdGeomSubset> subsets =
                UsdGeomSubset::GetGeomSubsets(usdMesh,
                                              UsdGeomTokens->face,
                                              materialBindFamily);
            if (subsets.size() > 1)
            {
                const std::vector<UsdGeomMesh>& meshes = _SplitMeshBySubsets(
                    primPath,
                    usdMesh,
                    points,
                    faceVertexCounts,
                    faceVertexIndices,
                    subsets);

                for (const auto& mesh : meshes)
                {
                    std::string primPath = mesh.GetPath().GetString();
                    _drawMesh(primPath, mesh, modelMatrix, shaderName, color);
                }

                return;
            }

            
            geom::TriangleMesh3 geom;

            // Get points.
            geom.v.reserve(points.size());
            for (int i = 0; i < points.size(); ++i)
            {
                const auto& p = points[i];
                geom.v.push_back(math::Vector3f(p[0], p[1], p[2]));
            }

            // Get Normals if any.
            UsdGeomPrimvarsAPI primvarsAPI(usdMesh);
            UsdGeomPrimvar normalsPrimvar = primvarsAPI.GetPrimvar(UsdGeomTokens->normals);
            if (normalsPrimvar.IsDefined()) {
                
                VtArray<GfVec3f> normals;
                normalsPrimvar.ComputeFlattened(&normals, p.time);
                
                TfToken interp = normalsPrimvar.GetInterpolation();
                // std::cout << "Normals count: " << normals.size()
                //           << "  interpolation: " << interp << "\n";

                // walks faceVertexIndices for faceVarying
                int faceCornerIdx = 0; 

                for (size_t faceIdx = 0; faceIdx < faceVertexCounts.size();
                     ++faceIdx) {
                    int vertCount = faceVertexCounts[faceIdx];

                    for (int i = 0; i < vertCount; ++i) {
                        int pointIdx = faceVertexIndices[faceCornerIdx + i];
                        GfVec3f n;

                        if (interp == UsdGeomTokens->faceVarying) {
                            // One normal per face-corner, in face-winding order
                            n = normals[faceCornerIdx + i];
                        } else if (interp == UsdGeomTokens->vertex) {
                            // Normal indexed the same way as points
                            n = normals[pointIdx];
                        } else if (interp == UsdGeomTokens->uniform) {
                            // One normal for the entire face
                            n = normals[faceIdx];
                        } else {
                            // constant — one normal for the whole mesh
                            n = normals[0];
                        }

                        geom.n.push_back(math::Vector3f(n[0], n[1], n[2]));
                    }
                    faceCornerIdx += vertCount;
                }
            }

            //
            // Get UVs (st)
            //
            UsdGeomPrimvar st = primvarsAPI.GetPrimvar(TfToken("st"));
            if (st.IsDefined()) {
                
                VtArray<GfVec2f> values;
                if (st.Get(&values))
                {
                    const TfToken interp = st.GetInterpolation();
                    // std::cerr << primPath << std::endl;
                    // std::cerr << "\tSTs found=" << values.size()
                    //           << " interpolation=" << interp << std::endl;

                    if (st.IsIndexed())
                    {
                        VtArray<int> indices;
                        st.GetIndices(&indices);

                        std::vector<GfVec2f> expanded(indices.size());
                        for (size_t i = 0; i < indices.size(); ++i)
                            expanded[i] = values[indices[i]];

                        int faceCornerIdx = 0; 

                        for (size_t faceIdx = 0; faceIdx < faceVertexCounts.size();
                             ++faceIdx) {
                            int vertCount = faceVertexCounts[faceIdx];
                                
                            for (int i = 0; i < vertCount; ++i) {
                                int pointIdx = faceVertexIndices[faceCornerIdx + i];
                                GfVec2f uv;

                                if (interp == UsdGeomTokens->faceVarying) {
                                    // One uv per face-corner, in face-winding order
                                    uv = expanded[faceCornerIdx + i];
                                } else if (interp == UsdGeomTokens->vertex ||
                                           interp == UsdGeomTokens->varying) {
                                    // UV indexed the same way as points
                                    uv = expanded[pointIdx];
                                } else if (interp == UsdGeomTokens->uniform) {
                                    // One UV for the entire face
                                    uv = expanded[faceIdx];
                                } else {
                                    // constant — one UV for the whole mesh
                                    uv = expanded[0];
                                }

                                geom.t.push_back(math::Vector2f(uv[0], 1.0F - uv[1]));
                            }
                            faceCornerIdx += vertCount;
                        }
                    }
                    else
                    {
                        int faceCornerIdx = 0;

                        for (size_t faceIdx = 0; faceIdx < faceVertexCounts.size();
                             ++faceIdx) {
                            int vertCount = faceVertexCounts[faceIdx];
                                
                            for (int i = 0; i < vertCount; ++i) {
                                // Linear index of the current corner
                                int currentCorner = faceCornerIdx + i;
                                int pointIdx = faceVertexIndices[currentCorner];
                                GfVec2f uv;

                                if (interp == UsdGeomTokens->faceVarying) {
                                    // One uv per face-corner, in face-winding order
                                    uv = values[currentCorner];
                                } else if (interp == UsdGeomTokens->vertex ||
                                           interp == UsdGeomTokens->varying) {
                                    // UV indexed the same way as points
                                    uv = values[pointIdx];
                                } else if (interp == UsdGeomTokens->uniform) {
                                    // One UV for the entire face
                                    uv = values[faceIdx];
                                } else {
                                    // constant — one UV for the whole mesh
                                    uv = values[0];
                                }

                                geom.t.push_back(math::Vector2f(uv[0], 1.0f - uv[1]));
                            }
                            faceCornerIdx += vertCount;
                        }
                    }
                }
            }
            
            // Get triangles.
            int indexOffset = 0;
            const bool hasNormals = !geom.n.empty();
            const bool hasUVs = !geom.t.empty();
            
            std::vector<math::Vector3f> normals(points.size());
            for (int vertCount : faceVertexCounts)
            {
                // Fan triangulation: anchor at faceVertexIndices[indexOffset]
                // e.g. a quad [A, B, C, D] → (A,B,C), (A,C,D)
                for (int i = 1; i < vertCount - 1; ++i)
                {
                    geom::Triangle3 triangle;
                    const int i0 = faceVertexIndices[indexOffset];
                    const int i1 = faceVertexIndices[indexOffset + i];
                    const int i2 = faceVertexIndices[indexOffset + i + 1];
                    
                    triangle.v[0].v = i0 + 1;
                    triangle.v[1].v = i1 + 1;
                    triangle.v[2].v = i2 + 1;

                    if (hasNormals)
                    {
                        triangle.v[0].n = indexOffset + 1;
                        triangle.v[1].n = indexOffset + i + 1;
                        triangle.v[2].n = indexOffset + i + 2;
                    }
                    
                    if (hasUVs)
                    {
                        triangle.v[0].t = indexOffset + 1;
                        triangle.v[1].t = indexOffset + i + 1;
                        triangle.v[2].t = indexOffset + i + 2;
                    }
                    
                    geom.triangles.push_back(triangle);
                }
                indexOffset += vertCount;
            }

            //
            // Find the textures if the mesh has UVs 
            //
            usd::Material material;
            std::unordered_map<int, std::shared_ptr<vlk::Texture > > textures;
            if (hasUVs)
            {
                // 1. Try to get the Material bound directly to the Prim
                UsdPrim prim = usdMesh.GetPrim();
                UsdShadeMaterialBindingAPI bindingApi(prim);
                UsdShadeMaterial usdMaterial = bindingApi.ComputeBoundMaterial();

                if (!usdMaterial)
                {
                    UsdGeomImageable imageable(prim);
                    if (imageable)
                    {
                        for (const UsdGeomSubset& subset : UsdGeomSubset::GetAllGeomSubsets(imageable))
                        {
                            UsdShadeMaterialBindingAPI subsetBinding(subset.GetPrim());
                            usdMaterial = subsetBinding.ComputeBoundMaterial();
                            if (usdMaterial)
                                break;
                        }
                    }
                }
        
                std::string materialPath = usdMaterial.GetPath().GetString();
        
                auto i = p.textures.find(materialPath);
                if (i != p.textures.end())
                {
                    textures = i->second;
                    auto j = p.materials.find(materialPath);
                    if (j != p.materials.end())
                    {
                        material = j->second;
                    }
                    shaderName = "UsdPreviewSurface";
                }
            }
            else
            {
                shaderName = "dummy";
            }


            
            
            p.render->draw3DMesh(geom, modelMatrix, color, shaderName,
                                 textures, material);
        }

        void RenderEngine::draw(VkCommandBuffer cmd,
                                unsigned frameIndex,
                                unsigned renderWidth)
        {
            TLRENDER_P();
            
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
                usd::CollectMaterials(ctx, p.stage, p.materials);
            }
            if (p.collectTextures)
            {
                p.collectTextures = false;
                std::unordered_map<std::string, std::shared_ptr<vlk::Texture > > textureCache;
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
    
            const GfMatrix4d& viewMatrix = frustum.ComputeViewMatrix();
            const GfMatrix4d& projectionMatrix = frustum.ComputeProjectionMatrix();

            const GfMatrix4d& Gfmvp = viewMatrix * projectionMatrix;
            const math::Matrix4x4f mvp(
                Gfmvp[0][0], Gfmvp[0][1], Gfmvp[0][2], Gfmvp[0][3],
                Gfmvp[1][0], Gfmvp[1][1], Gfmvp[1][2], Gfmvp[1][3],
                Gfmvp[2][0], Gfmvp[2][1], Gfmvp[2][2], Gfmvp[2][3],
                Gfmvp[3][0], Gfmvp[3][1], Gfmvp[3][2], Gfmvp[3][3]);
    
            vlk::OffscreenBufferOptions offscreenBufferOptions;
            offscreenBufferOptions.colorType = image::PixelType::RGBA_F16;
            offscreenBufferOptions.depth = vlk::OffscreenDepth::_32;
            offscreenBufferOptions.stencil = vlk::OffscreenStencil::kNone;
            offscreenBufferOptions.sampling = vlk::OffscreenSampling::_4;
            offscreenBufferOptions.pbo = true;
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
            renderOptions.clearColor = image::Color4f(0.0F, 0.0F, 0.0F, 0.F);
    
            p.render->begin(cmd, p.buffer, frameIndex, renderSize,
                            renderOptions);
            const math::Vector3f camPos = math::Vector3f(cameraPos[0],
                                                         cameraPos[1],
                                                         cameraPos[2]);
            p.render->setCameraWorldPosition(camPos);
            p.render->applyTransforms();
            p.render->beginLoadRenderPass();
            p.render->setupViewportAndScissor();
            
            auto oldTransform = p.render->getTransform();
            p.render->setTransform(mvp);


    
            UsdPrimRange range(p.stage->GetPseudoRoot(),
                               UsdTraverseInstanceProxies());
            GfMatrix4d matrix;
            UsdSkelCache skelCache;
            UsdGeomXformCache xformCache(p.time);
            std::string primPath;

            //
            // Stats
            //
            std::size_t numMeshes = 0;
            std::size_t numSubdivs = 0;
            std::size_t numNurbsPatches = 0;
            std::size_t numNurbsCurves = 0;
            std::size_t numBasisCurves = 0;
            std::size_t numSkeletons = 0;
            std::size_t numPoints = 0;
            std::size_t numSpheres = 0;
    
            std::shared_ptr<vlk::Texture> texture;    
            for (auto it = range.begin(); it != range.end(); ++it) {
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
        
                VtArray<GfVec3f> colors;
                UsdGeomGprim gprim(*it);
                if (gprim)
                    gprim.GetDisplayColorAttr().Get(&colors);

                image::Color4f color(0.5F, 0.5F, 0.5F);

                if (colors.size() == 1)
                {
                    color.r = colors[0][0];
                    color.g = colors[0][1];
                    color.b = colors[0][2];
                }
                std::string shaderName;
                if (colors.size() == 0)
                {
                    UsdShadeMaterialBindingAPI bindingApi(*it);
                    UsdShadeMaterial material = bindingApi.ComputeBoundMaterial();
                    
                    if (material)
                    {
                        UsdShadeShader shader = material.ComputeSurfaceSource();
                        TfToken shaderId;
                        if (shader)
                        {
                            shader.GetIdAttr().Get(&shaderId);
                            shaderName = shaderId.GetString();
                        
                            GfVec3f diffuse;
                            UsdShadeInput diffuseColorInput = shader.GetInput(TfToken("diffuseColor"));
                            if (diffuseColorInput)
                            {
                                diffuseColorInput.Get(&diffuse);
                                color.r = diffuse[0];
                                color.g = diffuse[1];
                                color.b = diffuse[2];
                            }
                        }
                        else
                        {
                            std::cerr << "no shader for material " << std::endl;
                        }
                    }
                }
        
                if (it->IsA<UsdSkelRoot>())
                {
                    ++numSkeletons;

                    //UsdSkelRoot skelRoot(*it);
                    //usd::ProcessSkeletonRoot(skelRoot, skelCache, p.time);
                }
                else if (it->IsA<UsdGeomMesh>())
                {
                    ++numMeshes;

                    UsdGeomMesh usdMesh = UsdGeomMesh(*it);
                    _drawMesh(primPath, usdMesh, modelMatrix, shaderName, color);
                }
                else if (it->IsA<UsdGeomNurbsPatch>())
                {
                    ++numNurbsPatches; 
                    UsdGeomNurbsPatch out = UsdGeomNurbsPatch(*it);
                }
                else if (it->IsA<UsdGeomNurbsCurves>())
                {
                    ++numNurbsCurves; 
                    UsdGeomNurbsCurves out = UsdGeomNurbsCurves(*it);
                }
                else if (it->IsA<UsdGeomBasisCurves>())
                {
                    ++numBasisCurves; 
                    UsdGeomBasisCurves out = UsdGeomBasisCurves(*it);
                }
                else if (it->IsA<UsdGeomSphere>())
                {
                    ++numSpheres; 
                    UsdGeomSphere out = UsdGeomSphere(*it);
                    float radius = 1;
                    out.GetRadiusAttr().Get(&radius, p.time);
                    auto geom = geom::sphere(radius, 16, 16);
                    const bool hasUVs = !geom.t.empty();
                    
                    std::unordered_map<int, std::shared_ptr<vlk::Texture > > textures;
                    if (hasUVs)
                    {
                        UsdShadeMaterialBindingAPI bindingApi(*it);
                        UsdShadeMaterial material = bindingApi.ComputeBoundMaterial();

                        const std::string materialPath = material.GetPath().GetString();
        
                        auto i = p.textures.find(materialPath);
                        if (i != p.textures.end())
                        {
                            textures = i->second;
                        }
                    }
            
                    p.render->draw3DMesh(geom, modelMatrix, color,
                                         shaderName, textures);
                }
                // \@todo: cylinder
            }

            p.render->endRenderPass();
            p.render->end();

#if PRINT_STATS
            std::cout << "       Meshes = " << numMeshes << std::endl
                      << "      Subdivs = " << numSubdivs << std::endl
                      << "Nurbs Patches = " << numNurbsPatches << std::endl
                      << " Nurbs Curves = " << numNurbsCurves << std::endl
                      << " Basis Curves = " << numBasisCurves << std::endl
                      << "       Points = " << numPoints << std::endl
                      << "    Skeletons = " << numSkeletons << std::endl
                      << "      Spheres = " << numSpheres << std::endl;
#endif
    
            p.render->setTransform(oldTransform);
    
            if (p.buffer)
            {
                p.buffer->transitionToShaderRead(cmd);
            }        
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
