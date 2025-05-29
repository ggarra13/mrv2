// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimeObject.h>

#include <QWidget>

#include <memory>

namespace tl
{
    namespace qtwidget
    {
        //! Speed label.
        class SpeedLabel : public QWidget
        {
            Q_OBJECT
            Q_PROPERTY(otime::RationalTime value READ value WRITE setValue)

        public:
            SpeedLabel(QWidget* parent = nullptr);

            virtual ~SpeedLabel();

            //! Get the speed value.
            const otime::RationalTime& value() const;

        public Q_SLOTS:
            //! Set the speed value.
            void setValue(const otime::RationalTime&);

        private:
            void _textUpdate();

            TLRENDER_PRIVATE();
        };
    } // namespace qtwidget
} // namespace tl
