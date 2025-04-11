// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayApp/Tools.h>

#include <tlUI/IWidget.h>

namespace tl
{
    namespace play_app
    {
        class App;

        //! Tools widget.
        class ToolsWidget : public ui::IWidget
        {
            TLRENDER_NON_COPYABLE(ToolsWidget);

        protected:
            void _init(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            ToolsWidget();

        public:
            virtual ~ToolsWidget();

            //! Create a new widget.
            static std::shared_ptr<ToolsWidget> create(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;

        private:
            std::map<Tool, bool> _getToolsVisible() const;

            TLRENDER_PRIVATE();
        };
    } // namespace play_app
} // namespace tl
