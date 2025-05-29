// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Path.h>

#include <QAction>
#include <QObject>
#include <QMenu>

namespace tl
{
    namespace play_qt
    {
        class App;

        //! File actions.
        class FileActions : public QObject
        {
            Q_OBJECT

        public:
            FileActions(App*, QObject* parent = nullptr);

            virtual ~FileActions();

            //! Get the actions.
            const QMap<QString, QAction*>& actions() const;

            //! Get the menu.
            QMenu* menu() const;

        private:
            void _recentUpdate(const std::vector<file::Path>&);
            void _actionsUpdate();

            TLRENDER_PRIVATE();
        };
    } // namespace play_qt
} // namespace tl
