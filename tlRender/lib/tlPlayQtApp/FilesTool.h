// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlPlayQtApp/IToolWidget.h>

#include <tlPlay/FilesModel.h>

#include <QDockWidget>

namespace tl
{
    namespace play_qt
    {
        class App;

        //! Files tool.
        class FilesTool : public IToolWidget
        {
            Q_OBJECT

        public:
            FilesTool(App*, QWidget* parent = nullptr);

            virtual ~FilesTool();

        private:
            void _filesUpdate(
                const std::vector<std::shared_ptr<play::FilesModelItem> >&);
            void _aUpdate(const std::shared_ptr<play::FilesModelItem>&);
            void _bUpdate(
                const std::vector<std::shared_ptr<play::FilesModelItem> >&);
            void _layersUpdate(const std::vector<int>&);
            void _compareUpdate(const timeline::CompareOptions&);
            void _thumbnailsUpdate();

            TLRENDER_PRIVATE();
        };

        //! Files tool dock widget.
        class FilesDockWidget : public QDockWidget
        {
            Q_OBJECT

        public:
            FilesDockWidget(FilesTool*, QWidget* parent = nullptr);
        };
    } // namespace play_qt
} // namespace tl
