// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlTimelineUI/IItem.h>

namespace tl
{
    namespace timelineui
    {
        void to_json(nlohmann::json& j, const ItemOptions& value)
        {
            j["editAssociatedClips"] = value.editAssociatedClips;
            j["thumbnails"] = value.thumbnails;
            j["thumbnailHeight"] = value.thumbnailHeight;
            j["waveformWidth"] = value.waveformHeight;
            j["waveformHeight"] = value.waveformHeight;
            j["showTransitions"] = value.showTransitions;
            j["showMarkers"] = value.showMarkers;
        }

        void from_json(const nlohmann::json& j, ItemOptions& value)
        {
            j.at("editAssociatedClips").get_to(value.editAssociatedClips);
            j.at("thumbnails").get_to(value.thumbnails);
            j.at("thumbnailHeight").get_to(value.thumbnailHeight);
            j.at("waveformWidth").get_to(value.waveformWidth);
            j.at("waveformHeight").get_to(value.waveformHeight);
            j.at("showTransitions").get_to(value.showTransitions);
            j.at("showMarkers").get_to(value.showMarkers);
        }
    }; // namespace timelineui

} // namespace tl
