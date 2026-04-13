#include <USDProcessSkeletonRoot.h>

// Primitive types
#include <pxr/usd/usdGeom/basisCurves.h>
#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/nurbsCurves.h>
#include <pxr/usd/usdGeom/nurbsPatch.h>
#include <pxr/usd/usdGeom/xformCache.h>

#include <pxr/usd/usdSkel/animQuery.h>
#include <pxr/usd/usdSkel/bindingAPI.h>
#include <pxr/usd/usdSkel/skeletonQuery.h>
#include <pxr/usd/usdSkel/utils.h>

#include <iostream>

namespace tl
{
    namespace usd
    {
// ─────────────────────────────────────────────────────────────────────────────
// Apply Linear Blend Skinning on the CPU.
//
// Formula per vertex v_i:
//   v_i' = Σ_j  weight[i][j] * skinMtx[j] * v_i
//
// where skinMtx[j] = worldAnim[j]  *  inv(worldRest[j])  *  geomBindXform
// ─────────────────────────────────────────────────────────────────────────────
        VtVec3fArray ApplyLBS(
            const VtVec3fArray&    restPoints,      // mesh points in geom-local space
            const VtIntArray&      jointIndices,    // flat list, numInfluences per vertex
            const VtFloatArray&    jointWeights,    // same layout as jointIndices
            int                    numInfluences,   // influences per vertex
            const VtMatrix4dArray& skinningMtx)    // one 4×4 matrix per joint
        {
            const size_t numVerts = restPoints.size();
            VtVec3fArray skinnedPoints(numVerts, GfVec3f(0.f));

            for (size_t vi = 0; vi < numVerts; ++vi) {
                const GfVec3f& p = restPoints[vi];

                GfVec3f blended(0.f);
                for (int inf = 0; inf < numInfluences; ++inf) {
                    const int   base   = static_cast<int>(vi) * numInfluences + inf;
                    const int   joint  = jointIndices[base];
                    const float weight = jointWeights[base];

                    if (weight == 0.f || joint < 0) continue;

                    // GfMatrix4d::Transform() applies the 4×4 as a point transform
                    const GfVec3f tp = GfVec3f(skinningMtx[joint].Transform(GfVec3d(p)));
                    blended += tp * weight;
                }
                skinnedPoints[vi] = blended;
            }
            return skinnedPoints;
        }

// ─────────────────────────────────────────────────────────────────────────────
// Build per-joint skinning matrices.
//
//   skinMtx[j] = worldAnim[j]  ×  inv(worldRest[j])  ×  geomBind
//
// OpenUSD provides UsdSkelUtils::ComputeSkinningTransforms() which does exactly
// this, but we replicate it manually here to show each step.
// ─────────────────────────────────────────────────────────────────────────────
        bool BuildSkinningMatrices(
            const UsdSkelSkeletonQuery& skelQuery,
            const GfMatrix4d&           geomBindXform,   // skinningQuery.GetGeomBindTransform()
            UsdTimeCode                 time,
            VtMatrix4dArray&            outSkinMtx)
        {
            // 1) World-space joint transforms at the current animation time
            UsdGeomXformCache xfCache(time);
            VtMatrix4dArray worldAnimXforms;
            if (!skelQuery.ComputeJointWorldTransforms(&worldAnimXforms, &xfCache)) {
                TF_WARN("Failed to compute animated world joint transforms");
                return false;
            }

            // 2) World-space rest transforms (bind pose)
            VtMatrix4dArray worldRestXforms;
            if (!skelQuery.ComputeJointWorldTransforms(&worldRestXforms,
                                                       &xfCache, true)) {
                TF_WARN("Failed to compute rest world joint transforms");
                return false;
            }

            const size_t numJoints = worldAnimXforms.size();
            outSkinMtx.resize(numJoints);

            for (size_t j = 0; j < numJoints; ++j) {
                // inv(restWorld) converts a vertex from world-rest into joint-local space
                GfMatrix4d invRest = worldRestXforms[j].GetInverse();

                // Full matrix: geomBind → rest-world-space → joint-delta → anim-world
                outSkinMtx[j] = geomBindXform * invRest * worldAnimXforms[j];
            }
            return true;
        }


