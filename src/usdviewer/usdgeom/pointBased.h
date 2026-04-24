

#include "usdgeom/prim.h"

#include <pxr/base/gf/vec3f.h>
#include <pxr/base/vt/array.h>

namespace tl
{
    namespace usdgeom
    {
        class PointBased : public Prim
        {
        public:
            PointBased(const pxr::UsdGeomPointBased& usdPrim,
                       const pxr::UsdTimeCode time) :
                Prim(usdPrim.GetPrim(), time)
                {
                    usdPrim.GetPointsAttr().Get(&points, time);
                }

            
            pxr::VtArray<pxr::GfVec3f> getPoints()
                {
                    return points;
                }

            
        protected:
            pxr::VtArray<pxr::GfVec3f> points;
        };
    }
}
