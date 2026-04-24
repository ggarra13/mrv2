
#include "USDMeshOptimization.h"

#include <pxr/usd/usdGeom/pointBased.h>

#include <tlCore/Mesh.h>

namespace tl
{
    namespace usdgeom
    {
        class Prim
        {
        public:
            Prim(const pxr::UsdPrim& usdPrim, pxr::UsdTimeCode t) :
                prim(usdPrim),
                time(t)
                {
                }

            virtual std::shared_ptr<geom::TriangleMesh3> convert(usd::MeshOptimization&) = 0;
            
        protected:
            pxr::UsdPrim prim;
            pxr::UsdTimeCode time;
        };
    }
}
