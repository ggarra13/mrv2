// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/InfoTool.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/DockTitleBar.h>
#include <tlPlayQtApp/InfoModel.h>

#include <tlQtWidget/SearchWidget.h>

#include <tlIO/IO.h>

#include <QAction>
#include <QBoxLayout>
#include <QHeaderView>
#include <QLineEdit>
#include <QSortFilterProxyModel>
#include <QToolButton>
#include <QTreeView>

namespace tl
{
    namespace play_qt
    {
        struct InfoTool::Private
        {
            TagsModel* tagsModel = nullptr;
            QSortFilterProxyModel* tagsProxyModel = nullptr;

            QTreeView* tagsView = nullptr;
            qtwidget::SearchWidget* tagsSearchWidget = nullptr;
        };

        InfoTool::InfoTool(App* app, QWidget* parent) :
            IToolWidget(app, parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.tagsModel = new TagsModel(this);
            p.tagsProxyModel = new QSortFilterProxyModel(this);
            p.tagsProxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
            p.tagsProxyModel->setFilterKeyColumn(-1);
            p.tagsProxyModel->setSourceModel(p.tagsModel);

            p.tagsView = new QTreeView;
            p.tagsView->setAllColumnsShowFocus(true);
            p.tagsView->setAlternatingRowColors(true);
            p.tagsView->setSelectionMode(QAbstractItemView::NoSelection);
            p.tagsView->setHorizontalScrollMode(
                QAbstractItemView::ScrollPerPixel);
            p.tagsView->setVerticalScrollMode(
                QAbstractItemView::ScrollPerPixel);
            p.tagsView->setIndentation(0);
            p.tagsView->setModel(p.tagsProxyModel);

            p.tagsSearchWidget = new qtwidget::SearchWidget;

            auto widget = new QWidget;
            auto layout = new QVBoxLayout;
            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(p.tagsView);
            layout->addWidget(p.tagsSearchWidget);
            widget->setLayout(layout);
            addWidget(widget);

            addStretch();

            connect(
                p.tagsSearchWidget, SIGNAL(searchChanged(const QString&)),
                p.tagsProxyModel, SLOT(setFilterFixedString(const QString&)));
        }

        InfoTool::~InfoTool() {}

        void InfoTool::setInfo(const io::Info& value)
        {
            TLRENDER_P();
            p.tagsModel->setTags(value.tags);
        }

        InfoDockWidget::InfoDockWidget(InfoTool* infoTool, QWidget* parent)
        {
            setObjectName("InfoTool");
            setWindowTitle(tr("Information"));
            setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

            auto dockTitleBar = new DockTitleBar;
            dockTitleBar->setText(tr("Information"));
            dockTitleBar->setIcon(QIcon(":/Icons/Info.svg"));
            auto dockWidget = new QDockWidget;
            setTitleBarWidget(dockTitleBar);

            setWidget(infoTool);

            toggleViewAction()->setIcon(QIcon(":/Icons/Info.svg"));
            toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F4));
            toggleViewAction()->setToolTip(tr("Show information"));
        }
    } // namespace play_qt
} // namespace tl
