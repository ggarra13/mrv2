// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Util.h>

#include <QWidget>

#include <memory>

namespace tl
{
    namespace play_qt
    {
        //! Dock widget title bar.
        class DockTitleBar : public QWidget
        {
            Q_OBJECT

        public:
            DockTitleBar(QWidget* parent = nullptr);

            virtual ~DockTitleBar();

        public Q_SLOTS:
            //! Set the title bar text.
            void setText(const QString&);

            //! Set the title bar icon.
            void setIcon(const QIcon&);

        protected:
            void paintEvent(QPaintEvent*) override;

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace play_qt
} // namespace tl
