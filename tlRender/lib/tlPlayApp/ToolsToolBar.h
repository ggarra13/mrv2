// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Action.h>
#include <tlUI/IWidget.h>

#include <tlTimeline/Player.h>

namespace tl
{
    namespace play_app
    {
        class App;

        //! Tools tool bar.
        class ToolsToolBar : public ui::IWidget
        {
            TLRENDER_NON_COPYABLE(ToolsToolBar);

        protected:
            void _init(
                const std::map<std::string, std::shared_ptr<ui::Action> >&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            ToolsToolBar();

        public:
            ~ToolsToolBar();

            static std::shared_ptr<ToolsToolBar> create(
                const std::map<std::string, std::shared_ptr<ui::Action> >&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace play_app
} // namespace tl
