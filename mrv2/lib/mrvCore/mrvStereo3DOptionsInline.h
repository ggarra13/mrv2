// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

namespace mrv
{
    inline bool Stereo3DOptions::operator==(const Stereo3DOptions& b) const
    {
        return (
            output == b.output && input == b.input &&
            eyeSeparation == b.eyeSeparation);
    }

    inline bool Stereo3DOptions::operator!=(const Stereo3DOptions& b) const
    {
        return !(*this == b);
    }
} // namespace mrv
