// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlQtWidget/SearchWidget.h>

#include <QBoxLayout>
#include <QLineEdit>
#include <QToolButton>

namespace tl
{
    namespace qtwidget
    {
        struct SearchWidget::Private
        {
            QLineEdit* lineEdit = nullptr;
            QToolButton* clearButton = nullptr;
        };

        SearchWidget::SearchWidget(QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.lineEdit = new QLineEdit;
            p.lineEdit->setToolTip(tr("Search"));

            p.clearButton = new QToolButton;
            p.clearButton->setIcon(QIcon(":/Icons/Clear.svg"));
            p.clearButton->setAutoRaise(true);
            p.clearButton->setToolTip(tr("Clear the search"));

            auto layout = new QHBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(1);
            layout->addWidget(p.lineEdit);
            layout->addWidget(p.clearButton);
            setLayout(layout);

            _widgetUpdate();

            connect(
                p.lineEdit, &QLineEdit::textChanged,
                [this](const QString& value)
                {
                    Q_EMIT searchChanged(value);
                    _widgetUpdate();
                });

            connect(p.clearButton, &QToolButton::clicked, [this] { clear(); });
        }

        SearchWidget::~SearchWidget() {}

        void SearchWidget::clear()
        {
            _p->lineEdit->clear();
        }

        void SearchWidget::_widgetUpdate()
        {
            TLRENDER_P();
            p.clearButton->setEnabled(!p.lineEdit->text().isEmpty());
        }
    } // namespace qtwidget
} // namespace tl
