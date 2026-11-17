// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#define NOMINMAX

#include <tlCore/AudioSystem.h>

#include <tlCore/Context.h>
#include <tlCore/Error.h>
#include <tlCore/LogSystem.h>
#include <tlCore/String.h>

#if defined(TLRENDER_AUDIO)
// miniaudio is a single-header library. The implementation must be compiled
// in exactly one translation unit. If another file in your project already
// defines MINIAUDIO_IMPLEMENTATION (e.g. the audio output code), remove it
// from here.
#    define MINIAUDIO_IMPLEMENTATION
#    include <miniaudio/miniaudio.h>
#endif // TLRENDER_AUDIO

#include <algorithm>
#include <array>
#include <limits>
#include <map>
#include <set>
#include <sstream>

namespace tl
{
    namespace audio
    {
        TLRENDER_ENUM_IMPL(
            DeviceFormat, "S8", "S16", "S24", "S32", "F32", "F64");
        TLRENDER_ENUM_SERIALIZE_IMPL(DeviceFormat);

#if defined(TLRENDER_AUDIO)
        namespace
        {
            struct ContextDeleter
            {
                void operator()(ma_context* p) const
                {
                    ma_context_uninit(p);
                    delete p;
                }
            };

            //! Sample rates reported when a backend says "any rate" (0).
            const std::array<size_t, 11> standardSampleRates = {
                8000,  11025, 16000, 22050,  32000, 44100,
                48000, 88200, 96000, 176400, 192000};

            //! Channel count assumed when a backend says "any" (0).
            const size_t defaultChannelCount = 2;
        } // namespace
#endif // TLRENDER_AUDIO

        struct System::Private
        {
#if defined(TLRENDER_AUDIO)
            std::shared_ptr<ma_context> context;
            //! Enabled miniaudio backends, parallel to "apis".
            std::vector<ma_backend> backends;
            //! miniaudio device IDs, parallel to "devices". These are what
            //! you need to open a device with ma_device_init().
            std::vector<ma_device_id> deviceIds;
#endif // TLRENDER_AUDIO
            //! Index into "apis"; -1 means "let miniaudio choose".
            int currentApi = -1;
            std::vector<std::string> apis;
            std::vector<Device> devices;
            size_t defaultInputDevice = 0;
            size_t defaultOutputDevice = 0;
            std::string inputDeviceName;
            std::string outputDeviceName;
        };

        void System::_init(const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

#if defined(TLRENDER_AUDIO)
            {
                std::stringstream ss;
                ss << "miniaudio version: " << ma_version_string();
                _log(ss.str());
            }

            std::array<ma_backend, MA_BACKEND_COUNT> backends;
            size_t backendCount = 0;
            if (ma_get_enabled_backends(
                    backends.data(), backends.size(), &backendCount) ==
                MA_SUCCESS)
            {
                for (size_t i = 0; i < backendCount; ++i)
                {
                    if (backends[i] == ma_backend_jack)
                        continue;
                    p.backends.push_back(backends[i]);
                    p.apis.push_back(ma_get_backend_name(backends[i]));

                    std::stringstream ss;
                    ss << "Audio API: " << ma_get_backend_name(backends[i]);
                    _log(ss.str());
                }
            }
            _getDevices();
#endif // TLRENDER_AUDIO
        }

        System::System(const std::shared_ptr<system::Context>& context) :
            system::ISystem(context, "tl::time::AudioSystem"),
            _p(new Private)
        {
            _init(context);
        }

        System::~System() {}

        std::shared_ptr<System>
        System::create(const std::shared_ptr<system::Context>& context)
        {
            auto out = context->getSystem<System>();
            if (!out)
            {
                out = std::shared_ptr<System>(new System(context));
            }
            return out;
        }

        const std::vector<std::string>& System::getAPIs() const
        {
            return _p->apis;
        }

        const std::vector<Device>& System::getDevices() const
        {
            return _p->devices;
        }

        size_t System::getDefaultInputDevice() const
        {
            return _p->defaultInputDevice;
        }

        size_t System::getDefaultOutputDevice() const
        {
            return _p->defaultOutputDevice;
        }

