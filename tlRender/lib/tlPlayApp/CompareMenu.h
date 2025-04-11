// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Menu.h>

#include <tlPlay/FilesModel.h>

namespace tl
{
    namespace play_app
    {
        class App;

        //! Compare menu.
        class CompareMenu : public ui::Menu
        {
            TLRENDER_NON_COPYABLE(CompareMenu);

        protected:
            void _init(
                const std::map<std::string, std::shared_ptr<ui::Action> >&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            CompareMenu();

        public:
            ~CompareMenu();

            static std::shared_ptr<CompareMenu> create(
                const std::map<std::string, std::shared_ptr<ui::Action> >&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void close() override;

        private:
            void _filesUpdate(
                const std::vector<std::shared_ptr<play::FilesModelItem> >&);
            void _bUpdate(const std::vector<int>&);
            void _compareUpdate(const timeline::CompareOptions&);
            void _compareTimeUpdate(timeline::CompareTimeMode);

            TLRENDER_PRIVATE();
        };
    } // namespace play_app
} // namespace tl
