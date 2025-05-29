// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Range.h>
#include <tlCore/Util.h>

#include <QWidget>

#include <memory>

namespace tl
{
    namespace qtwidget
    {
        //! Integer value editor and slider.
        class IntEditSlider : public QWidget
        {
            Q_OBJECT
            Q_PROPERTY(tl::math::IntRange range READ range WRITE setRange NOTIFY
                           rangeChanged)
            Q_PROPERTY(int value READ value WRITE setValue NOTIFY valueChanged)
            Q_PROPERTY(int defaultValue READ defaultValue WRITE setDefaultValue)
            Q_PROPERTY(int singleStep READ singleStep WRITE setSingleStep)
            Q_PROPERTY(int pageStep READ pageStep WRITE setPageStep)
            Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE
                           setOrientation)

        public:
            IntEditSlider(
                Qt::Orientation = Qt::Horizontal, QWidget* parent = nullptr);

            virtual ~IntEditSlider();

            //! Get the range.
            const math::IntRange& range() const;

            //! Get the value.
            int value() const;

            //! Get the default value.
            int defaultValue() const;

            //! Get the single step.
            int singleStep() const;

            //! Get the path step.
            int pageStep() const;

            //! Get the orientation.
            Qt::Orientation orientation() const;

        public slots:
            //! Set the range.
            void setRange(const tl::math::IntRange&);

            //! Set the value.
            void setValue(int);

            //! Set the default value.
            void setDefaultValue(int);

            //! Set the single step.
            void setSingleStep(int);

            //! Set the page step.
            void setPageStep(int);

            //! Set the orientation.
            void setOrientation(Qt::Orientation);

        Q_SIGNALS:
            //! This signal is emitted when the range is changed.
            void rangeChanged(const tl::math::IntRange&);

            //! This signal is emitted when the value is changed.
            void valueChanged(int);

        private:
            void _layoutUpdate();
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };
    } // namespace qtwidget
} // namespace tl
