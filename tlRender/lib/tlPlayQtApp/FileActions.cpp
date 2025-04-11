// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/FileActions.h>

#include <tlPlayQtApp/App.h>

#include <tlUI/RecentFilesModel.h>

#include <tlPlay/FilesModel.h>

#include <QActionGroup>

namespace tl
{
    namespace play_qt
    {
        struct FileActions::Private
        {
            App* app = nullptr;

            QMap<QString, QAction*> actions;
            QMap<QString, QActionGroup*> actionGroups;

            QScopedPointer<QMenu> menu;
            QScopedPointer<QMenu> recentMenu;
            QScopedPointer<QMenu> currentMenu;
            QScopedPointer<QMenu> layerMenu;

            std::shared_ptr<
                observer::ListObserver<std::shared_ptr<play::FilesModelItem> > >
                filesObserver;
            std::shared_ptr<observer::ValueObserver<int> > aIndexObserver;
            std::shared_ptr<observer::ListObserver<int> > layersObserver;
            std::shared_ptr<observer::ListObserver<file::Path> > recentObserver;
        };

        FileActions::FileActions(App* app, QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

            p.actions["Open"] = new QAction(this);
            p.actions["Open"]->setText(tr("Open"));
            p.actions["Open"]->setIcon(QIcon(":/Icons/FileOpen.svg"));
            p.actions["Open"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_O));
            p.actions["Open"]->setToolTip(tr("Open a file"));

            p.actions["OpenSeparateAudio"] = new QAction(this);
            p.actions["OpenSeparateAudio"]->setText(
                tr("Open With Separate Audio"));
            p.actions["OpenSeparateAudio"]->setIcon(
                QIcon(":/Icons/FileOpenSeparateAudio.svg"));
            p.actions["OpenSeparateAudio"]->setShortcut(
                QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_O));
            p.actions["OpenSeparateAudio"]->setToolTip(
                tr("Open a file with separate audio"));