        namespace
        {
            DeviceFormat getBestFormat(std::vector<DeviceFormat> value)
            {
                std::sort(
                    value.begin(), value.end(),
                    [](DeviceFormat a, DeviceFormat b) {
                        return static_cast<size_t>(a) < static_cast<size_t>(b);
                    });
                return !value.empty() ? value.back() : DeviceFormat::F32;
            }

            DataType getDataType(const Device& device)
            {
                DataType out = DataType::F32;
                switch (getBestFormat(device.nativeFormats))
                {
                case DeviceFormat::S8:
                    out = DataType::S8;
                    break;
                case DeviceFormat::S16:
                    out = DataType::S16;
                    break;
                case DeviceFormat::S24:
                case DeviceFormat::S32:
                    out = DataType::S32;
                    break;
                case DeviceFormat::F32:
                    out = DataType::F32;
                    break;
                case DeviceFormat::F64:
                    out = DataType::F64;
                    break;
                default:
                    break;
                }
                return out;
            }
        } // namespace

        Info System::getInputInfo(const size_t deviceIndex) const
        {
            TLRENDER_P();
            Info out;
            if (deviceIndex < p.devices.size())
            {
                const auto& device = p.devices[deviceIndex];
                out.channelCount = device.inputChannels;
                out.dataType = getDataType(device);
                out.sampleRate = device.preferredSampleRate;
            }
            return out;
        }

        Info System::getDefaultInputInfo() const
        {
            return getInputInfo(getInputDevice());
        }

        Info System::getOutputInfo(const size_t deviceIndex) const
        {
            TLRENDER_P();
            Info out;
            if (deviceIndex < p.devices.size())
            {
                const auto& device = p.devices[deviceIndex];
                out.channelCount = device.outputChannels;
                out.dataType = getDataType(device);
                out.sampleRate = device.preferredSampleRate;
            }
            return out;
        }

        Info System::getDefaultOutputInfo() const
        {
            return getOutputInfo(getOutputDevice());
        }

        std::shared_ptr<ma_context> System::getContext() const
        {
#if defined(TLRENDER_AUDIO)
            return _p->context;
#else
            return nullptr;
#endif // TLRENDER_AUDIO
        }

        const void* System::getDeviceId(const size_t index) const
        {
#if defined(TLRENDER_AUDIO)
            if (index < _p->deviceIds.size())
            {
                return &_p->deviceIds[index];
            }
#endif // TLRENDER_AUDIO
            return nullptr;
        }

        void System::setAPI(const std::string& apiName)
        {
            TLRENDER_P();
            int index = 0;
            for (const auto& api : p.apis)
            {
                if (api == apiName)
                {
                    if (p.currentApi != index)
                    {
                        p.currentApi = index;
                        _getDevices();
                    }
                    break;
                }
                ++index;
            }
        }

        int System::getCurrentAPI() const
        {
            return _p->currentApi;
        }

        void System::setOutputDevice(const std::string& outputDeviceName)
        {
            TLRENDER_P();
            for (const auto& device : p.devices)
            {
                if (outputDeviceName == device.name &&
                    device.outputChannels > 0)
                {
                    p.outputDeviceName = outputDeviceName;
                    break;
                }
            }
        }

        size_t System::getOutputDevice() const
        {
            TLRENDER_P();
            size_t out = 0;
            if (p.outputDeviceName.empty())
            {
                out = getDefaultOutputDevice();
            }
            else
            {
                out = _deviceNameToIndex(p.outputDeviceName, false);
            }
            return out;
        }

        void System::setInputDevice(const std::string& inputDeviceName)
        {
            TLRENDER_P();
            for (const auto& device : p.devices)
            {
                if (inputDeviceName == device.name && device.inputChannels > 0)
                {
                    p.inputDeviceName = inputDeviceName;
                    break;
                }
            }
        }

        size_t System::getInputDevice() const
        {
            TLRENDER_P();
            size_t out = 0;
            if (p.inputDeviceName.empty())
            {
                out = getDefaultInputDevice();
            }
            else
            {
                out = _deviceNameToIndex(p.inputDeviceName, true);
            }
            return out;
        }

        // miniaudio lists playback and capture devices separately, so the same
        // name can appear once as an output and once as an input. The lookup
        // therefore has to know which direction it wants.
        size_t System::_deviceNameToIndex(
            const std::string& name, const bool input) const
        {
            TLRENDER_P();
            size_t out = input ? p.defaultInputDevice : p.defaultOutputDevice;
            for (size_t i = 0; i < p.devices.size(); ++i)
            {
                const auto& device = p.devices[i];
                const size_t channels =
                    input ? device.inputChannels : device.outputChannels;
                if (device.name == name && channels > 0)
                {
                    out = i;
                    break;
                }
            }
            return out;
        }

