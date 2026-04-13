
#include <pxr/usd/usdSkel/cache.h>
#include <pxr/usd/usdSkel/root.h>


namespace tl
{
    namespace usd
    {
        using namespace PXR_NS;
        
        void ProcessSkeletonRoot(UsdSkelRoot& skelRoot,
                                 UsdSkelCache& skelCache,
                                 UsdTimeCode time);
    }
}
