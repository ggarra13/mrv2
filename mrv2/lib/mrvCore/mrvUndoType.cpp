
#include <nlohmann/json.hpp>

#include <tlCore/Util.h>
#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include "mrvCore/mrvUndoType.h"

namespace mrv
{
    using namespace tl;

    TLRENDER_ENUM_IMPL(UndoType, "Draw", "Edit");
    TLRENDER_ENUM_SERIALIZE_IMPL(UndoType);
} // namespace mrv
