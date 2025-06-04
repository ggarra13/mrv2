// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace timeline
    {
        inline bool AudioLayer::operator==(const AudioLayer& other) const
        {
            return audio == other.audio;
        }

        inline bool AudioLayer::operator!=(const AudioLayer& other) const
        {
            return !(*this == other);
        }

        inline bool AudioData::operator==(const AudioData& other) const
        {
            return seconds == other.seconds && layers == other.layers;
        }

        inline bool AudioData::operator!=(const AudioData& other) const
        {
            return !(*this == other);
        }

        inline bool isTimeEqual(const AudioData& a, const AudioData& b)
        {
            return a.seconds == b.seconds;
        }
    } // namespace timeline
} // namespace tl