        void System::_getDevices()
        {
            TLRENDER_P();

#if defined(TLRENDER_AUDIO)
            p.devices.clear();
            p.deviceIds.clear();
            p.defaultInputDevice = 0;
            p.defaultOutputDevice = 0;
            p.context.reset();

            // Create a context for the selected backend (or let miniaudio
            // choose when no backend has been selected).
            ma_backend backend = ma_backend_null;
            const ma_backend* backends = nullptr;
            ma_uint32 backendCount = 0;
            if (p.currentApi >= 0 &&
                p.currentApi < static_cast<int>(p.backends.size()))
            {
                backend = p.backends[p.currentApi];
                backends = &backend;
                backendCount = 1;
            }
            std::shared_ptr<ma_context> context;
            {
                std::unique_ptr<ma_context> raw(new ma_context);
                const ma_result r =
                    ma_context_init(backends, backendCount, nullptr, raw.get());
                if (r != MA_SUCCESS)
                {
                    std::stringstream ss;
                    ss << "Cannot initialize audio system: "
                       << ma_result_description(r);
                    _log(ss.str(), log::Type::Error);
                    return;
                }
                // The context is initialized; from here on the deleter
                // is responsible for ma_context_uninit().
                context = std::shared_ptr<ma_context>(
                    raw.release(), ContextDeleter());
            }

            // Record which API we actually ended up with.
            for (size_t i = 0; i < p.backends.size(); ++i)
            {
                if (p.backends[i] == context->backend)
                {
                    p.currentApi = static_cast<int>(i);
                    break;
                }
            }

            ma_device_info* playbackInfos = nullptr;
            ma_uint32 playbackCount = 0;
            ma_device_info* captureInfos = nullptr;
            ma_uint32 captureCount = 0;
            {
                const ma_result r = ma_context_get_devices(
                    context.get(), &playbackInfos, &playbackCount,
                    &captureInfos, &captureCount);
                if (r != MA_SUCCESS)
                {
                    std::stringstream ss;
                    ss << "Cannot get audio devices: "
                       << ma_result_description(r);
                    _log(ss.str(), log::Type::Error);
                    p.context = std::move(context);
                    return;
                }
            }

            // Add the devices for one direction. Returns the index of the
            // default device for that direction, or SIZE_MAX if none found.
            auto addDevices = [&](ma_device_type type,
                                  const ma_device_info* infos,
                                  ma_uint32 count) -> size_t
            {
                const bool input = ma_device_type_capture == type;
                size_t defaultIndex = std::numeric_limits<size_t>::max();
                size_t firstIndex = std::numeric_limits<size_t>::max();
                for (ma_uint32 i = 0; i < count; ++i)
                {
                    // The enumeration only gives names and IDs. Probe each
                    // device for its native formats.
                    ma_device_info info;
                    if (ma_context_get_device_info(
                            context.get(), type, &infos[i].id, &info) !=
                        MA_SUCCESS)
                    {
                        continue;
                    }

                    Device device;
                    device.name = info.name;

                    size_t channels = 0;
                    std::set<size_t> sampleRates;
                    std::set<DeviceFormat> formats;
                    for (ma_uint32 j = 0; j < info.nativeDataFormatCount; ++j)
                    {
                        const auto& f = info.nativeDataFormats[j];
                        switch (f.format)
                        {
                        // miniaudio only has unsigned 8-bit; S8 is the
                        // closest DeviceFormat.
                        case ma_format_u8:
                            formats.insert(DeviceFormat::S8);
                            break;
                        case ma_format_s16:
                            formats.insert(DeviceFormat::S16);
                            break;
                        case ma_format_s24:
                            formats.insert(DeviceFormat::S24);
                            break;
                        case ma_format_s32:
                            formats.insert(DeviceFormat::S32);
                            break;
                        case ma_format_f32:
                            formats.insert(DeviceFormat::F32);
                            break;
                        default:
                            break; // ma_format_unknown: any format.
                        }
                        channels = std::max<size_t>(channels, f.channels);
                        if (f.sampleRate > 0)
                        {
                            sampleRates.insert(f.sampleRate);
                        }
                    }
                    // Backends such as ALSA and PulseAudio report 0 for
                    // "any". Fill in sensible values.
                    if (0 == channels)
                    {
                        channels = defaultChannelCount;
                    }
                    if (sampleRates.empty())
                    {
                        sampleRates.insert(
                            standardSampleRates.begin(),
                            standardSampleRates.end());
                    }
                    if (formats.empty())
                    {
                        formats.insert(DeviceFormat::F32);
                    }

                    if (input)
                    {
                        device.inputChannels = channels;
                    }
                    else
                    {
                        device.outputChannels = channels;
                    }
                    // Playback and capture devices are separate entries.
                    device.duplexChannels = 0;
                    device.sampleRates.assign(
                        sampleRates.begin(), sampleRates.end());
                    // miniaudio has no "preferred" rate: use 48kHz if the
                    // device supports it, otherwise the first rate.
                    device.preferredSampleRate =
                        sampleRates.count(48000) ? 48000 : *sampleRates.begin();
                    device.nativeFormats.assign(
                        formats.begin(), formats.end());

                    const size_t index = p.devices.size();
                    p.devices.push_back(device);
                    p.deviceIds.push_back(infos[i].id);
                    if (std::numeric_limits<size_t>::max() == firstIndex)
                    {
                        firstIndex = index;
                    }
                    if (infos[i].isDefault)
                    {
                        defaultIndex = index;
                    }
                }
                if (std::numeric_limits<size_t>::max() == defaultIndex)
                {
                    defaultIndex = firstIndex;
                }
                return defaultIndex;
            };
            const size_t defaultOutput = addDevices(
                ma_device_type_playback, playbackInfos, playbackCount);
            const size_t defaultInput =
                addDevices(ma_device_type_capture, captureInfos, captureCount);
            if (defaultOutput != std::numeric_limits<size_t>::max())
            {
                p.defaultOutputDevice = defaultOutput;
            }
            if (defaultInput != std::numeric_limits<size_t>::max())
            {
                p.defaultInputDevice = defaultInput;
            }
            p.context = std::move(context);

            // Log the results.
            std::vector<std::string> log;
            log.push_back(std::string());
            for (size_t i = 0; i < p.devices.size(); ++i)
            {
                const auto& device = p.devices[i];
                {
                    std::stringstream ss;
                    ss << "    Device " << i << ": " << device.name;
                    log.push_back(ss.str());
                }
                {
                    std::stringstream ss;
                    ss << "        Channels: " << device.outputChannels
                       << " output, " << device.inputChannels << " input, "
                       << device.duplexChannels << " duplex";
                    log.push_back(ss.str());
                }
                {
                    std::stringstream ss;
                    ss << "        Sample rates: ";
                    for (auto j : device.sampleRates)
                    {
                        ss << j << " ";
                    }
                    log.push_back(ss.str());
                }
                {
                    std::stringstream ss;
                    ss << "        Preferred sample rate: "
                       << device.preferredSampleRate;
                    log.push_back(ss.str());
                }
                {
                    std::stringstream ss;
                    ss << "        Native formats: ";
                    for (auto j : device.nativeFormats)
                    {
                        ss << j << " ";
                    }
                    log.push_back(ss.str());
                }
            }
            {
                std::stringstream ss;
                ss << "    Default input device: " << getDefaultInputDevice();
                log.push_back(ss.str());
            }
            {
                const Info info = getDefaultInputInfo();
                std::stringstream ss;
                ss << "    Default input info: "
                   << static_cast<size_t>(info.channelCount) << " "
                   << info.dataType << " " << info.sampleRate;
                log.push_back(ss.str());
            }
            {
                std::stringstream ss;
                ss << "    Default output device: " << getDefaultOutputDevice();
                log.push_back(ss.str());
            }
            {
                const Info info = getDefaultOutputInfo();
                std::stringstream ss;
                ss << "    Default output info: "
                   << static_cast<size_t>(info.channelCount) << " "
                   << info.dataType << " " << info.sampleRate;
                log.push_back(ss.str());
            }
            _log(string::join(log, "\n"));
#endif // TLRENDER_AUDIO
        }
    } // namespace audio
} // namespace tl
