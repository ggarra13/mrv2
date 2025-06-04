// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/BackgroundOptions.h>
#include <tlTimeline/DisplayOptions.h>

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

        //! Viewport model.
        class ViewportModel : public std::enable_shared_from_this<ViewportModel>
        {
            TLRENDER_NON_COPYABLE(ViewportModel);

        protected:
            void _init(
                const std::shared_ptr<Settings>&,
                const std::shared_ptr<system::Context>&);

            ViewportModel();

        public:
            ~ViewportModel();

            //! Create a new model.
            static std::shared_ptr<ViewportModel> create(
                const std::shared_ptr<Settings>&,
                const std::shared_ptr<system::Context>&);

            //! Get the display options.
            const timeline::DisplayOptions& getDisplayOptions() const;

            //! Observe the display options.
            std::shared_ptr<observer::IValue<timeline::DisplayOptions> >
            observeDisplayOptions() const;

            //! Set the display options.
            void setDisplayOptions(const timeline::DisplayOptions&);

            //! Get the background options.
            const timeline::BackgroundOptions& getBackgroundOptions() const;

            //! Observe the background options.
            std::shared_ptr<observer::IValue<timeline::BackgroundOptions> >
            observeBackgroundOptions() const;

            //! Set the background options.
            void setBackgroundOptions(const timeline::BackgroundOptions&);

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace play
} // namespace tl
