// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQtWidget/TimelineViewport.h>

namespace tl
{
    namespace play_qt
    {
        //! Viewport widget.
        class Viewport : public qtwidget::TimelineViewport
        {
            Q_OBJECT

        public:
            Viewport(
                const std::shared_ptr<system::Context>&,
                QWidget* parent = nullptr);

            virtual ~Viewport();

            bool hasHUD() const;

        public Q_SLOTS:
            void setHUD(bool);

        Q_SIGNALS:
            void hudChanged(bool);

        protected:
            void paintGL() override;

        private:
            void _textUpdate();

            TLRENDER_PRIVATE();
        };
    } // namespace play_qt
} // namespace tl
