// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "mrvOptions/mrvTimelineItemOptions.h"

namespace tl
{
#ifdef OPENGL_BACKEND
    namespace timelineui
    {
        void to_json(nlohmann::json& j, const ItemOptions& value)
        {
            j["inputEnabled"] = value.inputEnabled;
            j["editAssociatedClips"] = value.editAssociatedClips;
        }

        void from_json(const nlohmann::json& j, ItemOptions& value)
        {
            j.at("inputEnabled").get_to(value.inputEnabled);
            j.at("editAssociatedClips").get_to(value.editAssociatedClips);
        }
        
        void to_json(nlohmann::json& j, const DisplayOptions& value)
        {
            j["trackInfo"] = value.trackInfo;
            j["clipInfo"] = value.clipInfo;
            j["thumbnails"] = value.thumbnails;
            j["thumbnailHeight"] = value.thumbnailHeight;
            j["waveformWidth"] = value.waveformHeight;
            j["waveformHeight"] = value.waveformHeight;
            j["transitions"] = value.transitions;
            j["markers"] = value.markers;
        }

        void from_json(const nlohmann::json& j, DisplayOptions& value)
        {
            j.at("trackInfo").get_to(value.trackInfo);
            j.at("clipInfo").get_to(value.clipInfo);
            j.at("thumbnails").get_to(value.thumbnails);
            j.at("thumbnailHeight").get_to(value.thumbnailHeight);
            j.at("waveformWidth").get_to(value.waveformWidth);
            j.at("waveformHeight").get_to(value.waveformHeight);
            j.at("transitions").get_to(value.transitions);
            j.at("markers").get_to(value.markers);
        }
    } // namespace timelineui
#endif

#ifdef VULKAN_BACKEND
    namespace timelineui_vk
    {
        void to_json(nlohmann::json& j, const ItemOptions& value)
        {
            j["inputEnabled"] = value.inputEnabled;
            j["editAssociatedClips"] = value.editAssociatedClips;
        }

        void from_json(const nlohmann::json& j, ItemOptions& value)
        {
            j.at("inputEnabled").get_to(value.inputEnabled);
            j.at("editAssociatedClips").get_to(value.editAssociatedClips);
        }
        
        void to_json(nlohmann::json& j, const DisplayOptions& value)
        {
            j["trackInfo"] = value.trackInfo;
            j["clipInfo"] = value.clipInfo;
            j["thumbnails"] = value.thumbnails;
            j["thumbnailHeight"] = value.thumbnailHeight;
            j["waveformWidth"] = value.waveformHeight;
            j["waveformHeight"] = value.waveformHeight;
            j["transitions"] = value.transitions;
            j["markers"] = value.markers;
        }

        void from_json(const nlohmann::json& j, DisplayOptions& value)
        {
            j.at("trackInfo").get_to(value.trackInfo);
            j.at("clipInfo").get_to(value.clipInfo);
            j.at("thumbnails").get_to(value.thumbnails);
            j.at("thumbnailHeight").get_to(value.thumbnailHeight);
            j.at("waveformWidth").get_to(value.waveformWidth);
            j.at("waveformHeight").get_to(value.waveformHeight);
            j.at("transitions").get_to(value.transitions);
            j.at("markers").get_to(value.markers);
        }
    } // namespace timelineui_vk
#endif
    
} // namespace tl
