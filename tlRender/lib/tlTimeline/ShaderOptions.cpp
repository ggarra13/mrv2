
#include <tlTimeline/ShaderOptions.h>

namespace tl
{
    namespace timeline
    {

        TLRENDER_ENUM_IMPL(
            Debanding, "None", "Low", "Medium", "High");
        TLRENDER_ENUM_SERIALIZE_IMPL(Debanding);
        
    }
}
