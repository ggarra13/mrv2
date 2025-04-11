// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlDevice/OutputData.h>

#include <tlTimeline/IRender.h>
#include <tlTimeline/Player.h>

namespace tl
{
    namespace device
    {
        //! Abstract output device.
        class IOutput : public virtual std::enable_shared_from_this<IOutput>
        {
            TLRENDER_NON_COPYABLE(IOutput);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            IOutput();

        public:
            virtual ~IOutput() = 0;

            //! Get the output device configuration.
            virtual DeviceConfig getConfig() const = 0;

            //! Observe the output device configuration.
            virtual std::shared_ptr<observer::IValue<DeviceConfig> >
            observeConfig() const = 0;

            //! Set the output device configuration.
            virtual void setConfig(const DeviceConfig&) = 0;

            //! Get whether the output device is enabled.
            virtual bool isEnabled() const = 0;

            //! Observe whether the output device is enabled.
            virtual std::shared_ptr<observer::IValue<bool> >
            observeEnabled() const = 0;

            //! Set whether the output device is enabled.
            virtual void setEnabled(bool) = 0;

            //! Get whether the output device is active.
            virtual bool isActive() const = 0;

            //! Observe whether the output device is active.
            virtual std::shared_ptr<observer::IValue<bool> >
            observeActive() const = 0;

            //! Get the output device size.
            virtual const math::Size2i& getSize() const = 0;

            //! Observe the output device size.
            virtual std::shared_ptr<observer::IValue<math::Size2i> >
            observeSize() const = 0;

            //! Get the output device frame rate.
            virtual const otime::RationalTime& getFrameRate() const = 0;

            //! Observe the output device frame rate.
            virtual std::shared_ptr<observer::IValue<otime::RationalTime> >
            observeFrameRate() const = 0;

            //! Set the view.
            virtual void setView(
                const tl::math::Size2i& viewportSize,
                const tl::math::Vector2i& position, double zoom, float rotateZ,
                bool frame) = 0;

            //! Set the OpenColorIO options.
            virtual void setOCIOOptions(const timeline::OCIOOptions&) = 0;

            //! Set the LUT options.
            virtual void setLUTOptions(const timeline::LUTOptions&) = 0;

            //! Set the image options.
            virtual void
            setImageOptions(const std::vector<timeline::ImageOptions>&) = 0;

            //! Set the display options.
            virtual void
            setDisplayOptions(const std::vector<timeline::DisplayOptions>&) = 0;

            //! Set the HDR mode and metadata.
            virtual void setHDR(HDRMode, const image::HDRData&) = 0;

            //! Set the comparison options.
            virtual void setCompareOptions(const timeline::CompareOptions&) = 0;

            //! Set the background options.
            virtual void
            setBackgroundOptions(const timeline::BackgroundOptions&) = 0;

            //! Set the overlay.
            virtual void setOverlay(const std::shared_ptr<image::Image>&) = 0;

            //! Set the volume.
            virtual void setVolume(float) = 0;

            //! Set whether the audio is muted.
            virtual void setMute(bool) = 0;

            //! Set the audio sync offset.
            virtual void setAudioOffset(double) = 0;

            //! Set the timeline player.
            virtual void
            setPlayer(const std::shared_ptr<timeline::Player>&) = 0;

            //! Tick the output device.
            virtual void tick() = 0;

        protected:
            std::weak_ptr<system::Context> _context;
        };
    } // namespace device
} // namespace tl
