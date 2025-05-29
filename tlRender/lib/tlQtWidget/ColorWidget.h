// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Color.h>
#include <tlCore/Util.h>

#include <QWidget>

namespace tl
{
    namespace qtwidget
    {
        //! Color widget.
        class ColorWidget : public QWidget
        {
            Q_OBJECT

        public:
            ColorWidget(QWidget* parent = nullptr);

            virtual ~ColorWidget();

            //! Get the color.
            const image::Color4f& color() const;

        public Q_SLOTS:
            //! Set the color.
            void setColor(const tl::image::Color4f&);

        Q_SIGNALS:
            //! This signal is emitted when the color is changed.
            void colorChanged(const tl::image::Color4f&);

        private:
            void _widgetUpdate();

            TLRENDER_PRIVATE();
        };
    } // namespace qtwidget
} // namespace tl
