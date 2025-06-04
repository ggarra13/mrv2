// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlTimeline/IRender.h>

#include <QAction>
#include <QObject>
#include <QMenu>

#include <memory>

namespace tl
{
    namespace play_qt
    {
        class App;

        //! Render actions.
        class RenderActions : public QObject
        {
            Q_OBJECT

        public:
            RenderActions(App*, QObject* parent = nullptr);

            virtual ~RenderActions();

            const std::vector<image::PixelType>& getColorBuffers() const;

            const QMap<QString, QAction*>& actions() const;

            QMenu* menu() const;

        private Q_SLOTS:

        private:
            void _actionsUpdate();

            TLRENDER_PRIVATE();
        };
    } // namespace play_qt
} // namespace tl
