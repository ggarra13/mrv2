
#pragma once

#include <tlCore/Time.h>

#include "mrvCore/mrvUndoType.h"

namespace mrv
{
    using namespace tl;

    struct UndoElement
    {
        UndoType type;
        otime::RationalTime time;
    };
} // namespace mrv
