// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


namespace mrv
{

    enum ColorAccuracy {
        kAccuracyAuto,
        kAccuracyFloat16,
        kAccuracyFloat32,
        kAccuracyFast
    };

    enum MonitorVSync { kVSynckNone, kVSyncAlways, kVSyncPresentationOnly };

    enum PixelDisplay { kRGBA_Float, kRGBA_Hex, kRGBA_Decimal };

    enum PixelValue { kFull, kOriginal };

    enum Blit { kNoBlit, kBlit };

    enum HudDisplay {
        kNone = 0,
        kFilename = 1 << 0,
        kDirectory = 1 << 1,
        kFrame = 1 << 2,
        kFrameRange = 1 << 3,
        kFrameCount = 1 << 4,
        kResolution = 1 << 5,
        kTimecode = 1 << 6,
        kFPS = 1 << 7,
        kMemory = 1 << 8,
        kCache = 1 << 9,
        kAttributes = 1 << 10,
    };

    enum MissingFrameType { kBlackFrame, kRepeatFrame, kScratchedFrame };

}
