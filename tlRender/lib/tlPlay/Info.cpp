// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlay/Info.h>

#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play
    {
        std::string infoLabel(const file::Path& path, const io::Info& info)
        {
            std::vector<std::string> s;
            s.push_back(string::elide(path.get(-1, file::PathType::FileName)));
            if (!info.video.empty())
            {
                s.push_back(std::string(string::Format("V: {0}, {1}")
                                            .arg(info.video[0].size)
                                            .arg(info.video[0].pixelType)));
            }
            if (info.audio.isValid())
            {
                s.push_back(
                    std::string(string::Format("A: {0}, {1}, {2}kHz")
                                    .arg(info.audio.channelCount)
                                    .arg(info.audio.dataType)
                                    .arg(info.audio.sampleRate / 1000)));
            }
            return string::join(s, ", ");
        }

        std::string infoToolTip(const file::Path& path, const io::Info& info)
        {
            std::vector<std::string> t;
            t.push_back(path.get());
            if (!info.video.empty())
            {
                t.push_back(std::string(string::Format("Video: {0}, {1}")
                                            .arg(info.video[0].size)
                                            .arg(info.video[0].pixelType)));
            }
            if (info.audio.isValid())
            {
                t.push_back(std::string(
                    string::Format("Audio: {0} {1}, {2}, {3}kHz")
                        .arg(info.audio.channelCount)
                        .arg(
                            1 == info.audio.channelCount ? "channel"
                                                         : "channels")
                        .arg(info.audio.dataType)
                        .arg(info.audio.sampleRate / 1000)));
            }
            return string::join(t, "\n");
        }
    } // namespace play
} // namespace tl
