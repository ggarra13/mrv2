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

        //! File actions.
        //!
        //! \todo Add actions for opening the next/previous file in the
        //! directory.
        class FileActions : public std::enable_shared_from_this<FileActions>
        {
            TLRENDER_NON_COPYABLE(FileActions);

        protected:
            void _init(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

            FileActions();

        public:
            ~FileActions();

            static std::shared_ptr<FileActions> create(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

            const std::map<std::string, std::shared_ptr<ui::Action> >&
            getActions() const;

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace play_app
} // namespace tl