        void ProcessSkeletonRoot(UsdSkelRoot& skelRoot,
                                 UsdSkelCache& skelCache,
                                 UsdTimeCode time)
        {
#if 0
            skelCache.Populate(skelRoot,
                               UsdTraverseInstanceProxies());
    
            // Collect all skel bindings under this root
            std::vector<UsdSkelBinding> bindings;
            skelCache.ComputeSkelBindings(skelRoot, &bindings,
                                          UsdTraverseInstanceProxies());

            for (const UsdSkelBinding& binding : bindings) {

                // ── Skeleton query ──────────────────────────────────────────
                const UsdSkelSkeleton& skel = binding.GetSkeleton();
                UsdSkelSkeletonQuery skelQuery = skelCache.GetSkelQuery(skel);
                if (!skelQuery) continue;

                // ── Iterate bound skinnable prims ───────────────────────────
                for (const UsdSkelSkinningQuery& skinQuery
                         : binding.GetSkinningTargets()) {

                    UsdPrim skinPrim = skinQuery.GetPrim();
                    
                    // Only handle polygon meshes in this example
                    UsdGeomMesh mesh(skinPrim);
                    if (!mesh) continue;

                    std::cout << "  Mesh: " << mesh.GetPath() << "\n";

                    // ── 1. Read rest (unskinned) point positions ─────────────────
                    VtVec3fArray restPoints;
                    mesh.GetPointsAttr().Get(&restPoints, UsdTimeCode::Default());
                    if (restPoints.empty()) {
                        TF_WARN("Mesh has no points");
                        continue;
                    }

                    // ── 2. Read joint indices & weights ─────────────────────────
                    //
                    // These primvars are on the mesh (not the skeleton).
                    // The interpolation is typically "vertex" (one entry per point)
                    // and the elementSize gives the number of influences per vertex.
                    VtIntArray   jointIndices;
                    VtFloatArray jointWeights;
                    int numInfluences = 0;

                    if (!skinQuery.ComputeJointInfluences(&jointIndices,
                                                          &jointWeights)) {
                        TF_WARN("Failed to read joint influences");
                        continue;
                    }

                    // elementSize == influences-per-vertex
                    numInfluences = skinQuery.GetNumInfluencesPerComponent();
                    std::cout << "    influences/vertex: " << numInfluences << "\n";
                    std::cout << "    vertices:          " << restPoints.size() << "\n";

                    // ── 3. Get geom-bind transform ───────────────────────────────
                    //
                    // This moves the mesh from its local space into the world space
                    // that the skeleton's rest pose was authored in.
                    GfMatrix4d geomBind = skinQuery.GetGeomBindTransform(time);

                    // ── 4. Build per-joint skinning matrices ─────────────────────
                    VtMatrix4dArray skinMtx;
                    if (!BuildSkinningMatrices(skelQuery, geomBind, time,
                                               skinMtx)) {
                        continue;
                    }

                    // ── 5. (Optional) Use OpenUSD's built-in helper instead ──────
                    // This function does not exist!
                    //
                    // UsdSkelUtils::ComputeSkinningTransforms() is equivalent to
                    // BuildSkinningMatrices() above. You can swap the call like so:
                    //
                    
                    // VtMatrix4dArray skinMtx;
                    // UsdSkelUtils::ComputeSkinningTransforms(
                    //     skelQuery, skinQuery, time, &skinMtx);

                    // ── 6. Apply Linear Blend Skinning ───────────────────────────
                    // ComputeSkinningTransforms lives on UsdSkelSkeletonQuery, not in utils.h
#if 1
                    VtMatrix4dArray skinningMtx;
                    skelQuery.ComputeSkinningTransforms(&skinningMtx, time);

                    // Then feed it into the free function from utils.h
                    VtVec3fArray skinnedPoints = restPoints;
                    UsdSkelSkinPointsLBS(
                        skinQuery.GetGeomBindTransform(time),
                        skinningMtx,
                        jointIndices,
                        jointWeights,
                        skinQuery.GetNumInfluencesPerComponent(),
                        &skinnedPoints);

#else
                    // This crashes AND does not deform correctly.
                    VtVec3fArray skinnedPoints =
                        ApplyLBS(restPoints, jointIndices, jointWeights,
                                 numInfluences, skinMtx);
#endif
                    
                    // ── 7. Use the result ──────────────────────────────
                    mesh.GetPointsAttr().Set(skinnedPoints, time);
                }
            }
#endif
        }
    }
}
