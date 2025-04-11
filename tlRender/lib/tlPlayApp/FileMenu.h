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

        //! File menu.
        class FileMenu : public ui::Menu
        {
            TLRENDER_NON_COPYABLE(FileMenu);

        protected:
            void _init(
                const std::map<std::string, std::shared_ptr<ui::Action> >&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            FileMenu();

        public:
            ~FileMenu();

            static std::shared_ptr<FileMenu> create(
                const std::map<std::string, std::shared_ptr<ui::Action> >&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void close() override;

        private:
            void _filesUpdate(
                const std::vector<std::shared_ptr<play::FilesModelItem> >&);
            void _aUpdate(const std::shared_ptr<play::FilesModelItem>&);
            void _aIndexUpdate(int);
            void _layersUpdate(const std::vector<int>&);
            void _recentUpdate(const std::vector<file::Path>&);

            TLRENDER_PRIVATE();
        };
    } // namespace play_app
} // namespace tl
