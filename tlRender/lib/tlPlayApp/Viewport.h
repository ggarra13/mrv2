// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimelineUI/TimelineViewport.h>

namespace tl
{
    namespace play_app
    {
        class Viewport : public timelineui::TimelineViewport
        {
            TLRENDER_NON_COPYABLE(Viewport);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            Viewport();

        public:
            virtual ~Viewport();

            static std::shared_ptr<Viewport> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            bool hasHUD() const;
            std::shared_ptr<observer::IValue<bool> > observeHUD() const;
            void setHUD(bool);

            void sizeHintEvent(const ui::SizeHintEvent&) override;
            void clipEvent(const math::Box2i&, bool) override;
            void drawEvent(const math::Box2i&, const ui::DrawEvent&) override;

        private:
            void _textUpdate();

            TLRENDER_PRIVATE();
        };
    } // namespace play_app
} // namespace tl
