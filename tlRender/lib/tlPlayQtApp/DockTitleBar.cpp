// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/DockTitleBar.h>

#include <QBoxLayout>
#include <QDockWidget>
#include <QLabel>
#include <QPainter>
#include <QToolButton>

namespace tl
{
    namespace play_qt
    {
        struct DockTitleBar::Private
        {
            QLabel* iconLabel = nullptr;
            QLabel* textLabel = nullptr;
            QToolButton* floatButton = nullptr;
            QToolButton* closeButton = nullptr;
        };

        DockTitleBar::DockTitleBar(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.iconLabel = new QLabel;

            p.textLabel = new QLabel;

            p.floatButton = new QToolButton;
            p.floatButton->setIconSize(QSize(12, 12));
            p.floatButton->setIcon(QIcon(":/Icons/DockWidgetNormal.svg"));
            p.floatButton->setAutoRaise(true);

            p.closeButton = new QToolButton;
            p.closeButton->setIconSize(QSize(12, 12));
            p.closeButton->setIcon(QIcon(":/Icons/DockWidgetClose.svg"));
            p.closeButton->setAutoRaise(true);

            auto layout = new QHBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(p.iconLabel);
            layout->addWidget(p.textLabel);
            layout->addStretch();
            auto buttonLayout = new QHBoxLayout;
            buttonLayout->setSpacing(1);
            buttonLayout->addWidget(p.floatButton);
            buttonLayout->addWidget(p.closeButton);
            layout->addLayout(buttonLayout);
            setLayout(layout);

            connect(
                p.floatButton, &QToolButton::clicked,
                [this]
                {
                    if (auto parent =
                            qobject_cast<QDockWidget*>(parentWidget()))
                    {
                        parent->setFloating(!parent->isFloating());
                    }
                });

            connect(
                p.closeButton, &QToolButton::clicked,
                [this]
                {
                    if (auto parent =
                            qobject_cast<QDockWidget*>(parentWidget()))
                    {
                        parent->close();
                    }
                });
        }

        DockTitleBar::~DockTitleBar() {}

        void DockTitleBar::setText(const QString& value)
        {
            _p->textLabel->setText(value);
        }

        void DockTitleBar::setIcon(const QIcon& value)
        {
            _p->iconLabel->setPixmap(value.pixmap(QSize(20, 20)));
        }

        void DockTitleBar::paintEvent(QPaintEvent*) {}
    } // namespace play_qt
} // namespace tl
