// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlQtWidget/BellowsPrivate.h>

#include <QLabel>
#include <QVBoxLayout>

namespace tl
{
    namespace qtwidget
    {
        struct BellowsButton::Private
        {
            QLabel* iconLabel = nullptr;
            QLabel* textLabel = nullptr;
            bool open = false;
        };

        BellowsButton::BellowsButton(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
            setBackgroundRole(QPalette::Button);
            setAutoFillBackground(true);
            setMouseTracking(true);

            p.iconLabel = new QLabel;
            p.textLabel = new QLabel;

            auto layout = new QHBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(p.iconLabel);
            layout->addWidget(p.textLabel, 1);
            setLayout(layout);

            _widgetUpdate();
        }

        BellowsButton::~BellowsButton() {}

        QString BellowsButton::text() const
        {
            return _p->textLabel->text();
        }

        bool BellowsButton::isOpen() const
        {
            return _p->open;
        }

        void BellowsButton::setText(const QString& value)
        {
            _p->textLabel->setText(value);
        }

        void BellowsButton::setOpen(bool value)
        {
            TLRENDER_P();
            if (value == p.open)
                return;
            p.open = value;
            _widgetUpdate();
            Q_EMIT openChanged(p.open);
        }

        void BellowsButton::mousePressEvent(QMouseEvent*)
        {
            setOpen(!_p->open);
        }

        void BellowsButton::mouseReleaseEvent(QMouseEvent*) {}

        void BellowsButton::mouseMoveEvent(QMouseEvent*) {}

        void BellowsButton::_widgetUpdate()
        {
            TLRENDER_P();
            p.iconLabel->setPixmap(
                p.open ? QPixmap(":/Icons/BellowsOpen.svg")
                       : QPixmap(":/Icons/BellowsClosed.svg"));
        }
    } // namespace qtwidget
} // namespace tl
