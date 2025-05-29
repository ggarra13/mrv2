// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/ToolActions.h>

#include <tlPlayQtApp/App.h>

namespace tl
{
    namespace play_qt
    {
        struct ToolActions::Private
        {
            App* app = nullptr;

            QMap<QString, QAction*> actions;
            QMap<QString, QActionGroup*> actionGroups;

            QScopedPointer<QMenu> menu;
        };

        ToolActions::ToolActions(App* app, QObject* parent) :
            QObject(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

            p.menu.reset(new QMenu);
            p.menu->setTitle(tr("&Tools"));

            _actionsUpdate();
        }

        ToolActions::~ToolActions() {}

        const QMap<QString, QAction*>& ToolActions::actions() const
        {
            return _p->actions;
        }

        QMenu* ToolActions::menu() const
        {
            return _p->menu.get();
        }

        void ToolActions::_actionsUpdate() {}
    } // namespace play_qt
} // namespace tl
