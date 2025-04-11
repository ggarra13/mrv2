// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Range.h>
#include <tlCore/Util.h>

#include <limits>
#include <list>
#include <memory>
#include <string>
#include <vector>

namespace tl
{
    //! Audio
    namespace audio
    {
        //! \name Audio Types
        ///@{

        //! Audio data types.
        enum class DataType {
            None,
            S8,
            S16,
            S32,
            F32,
            F64,

            Count,
            First = None
        };
        TLRENDER_ENUM(DataType);
        TLRENDER_ENUM_SERIALIZE(DataType);

        typedef int8_t S8_T;
        typedef int16_t S16_T;
        typedef int32_t S32_T;
        typedef float F32_T;
        typedef double F64_T;

        const math::Range<S8_T> S8Range(
            std::numeric_limits<S8_T>::min(), std::numeric_limits<S8_T>::max());
        const math::Range<S16_T> S16Range(
            std::numeric_limits<S16_T>::min(),
            std::numeric_limits<S16_T>::max());
        const math::Range<S32_T> S32Range(
            std::numeric_limits<S32_T>::min(),
            std::numeric_limits<S32_T>::max());
        const math::Range<F32_T> F32Range(-1.F, 1.F);
        const math::Range<F64_T> F64Range(-1.F, 1.F);

        //! Get the byte count for the given data type.
        size_t getByteCount(DataType);

        //! Determine the integer data type for a given byte count.
        DataType getIntType(size_t);

        //! Determine the floating point data type for a given byte count.
        DataType getFloatType(size_t);

        ///@}

        //! \name Audio Type Conversion
        ///@{

        void S8ToS16(S8_T, S16_T&);
        void S8ToS32(S8_T, S32_T&);
        void S8ToF32(S8_T, F32_T&);
        void S8ToF64(S8_T, F64_T&);

        void S16ToS8(S16_T, S8_T&);
        void S16ToS32(S16_T, S32_T&);
        void S16ToF32(S16_T, F32_T&);
        void S16ToF64(S16_T, F64_T&);

        void S32ToS8(S32_T, S8_T&);
        void S32ToS16(S32_T, S16_T&);
        void S32ToF32(S32_T, F32_T&);
        void S32ToF64(S32_T, F64_T&);

        void F32ToS8(F32_T, S8_T&);
        void F32ToS16(F32_T, S16_T&);
        void F32ToS32(F32_T, S32_T&);
        void F32ToF64(F32_T, F64_T&);

        void F64ToS8(F64_T, S8_T&);
        void F64ToS16(F64_T, S16_T&);
        void F64ToS32(F64_T, S32_T&);
        void F64ToF32(F64_T, F32_T&);

        ///@}

        class Info;

        //! Audio data information.
        class Info
        {
        public:
            Info();
            Info(size_t channelCount, DataType dataType, size_t sampleRate);

            std::string name = "Default";
            size_t channelCount = 0;
            DataType dataType = DataType::None;
            size_t sampleRate = 0;
            unsigned currentTrack = 0;
            unsigned trackCount = 1;
            std::vector<std::shared_ptr<Info>> audioInfo;

            //! Is the audio valid?
            bool isValid() const;

            //! Get the byte count.
            size_t getByteCount() const;

            bool operator==(const Info&) const;
            bool operator!=(const Info&) const;
        };

        //! Audio data.
        class Audio : public std::enable_shared_from_this<Audio>
        {
            TLRENDER_NON_COPYABLE(Audio);

        protected:
            void _init(const Info&, size_t sampleCount);

            Audio();

        public:
            ~Audio();

            //! Create new audio.
            static std::shared_ptr<Audio>
            create(const Info& info, size_t sampleCount);

            //! Get the audio information.
            const Info& getInfo() const;

            //! Get the audio channel count.
            size_t getChannelCount() const;

            //! Get the audio data type.
            DataType getDataType() const;

            //! Get the audio sample rate.
            size_t getSampleRate() const;

            //! Get the audio sample count.
            size_t getSampleCount() const;

            //! Is the audio valid?
            bool isValid() const;

            //! Get the audio data byte count.
            size_t getByteCount() const;

            //! Get the audio data.
            uint8_t* getData();

            //! Get the audio data.
            const uint8_t* getData() const;

            //! Zero the audio data.
            void zero();

        private:
            Info _info;
            size_t _sampleCount = 0;
            std::vector<uint8_t> _data;
        };

        //! \name Utility
        ///@{

        //! Reverse audio sources in place.
        void reverse(
            uint8_t** inOut, size_t inCount, size_t sampleCount,
            uint8_t channelCount, DataType dataType);

        //! Mix audio sources.
        void
        mix(const uint8_t** in, size_t inCount, uint8_t* out, float volume,
            const std::vector<float>& volumeScale, size_t sampleCount,
            size_t channelCount, DataType dataType);

        //! Convert audio data.
        std::shared_ptr<Audio> convert(const std::shared_ptr<Audio>&, DataType);

        //! Interleave audio data.
        std::shared_ptr<Audio> planarInterleave(const std::shared_ptr<Audio>&);

        //! Interleave audio data.
        template <typename T>
        void planarInterleave(
            const T** in, T* out, size_t channelCount, size_t sampleCount);

        //! De-interleave audio data.
        std::shared_ptr<Audio>
        planarDeinterleave(const std::shared_ptr<Audio>&);

        //! Get the total sample count from a list of audio data.
        size_t getSampleCount(const std::list<std::shared_ptr<audio::Audio> >&);

        //! Move audio data.
        void move(
            std::list<std::shared_ptr<Audio> >& in, uint8_t* out,
            size_t byteCount);

        ///@}
    } // namespace audio
} // namespace tl

#include <tlCore/AudioInline.h>
