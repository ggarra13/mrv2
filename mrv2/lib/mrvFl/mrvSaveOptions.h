

#pragma once

#include <tlIO/FFmpeg.h>
#include <tlIO/OpenEXR.h>

namespace mrv
{
    struct SaveOptions
    {
        bool annotations = false;

        tl::ffmpeg::Profile ffmpegProfile = tl::ffmpeg::Profile::None;

        tl::exr::Compression exrCompression = tl::exr::Compression::ZIP;
        int zipCompressionLevel = 4;
        float dwaCompressionLevel = 45.0F;
    };
} // namespace mrv
