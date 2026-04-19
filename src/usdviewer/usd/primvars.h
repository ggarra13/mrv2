
#include <pxr/base/tf/token.h>

#include <pxr/usd/sdf/tokens.h>

#include <pxr/usd/usdGeom/primvarsAPI.h>

namespace tl
{
    namespace usd
    {
        
        inline  pxr::TfToken ClassifyPrimvar(const pxr::UsdGeomPrimvar& pv,
                                             const pxr::TfToken& readerType)
        {
            using namespace pxr;
            
            // Primary: authored type role on the geometry
            TfToken role = pv.GetTypeName().GetRole();

            if (role == SdfValueRoleNames->TextureCoordinate)
                return TfToken("st");
            if (role == SdfValueRoleNames->Normal)
                return TfToken("normal");
            if (role == SdfValueRoleNames->Color)
                return TfToken("color");
            if (role == SdfValueRoleNames->Point)
                return TfToken("point");

            // Fallback: infer from reader type if role is untyped
            if (role.IsEmpty())
            {
                if (readerType == TfToken("UsdPrimvarReader_float2"))
                    return TfToken("st");         // Best guess for untyped float2
                if (readerType == TfToken("UsdPrimvarReader_normal3f"))
                    return TfToken("normal");
                if (readerType == TfToken("UsdPrimvarReader_float3"))
                    return TfToken("color");      // or "vector" — ambiguous
            }

            return TfToken("generic");
        }

        //! Helper struct to hold the primvar and its type.
        struct PrimvarAndType
        {
            pxr::UsdGeomPrimvar pv;
            pxr::TfToken type;
        };
        
        std::vector<PrimvarAndType> GetPrimvars(pxr::UsdPrim prim);
    }
}
