#include <pxr/base/vt/array.h>
#include <pxr/base/gf/vec3f.h>

#include <pxr/usd/usdGeom/mesh.h>
#include <pxr/usd/usdGeom/subset.h>

#include <pxr/usd/usd/stage.h>

namespace tl
{
    namespace usd
    {

        std::vector<pxr::UsdGeomMesh> subsetsSplit(
            pxr::UsdStageRefPtr stage,
            const pxr::UsdGeomMesh& mesh,
            const pxr::VtArray<pxr::GfVec3f>& points,
            const pxr::VtArray<int>& faceCounts,
            const pxr::VtArray<int> faceIndices,
            const std::vector<pxr::UsdGeomSubset>& subsets);
    }
}
