// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/IOutput.h>

namespace tl
{
    namespace bmd
    {
        //! BMD output device.
        class OutputDevice : public std::enable_shared_from_this<IOutput>
        {
            TLRENDER_NON_COPYABLE(IOutput);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            OutputDevice();

        public:
            ~OutputDevice();

            //! Create a new output device.
            static std::shared_ptr<OutputDevice>
            create(const std::shared_ptr<system::Context>&);

            //! Get the output device configuration.
            DeviceConfig getConfig() const;

            //! Observe the output device configuration.
            std::shared_ptr<observer::IValue<DeviceConfig> >
            observeConfig() const;

            //! Set the output device configuration.
            void setConfig(const DeviceConfig&);

            //! Get whether the output device is enabled.
            bool isEnabled() const;

            //! Observe whether the output device is enabled.
            std::shared_ptr<observer::IValue<bool> > observeEnabled() const;

            //! Set whether the output device is enabled.
            void setEnabled(bool);

            //! Get whether the output device is active.
            bool isActive() const;

            //! Observe whether the output device is active.
            std::shared_ptr<observer::IValue<bool> > observeActive() const;

            //! Get the output device size.
            const math::Size2i& getSize() const;

            //! Observe the output device size.
            std::shared_ptr<observer::IValue<math::Size2i> >
            observeSize() const;

            //! Get the output device frame rate.
            const otime::RationalTime& getFrameRate() const;

            //! Observe the output device frame rate.
            std::shared_ptr<observer::IValue<otime::RationalTime> >
            observeFrameRate() const;

            //! Set the view.
            void setView(
                const tl::math::Vector2i& position, double zoom, bool frame);

            //! Set the OpenColorIO options.
            void setOCIOOptions(const timeline::OCIOOptions&);

            //! Set the LUT options.
            void setLUTOptions(const timeline::LUTOptions&);

            //! Set the image options.
            void setImageOptions(const std::vector<timeline::ImageOptions>&);

            //! Set the display options.
            void
            setDisplayOptions(const std::vector<timeline::DisplayOptions>&);

            //! Set the HDR mode and metadata.
            void setHDR(HDRMode, const image::HDRData&);

            //! Set the comparison options.
            void setCompareOptions(const timeline::CompareOptions&);

            //! Set the background options.
            void setBackgroundOptions(const timeline::BackgroundOptions&);

            //! Set the overlay.
            void setOverlay(const std::shared_ptr<image::Image>&);

            //! Set the volume.
            void setVolume(float);

            //! Set whether the audio is muted.
            void setMute(bool);

            //! Set the audio sync offset.
            void setAudioOffset(double);

            //! Set the timeline player.
            void setPlayer(const std::shared_ptr<timeline::Player>&);

            //! Tick the output device.
            void tick();

        private:
            void _run();
            void _createDevice(
                const DeviceConfig&, bool& active, math::Size2i& size,
                otime::RationalTime& frameRate);
            void _render(
                const DeviceConfig&, const timeline::OCIOOptions&,
                const timeline::LUTOptions&,
                const std::vector<timeline::ImageOptions>&,
                const std::vector<timeline::DisplayOptions>&,
                const timeline::CompareOptions&,
                const timeline::BackgroundOptions&);
            void _read();

            TLRENDER_PRIVATE();
        };
    } // namespace bmd
} // namespace tl
