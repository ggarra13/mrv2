// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidgetPopup.h>

namespace tl
{
    namespace play_app
    {
        class App;

        //! Audio popup.
        class AudioPopup : public ui::IWidgetPopup
        {
            TLRENDER_NON_COPYABLE(AudioPopup);

        protected:
            void _init(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            AudioPopup();

        public:
            virtual ~AudioPopup();

            static std::shared_ptr<AudioPopup> create(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace play_app
} // namespace tl
