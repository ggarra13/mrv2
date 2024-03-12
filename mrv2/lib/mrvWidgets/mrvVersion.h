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

    const char* version();
    const char* build_date();

    void about_message(mrv::TextBrowser* b);

    void cpu_information(mrv::TextBrowser* b);
    std::string gpu_information(ViewerUI* uiMain);

} // namespace mrv
