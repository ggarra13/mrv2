// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/CompareActions.h>

#include <tlPlayQtApp/App.h>

#include <tlQt/MetaTypes.h>

#include <tlPlay/FilesModel.h>

#include <QActionGroup>

namespace tl
{
    namespace play_qt
    {
        struct CompareActions::Private
        {
            App* app = nullptr;

            QMap<QString, QAction*> actions;
            QMap<QString, QActionGroup*> actionGroups;

            QScopedPointer<QMenu> menu;
            QScopedPointer<QMenu> bMenu;
            QScopedPointer<QMenu> timeMenu;

            std::shared_ptr<
                observer::ListObserver<std::shared_ptr<play::FilesModelItem> > >
                filesObserver;
            std::shared_ptr<observer::ListObserver<int> > bIndexesObserver;
            std::shared_ptr<observer::ValueObserver<timeline::CompareOptions> >
                compareOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::CompareTimeMode> >
                compareTimeObserver;
        };

        CompareActions::CompareActions(App* app, QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

            p.actions["Next"] = new QAction(this);
            p.actions["Next"]->setText(tr("Next"));
            p.actions["Next"]->setIcon(QIcon(":/Icons/Next.svg"));
            p.actions["Next"]->setShortcut(
                QKeySequence(Qt::SHIFT | Qt::Key_PageDown));
            p.actions["Next"]->setToolTip(tr("Change to the next file"));

            p.actions["Prev"] = new QAction(this);
            p.actions["Prev"]->setText(tr("Previous"));
            p.actions["Prev"]->setIcon(QIcon(":/Icons/Prev.svg"));
            p.actions["Prev"]->setShortcut(
                QKeySequence(Qt::SHIFT | Qt::Key_PageUp));
            p.actions["Prev"]->setToolTip(tr("Change to the previous file"));

            p.actions["A"] = new QAction(this);
            p.actions["A"]->setData(QVariant::fromValue<timeline::CompareMode>(
                timeline::CompareMode::A));
            p.actions["A"]->setCheckable(true);
            p.actions["A"]->setText(tr("A"));
            p.actions["A"]->setIcon(QIcon(":/Icons/CompareA.svg"));
            p.actions["A"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_A));
            p.actions["A"]->setToolTip(tr("Show the A file"));

            p.actions["B"] = new QAction(this);
            p.actions["B"]->setData(QVariant::fromValue<timeline::CompareMode>(
                timeline::CompareMode::B));
            p.actions["B"]->setCheckable(true);
            p.actions["B"]->setText(tr("B"));
            p.actions["B"]->setIcon(QIcon(":/Icons/CompareB.svg"));
            p.actions["B"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_B));
            p.actions["B"]->setToolTip(tr("Show the B file"));

