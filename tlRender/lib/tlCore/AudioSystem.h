// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Audio.h>
#include <tlCore/ISystem.h>

#include <memory>

// Forward declarations of miniaudio types, so this header does not need
// miniaudio.h.
struct ma_context;

namespace tl
{
    namespace audio
    {
        //! Audio device format.
        enum class DeviceFormat {
            S8,
            S16,
            S24,
            S32,
            F32,
            F64,

            Count,
            First = S8
        };
        TLRENDER_ENUM(DeviceFormat);
        TLRENDER_ENUM_SERIALIZE(DeviceFormat);

        //! Audio device.
        struct Device
        {
            std::string name;
            size_t outputChannels = 0;
            size_t inputChannels = 0;
            size_t duplexChannels = 0;
            std::vector<size_t> sampleRates;
            size_t preferredSampleRate = 0;
            std::vector<DeviceFormat> nativeFormats;
        };

        //! Audio system.
        class System : public system::ISystem
        {
            TLRENDER_NON_COPYABLE(System);

            void _init(const std::shared_ptr<system::Context>&);
            System(const std::shared_ptr<system::Context>&);

        public:

            virtual ~System();

            //! Create a new system.
            static std::shared_ptr<System>
            create(const std::shared_ptr<system::Context>&);

            //! Get the list of audio APIs.
            const std::vector<std::string>& getAPIs() const;

            //! Return current audio API.
            int getCurrentAPI() const;

            //! Set Audio API to use.
            void setAPI(const std::string& api);

            //! Get the audio devices.
            const std::vector<Device>& getDevices() const;

            //! Get the default audio input device.
            size_t getDefaultInputDevice() const;

            //! Get the current audio input device.
            size_t getInputDevice() const;

            //! Set the current audio input device.
            void setInputDevice(const std::string&);

            //! Get the default audio output device.
            size_t getDefaultOutputDevice() const;

            //! Get the current audio output device.
            size_t getOutputDevice() const;

            //! Set the current audio outut device.
            void setOutputDevice(const std::string&);

            //! Get the audio information for input device.
            Info getInputInfo(const size_t) const;

            //! Get the default audio input device information.
            Info getDefaultInputInfo() const;

            //! Get the audio information for output device.
            Info getOutputInfo(const size_t) const;

            //! Get the default audio output device information.
            Info getDefaultOutputInfo() const;

            //! Get the miniaudio context. Hold on to the returned pointer for
            //! as long as you use devices created from it; it keeps the
            //! context alive even if the audio API is changed. Returns null
            //! if audio is not available.
            std::shared_ptr<ma_context> getContext() const;

            //! Get the miniaudio device ID (a const ma_device_id*) for a
            //! device index (an index into getDevices()). It is returned as
            //! void* because ma_device_id is an anonymous union and cannot be
            //! forward declared; cast it with
            //! static_cast<const ma_device_id*>(). Returns null for an
            //! invalid index, which miniaudio treats as "use the default
            //! device". The pointer is only valid until the next call to
            //! setAPI(); use it right away (ma_device_init() copies the ID).
            const void* getDeviceId(size_t index) const;

        private:
            size_t _deviceNameToIndex(const std::string&, bool input) const;
            void _getDevices();

            TLRENDER_PRIVATE();
        };
    } // namespace audio
} // namespace tl
