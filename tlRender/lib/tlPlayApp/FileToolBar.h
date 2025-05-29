// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/Action.h>
#include <tlUI/IWidget.h>

#include <tlPlay/FilesModel.h>

namespace tl
{
    namespace play_app
    {
        class App;

        //! File tool bar.
        class FileToolBar : public ui::IWidget
        {
            TLRENDER_NON_COPYABLE(FileToolBar);

        protected:
            void _init(
                const std::map<std::string, std::shared_ptr<ui::Action> >&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            FileToolBar();

        public:
            ~FileToolBar();

            static std::shared_ptr<FileToolBar> create(
                const std::map<std::string, std::shared_ptr<ui::Action> >&,
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const ui::SizeHintEvent&) override;

        private:
            void _filesUpdate(
                const std::vector<std::shared_ptr<play::FilesModelItem> >&);

            TLRENDER_PRIVATE();
        };
    } // namespace play_app
} // namespace tl