            p.actions["Wipe"] = new QAction(this);
            p.actions["Wipe"]->setData(
                QVariant::fromValue<timeline::CompareMode>(
                    timeline::CompareMode::Wipe));
            p.actions["Wipe"]->setCheckable(true);
            p.actions["Wipe"]->setText(tr("Wipe"));
            p.actions["Wipe"]->setIcon(QIcon(":/Icons/CompareWipe.svg"));
            p.actions["Wipe"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_W));
            p.actions["Wipe"]->setToolTip(
                tr("Wipe between the A and B files\n\n"
                   "Use the Alt key + left mouse button to move the wipe"));

            p.actions["Overlay"] = new QAction(this);
            p.actions["Overlay"]->setData(
                QVariant::fromValue<timeline::CompareMode>(
                    timeline::CompareMode::Overlay));
            p.actions["Overlay"]->setCheckable(true);
            p.actions["Overlay"]->setText(tr("Overlay"));
            p.actions["Overlay"]->setIcon(QIcon(":/Icons/CompareOverlay.svg"));
            p.actions["Overlay"]->setToolTip(
                tr("Show the A file over the B file with transparency"));

            p.actions["Difference"] = new QAction(this);
            p.actions["Difference"]->setData(
                QVariant::fromValue<timeline::CompareMode>(
                    timeline::CompareMode::Difference));
            p.actions["Difference"]->setCheckable(true);
            p.actions["Difference"]->setText(tr("Difference"));
            p.actions["Difference"]->setIcon(
                QIcon(":/Icons/CompareDifference.svg"));
            p.actions["Difference"]->setToolTip(
                tr("Show the difference between the A and B files"));

            p.actions["Horizontal"] = new QAction(this);
            p.actions["Horizontal"]->setData(
                QVariant::fromValue<timeline::CompareMode>(
                    timeline::CompareMode::Horizontal));
            p.actions["Horizontal"]->setCheckable(true);
            p.actions["Horizontal"]->setText(tr("Horizontal"));
            p.actions["Horizontal"]->setIcon(
                QIcon(":/Icons/CompareHorizontal.svg"));
            p.actions["Horizontal"]->setToolTip(
                tr("Show the A and B files side by side"));

            p.actions["Vertical"] = new QAction(this);
            p.actions["Vertical"]->setData(
                QVariant::fromValue<timeline::CompareMode>(
                    timeline::CompareMode::Vertical));
            p.actions["Vertical"]->setCheckable(true);
            p.actions["Vertical"]->setText(tr("Vertical"));
            p.actions["Vertical"]->setIcon(
                QIcon(":/Icons/CompareVertical.svg"));
            p.actions["Vertical"]->setToolTip(
                tr("Show the A file above the B file"));

            p.actions["Tile"] = new QAction(this);
            p.actions["Tile"]->setData(
                QVariant::fromValue<timeline::CompareMode>(
                    timeline::CompareMode::Tile));
            p.actions["Tile"]->setCheckable(true);
            p.actions["Tile"]->setText(tr("Tile"));
            p.actions["Tile"]->setIcon(QIcon(":/Icons/CompareTile.svg"));
            p.actions["Tile"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_T));
            p.actions["Tile"]->setToolTip(tr("Tile the A and B files"));

            p.actions["Relative"] = new QAction(this);
            p.actions["Relative"]->setData(
                QVariant::fromValue<timeline::CompareTimeMode>(
                    timeline::CompareTimeMode::Relative));
            p.actions["Relative"]->setCheckable(true);
            p.actions["Relative"]->setText(tr("Relative"));
            p.actions["Relative"]->setToolTip(tr("Compare relative times"));

            p.actions["Absolute"] = new QAction(this);
            p.actions["Absolute"]->setData(
                QVariant::fromValue<timeline::CompareTimeMode>(
                    timeline::CompareTimeMode::Absolute));
            p.actions["Absolute"]->setCheckable(true);
            p.actions["Absolute"]->setText(tr("Absolute"));
            p.actions["Absolute"]->setToolTip(tr("Compare absolute times"));

            p.actionGroups["B"] = new QActionGroup(this);

            p.actionGroups["Compare"] = new QActionGroup(this);
            p.actionGroups["Compare"]->setExclusive(true);
            p.actionGroups["Compare"]->addAction(p.actions["A"]);
            p.actionGroups["Compare"]->addAction(p.actions["B"]);
            p.actionGroups["Compare"]->addAction(p.actions["Wipe"]);
            p.actionGroups["Compare"]->addAction(p.actions["Overlay"]);
            p.actionGroups["Compare"]->addAction(p.actions["Difference"]);
            p.actionGroups["Compare"]->addAction(p.actions["Horizontal"]);
            p.actionGroups["Compare"]->addAction(p.actions["Vertical"]);
            p.actionGroups["Compare"]->addAction(p.actions["Tile"]);

            p.actionGroups["Time"] = new QActionGroup(this);
            p.actionGroups["Time"]->setExclusive(true);
            p.actionGroups["Time"]->addAction(p.actions["Relative"]);
            p.actionGroups["Time"]->addAction(p.actions["Absolute"]);

            p.menu.reset(new QMenu);
            p.menu->setTitle(tr("&Compare"));
            p.bMenu.reset(new QMenu);
            p.bMenu->setTitle(tr("&B"));
            p.menu->addMenu(p.bMenu.get());
            p.menu->addAction(p.actions["Next"]);
            p.menu->addAction(p.actions["Prev"]);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["A"]);
            p.menu->addAction(p.actions["B"]);
            p.menu->addAction(p.actions["Wipe"]);
            p.menu->addAction(p.actions["Overlay"]);
            p.menu->addAction(p.actions["Difference"]);
            p.menu->addAction(p.actions["Horizontal"]);
            p.menu->addAction(p.actions["Vertical"]);
            p.menu->addAction(p.actions["Tile"]);
            p.menu->addSeparator();
            p.timeMenu.reset(new QMenu);
            p.timeMenu->setTitle(tr("&Time"));
            p.menu->addMenu(p.timeMenu.get());
            p.timeMenu->addAction(p.actions["Relative"]);
            p.timeMenu->addAction(p.actions["Absolute"]);

            _actionsUpdate();

            connect(
                p.actions["Next"], &QAction::triggered,
                [app] { app->filesModel()->nextB(); });
            connect(
                p.actions["Prev"], &QAction::triggered,
                [app] { app->filesModel()->prevB(); });

            connect(
                p.actionGroups["B"], &QActionGroup::triggered,
                [this, app](QAction* action)
                {
                    const int index =
                        _p->actionGroups["B"]->actions().indexOf(action);
                    const auto bIndexes =
                        app->filesModel()->observeBIndexes()->get();
                    const auto i =
                        std::find(bIndexes.begin(), bIndexes.end(), index);
                    app->filesModel()->setB(index, i == bIndexes.end());
                });

            connect(
                p.actionGroups["Compare"], &QActionGroup::triggered,
                [this, app](QAction* action)
                {
                    auto options = app->filesModel()->getCompareOptions();
                    options.mode =
                        action->data().value<timeline::CompareMode>();
                    app->filesModel()->setCompareOptions(options);
                });

            connect(
                p.actionGroups["Time"], &QActionGroup::triggered,
                [this, app](QAction* action)
                {
                    const timeline::CompareTimeMode value =
                        action->data().value<timeline::CompareTimeMode>();
                    app->filesModel()->setCompareTime(value);
                });

            p.filesObserver = observer::
                ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                    app->filesModel()->observeFiles(),
                    [this](const std::vector<
                           std::shared_ptr<play::FilesModelItem> >&)
                    { _actionsUpdate(); });

            p.bIndexesObserver = observer::ListObserver<int>::create(
                app->filesModel()->observeBIndexes(),
                [this](const std::vector<int>&) { _actionsUpdate(); });

            p.compareOptionsObserver =
                observer::ValueObserver<timeline::CompareOptions>::create(
                    app->filesModel()->observeCompareOptions(),
                    [this](const timeline::CompareOptions&)
                    { _actionsUpdate(); });

            p.compareTimeObserver =
                observer::ValueObserver<timeline::CompareTimeMode>::create(
                    app->filesModel()->observeCompareTime(),
                    [this](timeline::CompareTimeMode) { _actionsUpdate(); });
        }

        CompareActions::~CompareActions() {}

        const QMap<QString, QAction*>& CompareActions::actions() const
        {
            return _p->actions;
        }

        QMenu* CompareActions::menu() const
        {
            return _p->menu.get();
        }

        void CompareActions::_actionsUpdate()
        {
            TLRENDER_P();

            for (auto i : p.actionGroups["B"]->actions())
            {
                p.actionGroups["B"]->removeAction(i);
            }
            p.bMenu->clear();
            const auto& files = p.app->filesModel()->observeFiles()->get();
            const auto bIndexes = p.app->filesModel()->observeBIndexes()->get();
            const size_t count = files.size();
            for (size_t i = 0; i < count; ++i)
            {
                auto action = new QAction(this);
                action->setCheckable(true);
                action->setChecked(
                    std::find(bIndexes.begin(), bIndexes.end(), i) !=
                    bIndexes.end());
                action->setText(QString::fromUtf8(
                    files[i]->path.get(-1, file::PathType::FileName).c_str()));
                p.actionGroups["B"]->addAction(action);
                p.bMenu->addAction(action);
            }

            {
                const auto options = p.app->filesModel()->getCompareOptions();
                QSignalBlocker blocker(p.actionGroups["Compare"]);
                for (auto action : p.actionGroups["Compare"]->actions())
                {
                    if (action->data().value<timeline::CompareMode>() ==
                        options.mode)
                    {
                        action->setChecked(true);
                        break;
                    }
                }
            }

            {
                const timeline::CompareTimeMode time =
                    p.app->filesModel()->getCompareTime();
                QSignalBlocker blocker(p.actionGroups["Time"]);
                for (auto action : p.actionGroups["Time"]->actions())
                {
                    if (action->data().value<timeline::CompareTimeMode>() ==
                        time)
                    {
                        action->setChecked(true);
                        break;
                    }
                }
            }
        }
    } // namespace play_qt
} // namespace tl
