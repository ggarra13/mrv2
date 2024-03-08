// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/Time.h>

#include "mrvDraw/Annotation.h"
#include "mrvNetwork/mrvFilePath.h"
#include "mrvApp/mrvFilesModel.h"

namespace mrv
{
    void to_json(nlohmann::json& j, const FilesModelItem& value)
    {
        j["path"] = value.path;
        j["audioPath"] = value.audioPath;
        j["timeRange"] = value.timeRange;
        j["speed"] = value.speed;
        j["playback"] = value.playback;
        j["loop"] = value.loop;
        j["currentTime"] = value.currentTime;
        j["inOutRange"] = value.inOutRange;
        j["videoLayer"] = value.videoLayer;
        j["volume"] = value.volume;
        j["mute"] = value.mute;
        j["audioOffset"] = value.audioOffset;
        j["ocioIcs"] = value.ocioIcs;
        std::vector< draw::Annotation > annotations;
        for (const auto& annotation : value.annotations)
        {
            annotations.push_back(*annotation.get());
        }
        j["annotations"] = annotations;
    }

    void from_json(const nlohmann::json& j, FilesModelItem& value)
    {
        using namespace mrv::draw;
        j.at("path").get_to(value.path);
        j.at("audioPath").get_to(value.audioPath);
        j.at("timeRange").get_to(value.timeRange);
        j.at("speed").get_to(value.speed);
        if (j["playback"].type() == nlohmann::json::value_t::string)
        {
            j.at("playback").get_to(value.playback);
        }
        else
        {
            int v;
            j.at("playback").get_to(v);
            value.playback = static_cast<timeline::Playback>(v);
        }
        if (j["loop"].type() == nlohmann::json::value_t::string)
        {
            j.at("loop").get_to(value.loop);
        }
        else
        {
            int v;
            j.at("loop").get_to(v);
            value.loop = static_cast<timeline::Loop>(v);
        }
        j.at("currentTime").get_to(value.currentTime);
        j.at("inOutRange").get_to(value.inOutRange);
        j.at("videoLayer").get_to(value.videoLayer);
        j.at("volume").get_to(value.volume);
        j.at("mute").get_to(value.mute);
        j.at("audioOffset").get_to(value.audioOffset);
        if (j.contains("ocioIcs"))
            j.at("ocioIcs").get_to(value.ocioIcs);
        const nlohmann::json& annotations = j["annotations"];
        for (const auto& annotation : annotations)
        {
            std::shared_ptr< Annotation > tmp = messageToAnnotation(annotation);
            value.annotations.push_back(tmp);
        }
    }
} // namespace mrv
