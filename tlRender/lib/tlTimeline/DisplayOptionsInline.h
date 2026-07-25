// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// Copyright (c) 2025-Present Gonzalo Garramuño.
// All rights reserved.

namespace tl
{
    namespace timeline
    {
        inline bool Color::operator==(const Color& other) const
        {
            return enabled == other.enabled && add == other.add &&
                   brightness == other.brightness &&
                   contrast == other.contrast &&
                   saturation == other.saturation && tint == other.tint &&
                   invert == other.invert;
        }

        inline bool Color::operator!=(const Color& other) const
        {
            return !(*this == other);
        }

        inline bool Levels::operator==(const Levels& other) const
        {
            return enabled == other.enabled && inLow == other.inLow &&
                   inHigh == other.inHigh && gamma == other.gamma &&
                   outLow == other.outLow && outHigh == other.outHigh;
        }

        inline bool Levels::operator!=(const Levels& other) const
        {
            return !(*this == other);
        }

        inline bool EXRDisplay::operator==(const EXRDisplay& other) const
        {
            return enabled == other.enabled && exposure == other.exposure &&
                   defog == other.defog && kneeLow == other.kneeLow &&
                   kneeHigh == other.kneeHigh;
        }

        inline bool EXRDisplay::operator!=(const EXRDisplay& other) const
        {
            return !(*this == other);
        }

        inline bool SoftClip::operator==(const SoftClip& other) const
        {
            return enabled == other.enabled && value == other.value;
        }

        inline bool SoftClip::operator!=(const SoftClip& other) const
        {
            return !(*this == other);
        }

        inline
        AspectRatio::AspectRatio(float num, float den) :
            num(num),
            den(den)
        {}

        inline
        bool AspectRatio::isValid() const
        {
            return num > 0.F && den > 0.F;
        }

        inline
        AspectRatio::operator float() const
        {
            return den > 0.F ? (num / den) : 0.F;
        }

        inline
        bool AspectRatio::operator == (const AspectRatio& other) const
        {
            return
                num == other.num &&
                den == other.den;
        }

        inline
        bool AspectRatio::operator != (const AspectRatio& other) const
        {
            return !(*this == other);
        }

        inline
        AspectRatioOptions::AspectRatioOptions(const AspectRatio& value, AspectRatioType type) :
            value(value),
            type(type)
        {}

        inline
        bool AspectRatioOptions::operator == (const AspectRatioOptions& other) const
        {
            return
                value == other.value &&
                type == other.type;
        }

        inline
        bool AspectRatioOptions::operator != (const AspectRatioOptions& other) const
        {
            return !(*this == other);
        }

        inline bool Normalize::operator==(const Normalize& other) const
        {
            return enabled == other.enabled && minimum == other.minimum &&
                   maximum == other.maximum;
        }

        inline bool Normalize::operator!=(const Normalize& other) const
        {
            return !(*this == other);
        }

        inline bool
        DisplayOptions::operator==(const DisplayOptions& other) const
        {
            return channels == other.channels && mirror == other.mirror &&
                   color == other.color && levels == other.levels &&
                   exrDisplay == other.exrDisplay &&
                   softClip == other.softClip &&
                   imageFilters == other.imageFilters &&
                   hdrInfo == other.hdrInfo &&
                   videoLevels == other.videoLevels &&
                   normalize == other.normalize &&
                   ignoreChromaticities == other.ignoreChromaticities &&
                   invalidValues == other.invalidValues;
        }

        inline bool
        DisplayOptions::operator!=(const DisplayOptions& other) const
        {
            return !(*this == other);
        }
    } // namespace timeline
} // namespace tl
