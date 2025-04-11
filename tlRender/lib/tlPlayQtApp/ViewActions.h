// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <QAction>
#include <QObject>
#include <QMenu>

#include <memory>

namespace tl
{
    namespace play_qt
    {
        class App;
        class MainWindow;

        //! View actions.
        class ViewActions : public QObject
        {
            Q_OBJECT

        public:
            ViewActions(App*, MainWindow*, QObject* parent = nullptr);

            virtual ~ViewActions();

            //! Get the actions.
            const QMap<QString, QAction*>& actions() const;

            //! Get the menu.
            QMenu* menu() const;

        private:
            void _actionsUpdate();

            TLRENDER_PRIVATE();
        };
    } // namespace play_qt
} // namespace tl
