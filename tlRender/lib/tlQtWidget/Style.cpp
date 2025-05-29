// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlQtWidget/Style.h>

namespace tl
{
    namespace qtwidget
    {
        QPalette darkStyle()
        {
            QPalette palette;
            palette.setColor(QPalette::ColorRole::Window, QColor(51, 51, 51));
            palette.setColor(
                QPalette::ColorRole::WindowText, QColor(255, 255, 255));
            palette.setColor(QPalette::ColorRole::Base, QColor(43, 43, 43));
            palette.setColor(
                QPalette::ColorRole::AlternateBase, QColor(53, 53, 53));
            palette.setColor(QPalette::ColorRole::Text, QColor(255, 255, 255));
            palette.setColor(QPalette::ColorRole::Button, QColor(76, 76, 76));
            palette.setColor(
                QPalette::ColorRole::ButtonText, QColor(255, 255, 255));
            palette.setColor(
                QPalette::ColorRole::BrightText, QColor(255, 255, 255));
            palette.setColor(QPalette::ColorRole::Light, QColor(50, 50, 50));
            palette.setColor(QPalette::ColorRole::Midlight, QColor(45, 45, 45));
            palette.setColor(QPalette::ColorRole::Dark, QColor(30, 30, 30));
            palette.setColor(QPalette::ColorRole::Mid, QColor(35, 35, 35));
            palette.setColor(QPalette::ColorRole::Shadow, QColor(0, 0, 0));
            palette.setColor(
                QPalette::ColorRole::Highlight, QColor(160, 120, 60));
            palette.setColor(
                QPalette::ColorRole::HighlightedText, QColor(240, 240, 240));
            return palette;
        }

        QString styleSheet()
        {
            return "QToolBar {\n"
                   "   border-top: 1px solid palette(shadow);\n"
                   "   border-bottom: 1px solid palette(shadow);\n"
                   "   border-left: none;\n"
                   "   border-right: none;\n"
                   "}"
                   "QDockWidget {\n"
                   "    titlebar-close-icon: "
                   "url(:/Icons/DockWidgetClose.svg);\n"
                   "    titlebar-normal-icon: "
                   "url(:/Icons/DockWidgetNormal.svg);\n"
                   "}\n";
        }
    } // namespace qtwidget
} // namespace tl
