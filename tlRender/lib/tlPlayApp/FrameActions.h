// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Action.h>

namespace tl
{
    namespace play_app
    {
        class App;
        class MainWindow;

        //! Frame actions.
        class FrameActions : public std::enable_shared_from_this<FrameActions>
        {
            TLRENDER_NON_COPYABLE(FrameActions);

        protected:
            void _init(
                const std::shared_ptr<MainWindow>&, const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

            FrameActions();

        public:
            ~FrameActions();

            static std::shared_ptr<FrameActions> create(
                const std::shared_ptr<MainWindow>&, const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

            const std::map<std::string, std::shared_ptr<ui::Action> >&
            getActions() const;

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace play_app
} // namespace tl