            p.actions["Close"] = new QAction(this);
            p.actions["Close"]->setText(tr("Close"));
            p.actions["Close"]->setIcon(QIcon(":/Icons/FileClose.svg"));
            p.actions["Close"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_E));
            p.actions["Close"]->setToolTip(tr("Close the current file"));

            p.actions["CloseAll"] = new QAction(this);
            p.actions["CloseAll"]->setText(tr("Close All"));
            p.actions["CloseAll"]->setIcon(QIcon(":/Icons/FileCloseAll.svg"));
            p.actions["CloseAll"]->setShortcut(
                QKeySequence(Qt::CTRL | Qt::SHIFT | Qt::Key_E));
            p.actions["CloseAll"]->setToolTip(tr("Close all files"));

            p.actions["Reload"] = new QAction(this);
            p.actions["Reload"]->setText(tr("Reload"));
            p.actions["Reload"]->setToolTip(tr("Reload files"));

            p.actions["Next"] = new QAction(this);
            p.actions["Next"]->setText(tr("Next"));
            p.actions["Next"]->setIcon(QIcon(":/Icons/Next.svg"));
            p.actions["Next"]->setShortcut(
                QKeySequence(Qt::CTRL | Qt::Key_PageDown));
            p.actions["Next"]->setToolTip(tr("Change to the next file"));

            p.actions["Prev"] = new QAction(this);
            p.actions["Prev"]->setText(tr("Previous"));
            p.actions["Prev"]->setIcon(QIcon(":/Icons/Prev.svg"));
            p.actions["Prev"]->setShortcut(
                QKeySequence(Qt::CTRL | Qt::Key_PageUp));
            p.actions["Prev"]->setToolTip(tr("Change to the previous file"));

            p.actions["NextLayer"] = new QAction(this);
            p.actions["NextLayer"]->setText(tr("Next Layer"));
            p.actions["NextLayer"]->setShortcut(
                QKeySequence(Qt::CTRL | Qt::Key_Equal));
            p.actions["NextLayer"]->setToolTip(tr("Change to the next layer"));

            p.actions["PrevLayer"] = new QAction(this);
            p.actions["PrevLayer"]->setText(tr("Previous Layer"));
            p.actions["PrevLayer"]->setShortcut(
                QKeySequence(Qt::CTRL | Qt::Key_Minus));
            p.actions["PrevLayer"]->setToolTip(
                tr("Change to the previous layer"));

            p.actions["Exit"] = new QAction(this);
            p.actions["Exit"]->setText(tr("Exit"));
            p.actions["Exit"]->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Q));

            p.actionGroups["Recent"] = new QActionGroup(this);

            p.actionGroups["Current"] = new QActionGroup(this);
            p.actionGroups["Current"]->setExclusive(true);

            p.actionGroups["Layer"] = new QActionGroup(this);
            p.actionGroups["Layer"]->setExclusive(true);

            p.menu.reset(new QMenu);
            p.menu->setTitle(tr("&File"));
            p.menu->addAction(p.actions["Open"]);
            p.menu->addAction(p.actions["OpenSeparateAudio"]);
            p.menu->addAction(p.actions["Close"]);
            p.menu->addAction(p.actions["CloseAll"]);
            p.menu->addAction(p.actions["Reload"]);
            p.recentMenu.reset(new QMenu);
            p.recentMenu->setTitle(tr("&Recent"));
            p.menu->addMenu(p.recentMenu.get());
            p.menu->addSeparator();
            p.currentMenu.reset(new QMenu);
            p.currentMenu->setTitle(tr("&Current"));
            p.menu->addMenu(p.currentMenu.get());
            p.menu->addAction(p.actions["Next"]);
            p.menu->addAction(p.actions["Prev"]);
            p.menu->addSeparator();
            p.layerMenu.reset(new QMenu);
            p.layerMenu->setTitle(tr("&Layer"));
            p.menu->addMenu(p.layerMenu.get());
            p.menu->addAction(p.actions["NextLayer"]);
            p.menu->addAction(p.actions["PrevLayer"]);
            p.menu->addSeparator();
            p.menu->addAction(p.actions["Exit"]);

            _actionsUpdate();

            connect(
                p.actions["Open"], &QAction::triggered, app, &App::openDialog);
            connect(
                p.actions["OpenSeparateAudio"], &QAction::triggered, app,
                &App::openSeparateAudioDialog);
            connect(
                p.actions["Close"], &QAction::triggered,
                [app] { app->filesModel()->close(); });
            connect(
                p.actions["CloseAll"], &QAction::triggered,
                [app] { app->filesModel()->closeAll(); });
            connect(
                p.actions["Reload"], &QAction::triggered,
                [app] { app->filesModel()->reload(); });
            connect(
                p.actions["Next"], &QAction::triggered,
                [app] { app->filesModel()->next(); });
            connect(
                p.actions["Prev"], &QAction::triggered,
                [app] { app->filesModel()->prev(); });
            connect(
                p.actions["NextLayer"], &QAction::triggered,
                [app] { app->filesModel()->nextLayer(); });
            connect(
                p.actions["PrevLayer"], &QAction::triggered,
                [app] { app->filesModel()->prevLayer(); });
            connect(p.actions["Exit"], &QAction::triggered, app, &App::quit);

            connect(
                p.actionGroups["Recent"], &QActionGroup::triggered,
                [app](QAction* action)
                { app->open(action->data().toString()); });

            connect(
                p.actionGroups["Current"], &QActionGroup::triggered,
                [this, app](QAction* action)
                {
                    const int index =
                        _p->actionGroups["Current"]->actions().indexOf(action);
                    app->filesModel()->setA(index);
                });

            connect(
                p.actionGroups["Layer"], &QActionGroup::triggered,
                [this, app](QAction* action)
                {
                    const int index =
                        _p->actionGroups["Layer"]->actions().indexOf(action);
                    const auto& a = app->filesModel()->getA();
                    app->filesModel()->setLayer(a, index);
                });

            p.filesObserver = observer::
                ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                    app->filesModel()->observeFiles(),
                    [this](const std::vector<
                           std::shared_ptr<play::FilesModelItem> >&)
                    { _actionsUpdate(); });

            p.aIndexObserver = observer::ValueObserver<int>::create(
                app->filesModel()->observeAIndex(),
                [this](int) { _actionsUpdate(); });

            p.layersObserver = observer::ListObserver<int>::create(
                app->filesModel()->observeLayers(),
                [this](const std::vector<int>&) { _actionsUpdate(); });

            p.recentObserver = observer::ListObserver<file::Path>::create(
                app->recentFilesModel()->observeRecent(),
                [this](const std::vector<file::Path>& value)
                { _recentUpdate(value); });
        }

        FileActions::~FileActions() {}

        const QMap<QString, QAction*>& FileActions::actions() const
        {
            return _p->actions;
        }

        QMenu* FileActions::menu() const
        {
            return _p->menu.get();
        }

        void FileActions::_recentUpdate(const std::vector<file::Path>& value)
        {
            TLRENDER_P();
            for (const auto& i : p.actionGroups["Recent"]->actions())
            {
                delete i;
            }
            p.recentMenu->clear();
            for (auto i = value.rbegin(); i != value.rend(); ++i)
            {
                auto action = new QAction(this);
                const QString label = QString::fromUtf8(i->get().c_str());
                action->setText(QString("%1").arg(label));
                const QString fileName = QString::fromUtf8(i->get().c_str());
                action->setData(fileName);
                p.actionGroups["Recent"]->addAction(action);
                p.recentMenu->addAction(action);
            }
        }

        void FileActions::_actionsUpdate()
        {
            TLRENDER_P();

            const auto& files = p.app->filesModel()->observeFiles()->get();
            const size_t count = files.size();
            p.actions["Close"]->setEnabled(count > 0);
            p.actions["CloseAll"]->setEnabled(count > 0);
            p.actions["Reload"]->setEnabled(count > 0);
            p.actions["Next"]->setEnabled(count > 1);
            p.actions["Prev"]->setEnabled(count > 1);
            p.actions["NextLayer"]->setEnabled(count > 0);
            p.actions["PrevLayer"]->setEnabled(count > 0);

            for (auto i : p.actionGroups["Current"]->actions())
            {
                p.actionGroups["Current"]->removeAction(i);
            }
            p.currentMenu->clear();
            const int aIndex = p.app->filesModel()->observeAIndex()->get();
            for (size_t i = 0; i < files.size(); ++i)
            {
                auto action = new QAction(this);
                action->setCheckable(true);
                action->setChecked(i == aIndex);
                action->setText(QString::fromUtf8(
                    files[i]->path.get(-1, file::PathType::FileName).c_str()));
                p.actionGroups["Current"]->addAction(action);
                p.currentMenu->addAction(action);
            }

            for (auto i : p.actionGroups["Layer"]->actions())
            {
                p.actionGroups["Layer"]->removeAction(i);
            }
            p.layerMenu->clear();
            if (const auto& a = p.app->filesModel()->getA())
            {
                for (size_t i = 0; i < a->videoLayers.size(); ++i)
                {
                    auto action = new QAction(this);
                    action->setCheckable(true);
                    action->setChecked(i == a->videoLayer);
                    action->setText(
                        QString::fromUtf8(a->videoLayers[i].c_str()));
                    p.actionGroups["Layer"]->addAction(action);
                    p.layerMenu->addAction(action);
                }
            }
        }
    } // namespace play_qt
} // namespace tl
