// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

#include "mrvWidgets/mrvBrowser.h"

namespace mrv
{

    void ffmpeg_formats(mrv::Browser& b);
    void ffmpeg_video_codecs(mrv::Browser& b);
    void ffmpeg_audio_codecs(mrv::Browser& b);
    void ffmpeg_subtitle_codecs(mrv::Browser& b);
    void ffmpeg_protocols(mrv::Browser& b);
    void ffmpeg_motion_estimation_methods(mrv::Browser* b);

    const char* version();
    const char* build_date();

    std::string about_message();

    std::string cpu_information();
    std::string gpu_information(ViewerUI* uiMain);

} // namespace mrv
