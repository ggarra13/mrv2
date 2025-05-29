// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlQtWidget/ColorSwatch.h>

#include <tlQtWidget/ColorDialog.h>
#include <tlQtWidget/Util.h>

#include <QMouseEvent>
#include <QPainter>

namespace tl
{
    namespace qtwidget
    {
        struct ColorSwatch::Private
        {
            image::Color4f color;
            int swatchSize = 20;
            bool editable = false;
        };

        ColorSwatch::ColorSwatch(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
        }

        ColorSwatch::~ColorSwatch() {}

        const image::Color4f& ColorSwatch::color() const
        {
            return _p->color;
        }

        void ColorSwatch::setSwatchSize(int value)
        {
            TLRENDER_P();
            if (value == p.swatchSize)
                return;
            p.swatchSize = value;
            updateGeometry();
        }

        void ColorSwatch::setEditable(bool value)
        {
            _p->editable = value;
        }

        QSize ColorSwatch::minimumSizeHint() const
        {
            TLRENDER_P();
            return QSize(p.swatchSize, p.swatchSize);
        }

        void ColorSwatch::setColor(const tl::image::Color4f& value)
        {
            TLRENDER_P();
            if (value == p.color)
                return;
            p.color = value;
            update();
            Q_EMIT colorChanged(p.color);
        }

        void ColorSwatch::paintEvent(QPaintEvent*)
        {
            TLRENDER_P();
            QPainter painter(this);
            painter.fillRect(0, 0, width(), height(), toQt(p.color));
        }

        void ColorSwatch::mousePressEvent(QMouseEvent* event)
        {
            TLRENDER_P();
            if (p.editable)
            {
                event->accept();
                QScopedPointer<ColorDialog> dialog(new ColorDialog(p.color));
                if (QDialog::Accepted == dialog->exec())
                {
                    setColor(dialog->color());
                }
            }
        }

        void ColorSwatch::mouseReleaseEvent(QMouseEvent* event)
        {
            TLRENDER_P();
            if (p.editable)
            {
                event->accept();
            }
        }
    } // namespace qtwidget
} // namespace tl
