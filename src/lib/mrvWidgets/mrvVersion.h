// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

#include "mrvWidgets/mrvTextBrowser.h"

namespace mrv
{

    void ffmpeg_formats(mrv::TextBrowser* b);
    void ffmpeg_video_codecs(mrv::TextBrowser* b);
    void ffmpeg_audio_codecs(mrv::TextBrowser* b);
    void ffmpeg_subtitle_codecs(mrv::TextBrowser* b);
    void ffmpeg_protocols(mrv::TextBrowser* b);
    void ffmpeg_codec_information(mrv::TextBrowser* b);

    const char* backend();
    const char* version();
    const std::string build_date();
    const std::string build_info();
    const std::string running_info();

    void about_message(mrv::TextBrowser* b);
    void thanks_message(mrv::TextBrowser* b);

    void cpu_information(mrv::TextBrowser* b);
    uint32_t getVulkanLoaderVersion();
    std::string gpu_list(ViewerUI* ui);
    std::string gpu_information(ViewerUI* ui);

} // namespace mrv
