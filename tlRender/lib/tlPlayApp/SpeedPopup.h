// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IMenuPopup.h>

namespace tl
{
    namespace play_app
    {
        //! Speed popup.
        class SpeedPopup : public ui::IMenuPopup
        {
            TLRENDER_NON_COPYABLE(SpeedPopup);

        protected:
            void _init(
                double defaultSpeed, const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            SpeedPopup();

        public:
            virtual ~SpeedPopup();

            static std::shared_ptr<SpeedPopup> create(
                double defaultSpeed, const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setCallback(const std::function<void(double)>&);

        private:
            void _menuUpdate();

            TLRENDER_PRIVATE();
        };
    } // namespace play_app
} // namespace tl
