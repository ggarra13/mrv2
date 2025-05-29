// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Image.h>

#include <QAction>
#include <QObject>
#include <QMenu>

#include <memory>

namespace tl
{
    namespace play_qt
    {
        class App;

        //! Window actions.
        class WindowActions : public QObject
        {
            Q_OBJECT

        public:
            WindowActions(App*, QObject* parent = nullptr);

            virtual ~WindowActions();

            //! Get the actions.
            const QMap<QString, QAction*>& actions() const;

            //! Get the menu.
            QMenu* menu() const;

        Q_SIGNALS:
            //! This signal is emitted to resize the window.
            void resize(const tl::image::Size&);

        private:
            void _actionsUpdate();

            TLRENDER_PRIVATE();
        };
    } // namespace play_qt
} // namespace tl
