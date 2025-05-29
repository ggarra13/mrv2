// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/IToolWidget.h>

namespace tl
{
    namespace play_app
    {
        //! Settings tool.
        class SettingsTool : public IToolWidget
        {
            TLRENDER_NON_COPYABLE(SettingsTool);

        protected:
            void _init(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            SettingsTool();

        public:
            virtual ~SettingsTool();

            static std::shared_ptr<SettingsTool> create(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace play_app
} // namespace tl
