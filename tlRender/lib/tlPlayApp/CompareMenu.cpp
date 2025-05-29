// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/CompareMenu.h>

#include <tlPlayApp/App.h>

namespace tl
{
    namespace play_app
    {
        struct CompareMenu::Private
        {
            std::weak_ptr<App> app;

            std::map<std::string, std::shared_ptr<ui::Action> > actions;
            std::vector<std::shared_ptr<ui::Action> > bActions;
            std::map<std::string, std::shared_ptr<ui::Menu> > menus;

            std::shared_ptr<
                observer::ListObserver<std::shared_ptr<play::FilesModelItem> > >
                filesObserver;
            std::shared_ptr<observer::ListObserver<int> > bIndexesObserver;
            std::shared_ptr<observer::ValueObserver<timeline::CompareOptions> >
                compareOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::CompareTimeMode> >
                compareTimeObserver;
        };

        void CompareMenu::_init(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            Menu::_init(context, parent);
            TLRENDER_P();

            p.app = app;
            p.actions = actions;

            p.menus["B"] = addSubMenu("B");
            addItem(p.actions["Next"]);
            addItem(p.actions["Prev"]);
            addDivider();
            const auto compareLabels = timeline::getCompareModeLabels();
            for (const auto& label : compareLabels)
            {
                addItem(p.actions[label]);
            }
            addDivider();
            p.menus["Time"] = addSubMenu("Time");
            const auto timeLabels = timeline::getCompareTimeModeLabels();
            for (const auto& label : timeLabels)
            {
                p.menus["Time"]->addItem(p.actions[label]);
            }

            p.filesObserver = observer::
                ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                    app->getFilesModel()->observeFiles(),
                    [this](const std::vector<
                           std::shared_ptr<play::FilesModelItem> >& value)
                    { _filesUpdate(value); });

            p.bIndexesObserver = observer::ListObserver<int>::create(
                app->getFilesModel()->observeBIndexes(),
                [this](const std::vector<int>& value) { _bUpdate(value); });

            p.compareOptionsObserver =
                observer::ValueObserver<timeline::CompareOptions>::create(
                    app->getFilesModel()->observeCompareOptions(),
                    [this](const timeline::CompareOptions& value)
                    { _compareUpdate(value); });

            p.compareTimeObserver =
                observer::ValueObserver<timeline::CompareTimeMode>::create(
                    app->getFilesModel()->observeCompareTime(),
                    [this](timeline::CompareTimeMode value)
                    { _compareTimeUpdate(value); });
        }

        CompareMenu::CompareMenu() :
            _p(new Private)
        {
        }

        CompareMenu::~CompareMenu() {}

        std::shared_ptr<CompareMenu> CompareMenu::create(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<CompareMenu>(new CompareMenu);
            out->_init(actions, app, context, parent);
            return out;
        }

        void CompareMenu::close()
        {
            Menu::close();
            TLRENDER_P();
            for (const auto& menu : p.menus)
            {
                menu.second->close();
            }
        }

        void CompareMenu::_filesUpdate(
            const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
        {
            TLRENDER_P();

            setItemEnabled(p.actions["Next"], value.size() > 1);
            setItemEnabled(p.actions["Prev"], value.size() > 1);

            p.menus["B"]->clear();
            p.bActions.clear();
            if (auto app = p.app.lock())
            {
                const auto bIndexes = app->getFilesModel()->getBIndexes();
                for (size_t i = 0; i < value.size(); ++i)
                {
                    auto action = std::make_shared<ui::Action>(
                        value[i]->path.get(-1, file::PathType::FileName),
                        [this, i]
                        {
                            close();
                            if (auto app = _p->app.lock())
                            {
                                app->getFilesModel()->toggleB(i);
                            }
                        });
                    const auto j =
                        std::find(bIndexes.begin(), bIndexes.end(), i);
                    action->checked = j != bIndexes.end();
                    p.menus["B"]->addItem(action);
                    p.bActions.push_back(action);
                }
            }
        }

        void CompareMenu::_bUpdate(const std::vector<int>& value)
        {
            TLRENDER_P();
            for (int i = 0; i < p.bActions.size(); ++i)
            {
                const auto j = std::find(value.begin(), value.end(), i);
                p.menus["B"]->setItemChecked(p.bActions[i], j != value.end());
            }
        }

        void CompareMenu::_compareUpdate(const timeline::CompareOptions& value)
        {
            TLRENDER_P();
            const auto enums = timeline::getCompareModeEnums();
            const auto labels = timeline::getCompareModeLabels();
            for (size_t i = 0; i < enums.size(); ++i)
            {
                setItemChecked(p.actions[labels[i]], enums[i] == value.mode);
            }
        }

        void CompareMenu::_compareTimeUpdate(timeline::CompareTimeMode value)
        {
            TLRENDER_P();
            const auto enums = timeline::getCompareTimeModeEnums();
            const auto labels = timeline::getCompareTimeModeLabels();
            for (size_t i = 0; i < enums.size(); ++i)
            {
                p.menus["Time"]->setItemChecked(
                    p.actions[labels[i]], enums[i] == value);
            }
        }
    } // namespace play_app
} // namespace tl
