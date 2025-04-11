// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/IOutput.h>

#include <tlTimeline/Audio.h>

#include <tlCore/Matrix.h>

namespace tl
{
    namespace ndi
    {
        //! NDI output device.
        class OutputDevice : public device::IOutput
        {

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            OutputDevice();

        public:
            ~OutputDevice();

            //! Create a new output device.
            static std::shared_ptr<OutputDevice>
            create(const std::shared_ptr<system::Context>&);

            //! Get the output device configuration.
            device::DeviceConfig getConfig() const override;

            //! Observe the output device configuration.
            std::shared_ptr<observer::IValue<device::DeviceConfig> >
            observeConfig() const override;

            //! Set the output device configuration.
            void setConfig(const device::DeviceConfig&) override;

            //! Get whether the output device is enabled.
            bool isEnabled() const override;

            //! Observe whether the output device is enabled.
            std::shared_ptr<observer::IValue<bool> >
            observeEnabled() const override;

            //! Set whether the output device is enabled.
            void setEnabled(bool) override;

            //! Get whether the output device is active.
            bool isActive() const override;

            //! Observe whether the output device is active.
            std::shared_ptr<observer::IValue<bool> >
            observeActive() const override;

            //! Get the output device size.
            const math::Size2i& getSize() const override;

            //! Observe the output device size.
            std::shared_ptr<observer::IValue<math::Size2i> >
            observeSize() const override;

            //! Get the output device frame rate.
            const otime::RationalTime& getFrameRate() const override;

            //! Observe the output device frame rate.
            std::shared_ptr<observer::IValue<otime::RationalTime> >
            observeFrameRate() const override;

            //! Set the view.
            void setView(
                const tl::math::Size2i& viewportSize,
                const tl::math::Vector2i& position, double zoom, float rotateZ,
                bool frame) override;

            //! Set the OpenColorIO options.
            void setOCIOOptions(const timeline::OCIOOptions&) override;

            //! Set the LUT options.
            void setLUTOptions(const timeline::LUTOptions&) override;

            //! Set the image options.
            void setImageOptions(
                const std::vector<timeline::ImageOptions>&) override;

            //! Set the display options.
            void setDisplayOptions(
                const std::vector<timeline::DisplayOptions>&) override;

            //! Set the HDR mode and metadata.
            void setHDR(device::HDRMode, const image::HDRData&) override;

            //! Set the comparison options.
            void setCompareOptions(const timeline::CompareOptions&) override;

            //! Set the background options.
            void
            setBackgroundOptions(const timeline::BackgroundOptions&) override;

            //! Set the overlay.
            void setOverlay(const std::shared_ptr<image::Image>&) override;

            //! Set the volume.
            void setVolume(float) override;

            //! Set whether the audio is muted.
            void setMute(bool) override;

            //! Set the audio sync offset.
            void setAudioOffset(double) override;

            //! Set the timeline player.
            void setPlayer(const std::shared_ptr<timeline::Player>&) override;

            //! Tick the output device.
            void tick() override;

        private:
            void _run();
            void _createDevice(
                const device::DeviceConfig&, bool& active, math::Size2i& size,
                otime::RationalTime& frameRate);
            timeline::AudioData findAudioData(double seconds);
            void _audio();
            math::Matrix4x4f _projectionMatrix() const noexcept;
            void _render(
                const device::DeviceConfig&, const timeline::OCIOOptions&,
                const timeline::LUTOptions&,
                const std::vector<timeline::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&,
                const timeline::BackgroundOptions&);
            void _read(const device::DeviceConfig&);
            void _cacheUpdate(const std::vector<timeline::AudioData>&);

            TLRENDER_PRIVATE();
        };
    } // namespace ndi
} // namespace tl
