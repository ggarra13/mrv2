

#pragma once

#include <nlohmann/json.hpp>

namespace mrv
{
    //! Insert data for mrv2 Network connections.
    struct InsertData
    {
        int oldIndex = 0;
        int oldTrackIndex = 0;
        int trackIndex = 0;
        int insertIndex = 0;
    };

    void to_json(nlohmann::json& j, const InsertData& value);

    void from_json(const nlohmann::json& j, InsertData& value);
} // namespace mrv
