// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlQtWidget/FileWidget.h>

#include <tlQtWidget/FileBrowserSystem.h>

#include <tlCore/Context.h>

#include <QBoxLayout>
#include <QLineEdit>
#include <QToolButton>

namespace tl
{
    namespace qtwidget
    {
        struct FileWidget::Private
        {
            std::weak_ptr<system::Context> context;
            QString fileName;
            QLineEdit* lineEdit = nullptr;
        };

        FileWidget::FileWidget(
            const std::shared_ptr<system::Context>& context, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.context = context;

            p.lineEdit = new QLineEdit;
            p.lineEdit->setToolTip(tr("File"));

            auto browseButton = new QToolButton;
            browseButton->setIcon(QIcon(":/Icons/FileBrowser.svg"));
            browseButton->setAutoRaise(true);
            browseButton->setToolTip(tr("Show the file browser"));

            auto clearButton = new QToolButton;
            clearButton->setIcon(QIcon(":/Icons/Clear.svg"));
            clearButton->setAutoRaise(true);
            clearButton->setToolTip(tr("Clear the file"));

            auto layout = new QHBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->setSpacing(1);
            layout->addWidget(p.lineEdit);
            layout->addWidget(browseButton);
            layout->addWidget(clearButton);
            setLayout(layout);

            _widgetUpdate();

            connect(
                p.lineEdit, &QLineEdit::editingFinished,
                [this] { setFile(_p->lineEdit->text()); });

            connect(
                browseButton, &QToolButton::clicked,
                [this]
                {
                    if (auto context = _p->context.lock())
                    {
                        if (auto fileBrowserSystem =
                                context->getSystem<FileBrowserSystem>())
                        {
                            fileBrowserSystem->open(
                                window(),
                                [this](const file::Path& value) {
                                    setFile(
                                        QString::fromUtf8(value.get().c_str()));
                                });
                        }
                    }
                });

            connect(clearButton, &QToolButton::clicked, [this] { clear(); });
        }

        FileWidget::~FileWidget() {}

        void FileWidget::setFile(const QString& value)
        {
            TLRENDER_P();
            if (value == p.fileName)
                return;
            p.fileName = value;
            _widgetUpdate();
            Q_EMIT fileChanged(_p->fileName);
        }

        void FileWidget::clear()
        {
            setFile(QString());
        }

        void FileWidget::_widgetUpdate()
        {
            TLRENDER_P();
            {
                QSignalBlocker signalBlocker(p.lineEdit);
                p.lineEdit->setText(p.fileName);
            }
        }
    } // namespace qtwidget
} // namespace tl
