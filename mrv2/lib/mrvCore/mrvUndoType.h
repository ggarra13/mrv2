
#pragma once

namespace mrv
{

    enum class UndoType {
        Draw,
        Edit,

        Count,
        First = Draw,
    };

    TLRENDER_ENUM(UndoType);
    TLRENDER_ENUM_SERIALIZE(UndoType);
} // namespace mrv
