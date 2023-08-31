

#include <tlCore/Util.h>
#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include "mrvEdit/mrvEditMode.h"

namespace mrv
{
    using namespace tl;

    TLRENDER_ENUM_IMPL(EditMode, "None", "Timeline", "Saved", "Full");
    TLRENDER_ENUM_SERIALIZE_IMPL(EditMode);
} // namespace mrv
