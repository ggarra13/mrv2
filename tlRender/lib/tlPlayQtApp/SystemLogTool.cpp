// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/SystemLogTool.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/DockTitleBar.h>

#include <tlQtWidget/Util.h>

#include <QAction>
#include <QBoxLayout>
#include <QClipboard>
#include <QListWidget>
#include <QToolButton>

namespace tl
{
    namespace play_qt
    {
        namespace
        {
            const int messagesMax = 100;
        }

        struct SystemLogTool::Private
        {
            QListWidget* listWidget = nullptr;
            QToolButton* copyButton = nullptr;
            QToolButton* clearButton = nullptr;
            std::shared_ptr<observer::ListObserver<log::Item> > logObserver;
        };

        SystemLogTool::SystemLogTool(App* app, QWidget* parent) :
            IToolWidget(app, parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.listWidget = new QListWidget;
            p.listWidget->setHorizontalScrollMode(
                QAbstractItemView::ScrollPerPixel);
            p.listWidget->setVerticalScrollMode(
                QAbstractItemView::ScrollPerPixel);
            const QFont fixedFont("Noto Mono");
            p.listWidget->setFont(fixedFont);

            p.copyButton = new QToolButton;
            p.copyButton->setText("Copy");
            p.copyButton->setAutoRaise(true);
            p.copyButton->setToolTip(tr("Copy the contents to the clipboard"));

            p.clearButton = new QToolButton;
            p.clearButton->setText("Clear");
            p.clearButton->setAutoRaise(true);
            p.clearButton->setToolTip(tr("Clear the contents"));

            auto layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(0);
            layout->addWidget(p.listWidget);
            auto hLayout = new QHBoxLayout;
            hLayout->setSpacing(1);
            hLayout->addWidget(p.copyButton);
            hLayout->addWidget(p.clearButton);
            hLayout->addStretch();
            layout->addLayout(hLayout);
            auto widget = new QWidget;
            widget->setLayout(layout);
            addWidget(widget);

            p.logObserver = observer::ListObserver<log::Item>::create(
                app->getContext()->getLogSystem()->observeLog(),
                [this](const std::vector<log::Item>& value)
                {
                    const size_t options =
                        static_cast<size_t>(log::StringConvert::Time) |
                        static_cast<size_t>(log::StringConvert::Prefix);
                    for (const auto& i : value)
                    {
                        _p->listWidget->addItem(QString::fromUtf8(
                            log::toString(i, options).c_str()));
                        while (_p->listWidget->count() > messagesMax)
                        {
                            delete _p->listWidget->takeItem(0);
                        }
                    }
                });

            connect(
                p.copyButton, &QToolButton::clicked,
                [this]
                {
                    auto clipboard = QGuiApplication::clipboard();
                    QStringList text;
                    for (int i = 0; i < _p->listWidget->count(); ++i)
                    {
                        text.append(_p->listWidget->item(i)->text());
                    }
                    clipboard->setText(text.join('\n'));
                });

            connect(
                p.clearButton, &QToolButton::clicked, p.listWidget,
                &QListWidget::clear);
        }

        SystemLogTool::~SystemLogTool() {}

        SystemLogDockWidget::SystemLogDockWidget(
            SystemLogTool* systemLogTool, QWidget* parent)
        {
            setObjectName("SystemLogTool");
            setWindowTitle(tr("System Log"));
            setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

            auto dockTitleBar = new DockTitleBar;
            dockTitleBar->setText(tr("System Log"));
            auto dockWidget = new QDockWidget;
            setTitleBarWidget(dockTitleBar);

            setWidget(systemLogTool);

            toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F9));
            toggleViewAction()->setToolTip(tr("Show system log"));
        }
    } // namespace play_qt
} // namespace tl
