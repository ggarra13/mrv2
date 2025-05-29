// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlQtWidget/BellowsWidget.h>

#include <tlQtWidget/BellowsPrivate.h>

#include <QVBoxLayout>

namespace tl
{
    namespace qtwidget
    {
        struct BellowsWidget::Private
        {
            BellowsButton* button = nullptr;
            QWidget* widget = nullptr;
            QVBoxLayout* layout = nullptr;
        };

        BellowsWidget::BellowsWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.button = new BellowsButton;

            auto layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 1);
            layout->setSpacing(0);
            layout->addWidget(p.button);
            p.layout = new QVBoxLayout;
            layout->addLayout(p.layout);
            setLayout(layout);

            _widgetUpdate();

            connect(p.button, SIGNAL(openChanged(bool)), SLOT(_openCallback()));
        }

        BellowsWidget::~BellowsWidget() {}

        void BellowsWidget::setWidget(QWidget* widget)
        {
            TLRENDER_P();
            if (p.widget)
            {
                delete p.widget;
            }
            p.widget = widget;
            if (p.widget)
            {
                p.layout->addWidget(p.widget);
            }
            _widgetUpdate();
        }

        bool BellowsWidget::isOpen() const
        {
            return _p->button->isOpen();
        }

        QString BellowsWidget::title() const
        {
            return _p->button->text();
        }

        void BellowsWidget::setTitle(const QString& value)
        {
            _p->button->setText(value);
        }

        void BellowsWidget::setOpen(bool value)
        {
            _p->button->setOpen(value);
        }

        void BellowsWidget::_openCallback()
        {
            _widgetUpdate();
        }

        void BellowsWidget::_widgetUpdate()
        {
            TLRENDER_P();
            if (p.widget)
            {
                p.widget->setVisible(p.button->isOpen());
            }
        }
    } // namespace qtwidget
} // namespace tl
