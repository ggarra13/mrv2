// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/ViewTool.h>

#include <tlTimeline/BackgroundOptions.h>

namespace tl
{
    namespace play_app
    {
        class BackgroundWidget : public ui::IWidget
        {
            TLRENDER_NON_COPYABLE(BackgroundWidget);

        protected:
            void _init(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            BackgroundWidget();

        public:
            virtual ~BackgroundWidget();

            static std::shared_ptr<BackgroundWidget> create(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;

        private:
            void _optionsUpdate(const timeline::BackgroundOptions&);

            TLRENDER_PRIVATE();
        };
    } // namespace play_app
} // namespace tl
