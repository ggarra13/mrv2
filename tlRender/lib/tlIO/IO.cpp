// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlIO/IO.h>

#include <sstream>

namespace tl
{
    namespace io
    {
        Options merge(const Options& a, const Options& b)
        {
            Options out = b;
            for (const auto& i : a)
            {
                out[i.first] = i.second;
            }
            return out;
        }

        io::Info merge(const io::Info& video, const io::Info& audio)
        {
            io::Info out = video;
            out.audio = audio.audio;
            out.audioTime = audio.audioTime;
            for (const auto& tag : audio.tags)
            {
                out.tags[tag.first] = tag.second;
            }
            return out;
        }

        void addVideoTags(io::Info& info)
        {
            if (!info.video.empty())
            {
                {
                    std::stringstream ss;
                    ss << info.video[0].size.w << " " << info.video[0].size.h;
                    info.tags["Video Resolution"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss.precision(2);
                    ss << std::fixed;
                    ss << info.video[0].size.pixelAspectRatio;
                    info.tags["Video Pixel Aspect Ratio"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << info.video[0].pixelType;
                    info.tags["Video Pixel Type"] = ss.str();
                }
                {
                    std::stringstream ss;
                    ss << info.video[0].videoLevels;
                    info.tags["Video Levels"] = ss.str();
                }
                if (info.videoTime.has_value())
                {
                    {
                        std::stringstream ss;
                        ss << info.videoTime->start_time().to_timecode();
                        info.tags["Video Start Time"] = ss.str();
                    }
                    {
                        std::stringstream ss;
                        ss << info.videoTime->duration().to_timecode();
                        info.tags["Video Duration"] = ss.str();
                    }
                    {
                        std::stringstream ss;
                        ss.precision(2);
                        ss << std::fixed;
                        ss << info.videoTime->start_time().rate() << " FPS";
                        info.tags["Video Speed"] = ss.str();
                    }
                }
            }
        }

        void addOtioTags(
            image::Tags& tags, const std::string& clipName,
            const OTIO_NS::RationalTime& time)
        {
            tags["otioClipName"] = clipName;

            {
                std::stringstream ss;
                ss << time;
                tags["otioClipTime"] = ss.str();
            }
        }
    } // namespace io
} // namespace tl
