// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQt/TimeObject.h>

#include <QAbstractSpinBox>

#include <memory>

namespace tl
{
    namespace qtwidget
    {
        //! Time spin box.
        class TimeSpinBox : public QAbstractSpinBox
        {
            Q_OBJECT
            Q_PROPERTY(otime::RationalTime value READ value WRITE setValue
                           NOTIFY valueChanged)
            Q_PROPERTY(tl::timeline::TimeUnits timeUnits READ timeUnits WRITE
                           setTimeUnits NOTIFY timeUnitsChanged)

        public:
            TimeSpinBox(QWidget* parent = nullptr);

            virtual ~TimeSpinBox();

            //! Set the time object.
            void setTimeObject(qt::TimeObject*);

            //! Get the time value.
            const otime::RationalTime& value() const;

            //! Get the time units.
            timeline::TimeUnits timeUnits() const;

            void stepBy(int steps) override;
            QValidator::State validate(QString&, int& pos) const override;
            QSize minimumSizeHint() const override;

        public Q_SLOTS:
            //! Set the time value.
            void setValue(const otime::RationalTime&);

            //! Set the time units.
            void setTimeUnits(tl::timeline::TimeUnits);

        Q_SIGNALS:
            //! This signal is emitted when the time is changed.
            void valueChanged(const otime::RationalTime&);

            //! This signal is emitted when the time units are changed.
            void timeUnitsChanged(tl::timeline::TimeUnits);

        protected:
            QAbstractSpinBox::StepEnabled stepEnabled() const override;

        private Q_SLOTS:
            void _lineEditCallback();

        private:
            void _vaidatorUpdate();
            void _textUpdate();

            TLRENDER_PRIVATE();
        };
    } // namespace qtwidget
} // namespace tl
