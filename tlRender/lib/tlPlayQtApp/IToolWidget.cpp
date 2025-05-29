// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/IToolWidget.h>

#include <tlQtWidget/BellowsWidget.h>

#include <QBoxLayout>
#include <QScrollArea>

namespace tl
{
    namespace play_qt
    {
        IToolWidget::IToolWidget(App* app, QWidget* parent) :
            QWidget(parent)
        {
            _layout = new QVBoxLayout;
            _layout->setContentsMargins(0, 0, 0, 0);
            _layout->setSpacing(0);
            auto scrollWidget = new QWidget;
            scrollWidget->setLayout(_layout);
            auto scrollArea = new QScrollArea;
            scrollArea->setWidgetResizable(true);
            scrollArea->setWidget(scrollWidget);
            auto scrollLayout = new QVBoxLayout;
            scrollLayout->addWidget(scrollArea);
            scrollLayout->setContentsMargins(0, 0, 0, 0);
            setLayout(scrollLayout);
        }

        void IToolWidget::addWidget(QWidget* widget, int stretch)
        {
            _layout->addWidget(widget, stretch);
        }

        void IToolWidget::addBellows(const QString& title, QWidget* widget)
        {
            auto bellowsWidget = new qtwidget::BellowsWidget;
            bellowsWidget->setTitle(title);
            bellowsWidget->setWidget(widget);
            _layout->addWidget(bellowsWidget);
        }

        void IToolWidget::addStretch(int stretch)
        {
            _layout->addStretch(stretch);
        }
    } // namespace play_qt
} // namespace tl
