// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlQt/ToolTipsFilter.h>

#include <QEvent>

namespace tl
{
    namespace qt
    {
        ToolTipsFilter::ToolTipsFilter(QObject* parent) :
            QObject(parent)
        {
        }

        bool ToolTipsFilter::eventFilter(QObject* watched, QEvent* event)
        {
            if (event->type() == QEvent::Type::ToolTip)
            {
                return true;
            }
            return false;
        }
    } // namespace qt
} // namespace tl
