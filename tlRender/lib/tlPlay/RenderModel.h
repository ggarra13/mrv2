// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/ImageOptions.h>
#include <tlTimeline/RenderOptions.h>

#include <tlCore/ValueObserver.h>

namespace tl
{
    namespace system
    {
        class Context;
    }

    namespace play
    {
        class Settings;

        //! Render model.
        class RenderModel : public std::enable_shared_from_this<RenderModel>
        {
            TLRENDER_NON_COPYABLE(RenderModel);

        protected:
            void _init(
                const std::shared_ptr<Settings>&,
                const std::shared_ptr<system::Context>&);

            RenderModel();

        public:
            ~RenderModel();

            //! Create a new model.
            static std::shared_ptr<RenderModel> create(
                const std::shared_ptr<Settings>&,
                const std::shared_ptr<system::Context>&);

            //! Get the image options.
            const timeline::ImageOptions& getImageOptions() const;

            //! Observe the image options.
            std::shared_ptr<observer::IValue<timeline::ImageOptions> >
            observeImageOptions() const;

            //! Set the image options.
            void setImageOptions(const timeline::ImageOptions&);

            //! Get the color buffer type.
            image::PixelType getColorBuffer() const;

            //! Observe the color buffer type.
            std::shared_ptr<observer::IValue<image::PixelType> >
            observeColorBuffer() const;

            //! Set the color buffer type.
            void setColorBuffer(image::PixelType);

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace play
} // namespace tl
