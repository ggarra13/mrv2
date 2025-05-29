// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlQtWidget/Separator.h>

namespace tl
{
    namespace qtwidget
    {
        Separator::Separator(Qt::Orientation orientation, QWidget* parent) :
            QFrame(parent),
            _orientation(orientation)
        {
            setForegroundRole(QPalette::Mid);
            _widgetUpdate();
        }

        Separator::~Separator() {}

        void Separator::setOrientation(Qt::Orientation value)
        {
            if (value == _orientation)
                return;
            _orientation = value;
            _widgetUpdate();
        }

        void Separator::_widgetUpdate()
        {
            setFrameShape(Qt::Horizontal ? QFrame::HLine : QFrame::VLine);
        }
    } // namespace qtwidget
} // namespace tl
