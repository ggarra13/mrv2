// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

namespace tl
{
    namespace timeline
    {
        inline bool VideoLayer::operator==(const VideoLayer& other) const
        {
            return image == other.image &&
                   imageOptions == other.imageOptions &&
                   imageB == other.imageB &&
                   imageOptionsB == other.imageOptionsB &&
                   bounds == other.bounds &&
                   boundsB == other.boundsB &&
                   transition == other.transition &&
                   transitionValue == other.transitionValue;
        }

        inline bool VideoLayer::operator!=(const VideoLayer& other) const
        {
            return !(*this == other);
        }

        inline bool VideoFrame::operator==(const VideoFrame& other) const
        {
            return size == other.size &&
                   canvasSize == other.canvasSize &&
                   time.strictly_equal(other.time) &&
                   layers == other.layers;
        }

        inline bool VideoFrame::operator!=(const VideoFrame& other) const
        {
            return !(*this == other);
        }

        inline bool isTimeEqual(const VideoFrame& a, const VideoFrame& b)
        {
            return a.time.strictly_equal(b.time);
        }
    } // namespace timeline
} // namespace tl
