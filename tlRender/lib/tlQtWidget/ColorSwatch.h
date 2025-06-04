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
        //! Color swatch.
        class ColorSwatch : public QWidget
        {
            Q_OBJECT

        public:
            ColorSwatch(QWidget* parent = nullptr);

            virtual ~ColorSwatch();

            //! Get the color.
            const image::Color4f& color() const;

            //! Set the size of the swatch.
            void setSwatchSize(int);

            //! Set whether the color is editable.
            void setEditable(bool);

            QSize minimumSizeHint() const override;

        public Q_SLOTS:
            //! Set the color.
            void setColor(const tl::image::Color4f&);

        Q_SIGNALS:
            //! This signal is emitted when the color is changed.
            void colorChanged(const tl::image::Color4f&);

        protected:
            void paintEvent(QPaintEvent*) override;
            void mousePressEvent(QMouseEvent*) override;
            void mouseReleaseEvent(QMouseEvent*) override;

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace qtwidget
} // namespace tl
