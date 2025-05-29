// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlQtWidget/Util.h>

namespace tl
{
    namespace qtwidget
    {
        QSize toQt(const math::Size2i& value)
        {
            return QSize(value.w, value.h);
        }

        math::Size2i fromQt(const QSize& value)
        {
            return math::Size2i(value.width(), value.height());
        }

        QColor toQt(const image::Color4f& value)
        {
            return QColor::fromRgbF(value.r, value.g, value.b, value.a);
        }

        image::Color4f fromQt(const QColor& value)
        {
            return image::Color4f(
                value.redF(), value.greenF(), value.blueF(), value.alphaF());
        }

        void setFloatOnTop(bool value, QWidget* window)
        {
            if (value && !(window->windowFlags() & Qt::WindowStaysOnTopHint))
            {
                window->setWindowFlags(
                    window->windowFlags() | Qt::WindowStaysOnTopHint);
                window->show();
            }
            else if (
                !value && (window->windowFlags() & Qt::WindowStaysOnTopHint))
            {
                window->setWindowFlags(
                    window->windowFlags() & ~Qt::WindowStaysOnTopHint);
                window->show();
            }
        }
    } // namespace qtwidget
} // namespace tl
