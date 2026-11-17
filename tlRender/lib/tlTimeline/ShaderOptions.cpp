
#include <tlTimeline/ShaderOptions.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <algorithm>

namespace tl
{
    namespace timeline
    {

        TLRENDER_ENUM_IMPL(
            Debanding, "None", "Low", "Medium", "High");
        TLRENDER_ENUM_SERIALIZE_IMPL(Debanding);

    }
}
