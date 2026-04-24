

#include "usdgeom/pointBased.h"

#include <pxr/usd/usdGeom/mesh.h>

namespace tl
{
    namespace usdgeom
    {
        class Mesh : public PointBased
        {
        public:
            Mesh(const pxr::UsdGeomMesh& usdPrim, const pxr::UsdTimeCode time) :
                PointBased(usdPrim, time)
                {    
                    usdPrim.GetFaceVertexCountsAttr().Get(&faceVertexCounts,
                                                          time);
            
                    usdPrim.GetFaceVertexIndicesAttr().Get(&faceVertexIndices,
                                                           time);
                }

            pxr::VtArray<int> GetFaceVertexCounts()
                {
                    return faceVertexCounts;
                }
            
            pxr::VtArray<int> GetFaceVertexIndices()
                {
                    return faceVertexIndices;
                }
            
            
            std::shared_ptr<geom::TriangleMesh3> convert(usd::MeshOptimization&) override;

        protected:
            pxr::VtArray<int> faceVertexCounts;
            pxr::VtArray<int> faceVertexIndices;
        };
    }
}
