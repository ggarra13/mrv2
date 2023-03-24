// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

namespace mrv2
{
    namespace image
    {
        //! Video levels.
        enum class VideoLevels {
            FullRange,
            LegalRange,
        };

        //! YUV coefficients.
        enum class YUVCoefficients {
            REC709,
            BT2020,
        };

        //! Channels.
        enum class Channels {
            Color,
            Red,
            Green,
            Blue,
            Alpha,
        };

        //! Input video levels.
        enum class InputVideoLevels {
            FromFile,
            FullRange,
            LegalRange,
        };

        //! Alpha channel blending.
        //!
        //! References:
        //! - https://microsoft.github.io/Win2D/html/PremultipliedAlpha.htm
        enum class AlphaBlend {
            None,
            Straight,
            Premultiplied,
        };

        //! Image filtering.
        enum class ImageFilter {
            Nearest,
            Linear,
        };

        //! LUT operation order.
        enum class LUTOrder {
            PostColorConfig,
            PreColorConfig,
        };

    } // namespace image

    namespace media
    {
        //! Comparison mode.
        enum class CompareMode {
            A,
            B,
            Wipe,
            Overlay,
            Difference,
            Horizontal,
            Vertical,
            Tile,
        };

    } // namespace media

    namespace timeline
    {
        //! File sequence.
        enum class FileSequenceAudio {
            None,      //!< No audio
            BaseName,  //!< Search for an audio file with the same base name as
                       //!< the file sequence
            FileName,  //!< Use the given audio file name
            Directory, //!< Use the first audio file in the given directory
        };

        //! Timer modes.
        enum class TimerMode {
            System,
            Audio,
        };

        //! Audio buffer frame counts.
        enum class AudioBufferFrameCount {
            _16,
            _32,
            _64,
            _128,
            _256,
            _512,
            _1024,
        };

        //! Playback modes.
        enum class Playback {
            Stop,
            Forward,
            Reverse,
        };

        //! Playback loop modes.
        enum class Loop {
            Loop,
            Once,
            PingPong,
        };
    } // namespace timeline

} // namespace mrv2
