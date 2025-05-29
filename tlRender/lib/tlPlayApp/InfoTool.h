// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/IToolWidget.h>

#include <tlTimeline/Player.h>

namespace tl
{
    namespace play_app
    {
        class App;

        //! Information tool.
        class InfoTool : public IToolWidget
        {
            TLRENDER_NON_COPYABLE(InfoTool);

        protected:
            void _init(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            InfoTool();

        public:
            virtual ~InfoTool();

            static std::shared_ptr<InfoTool> create(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };
    } // namespace play_app
} // namespace tl
