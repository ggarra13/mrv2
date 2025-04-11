// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/WindowToolBar.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>

#include <tlUI/RowLayout.h>
#include <tlUI/ToolButton.h>

namespace tl
{
    namespace play_app
    {
        struct WindowToolBar::Private
        {
            std::weak_ptr<App> app;

            std::map<std::string, std::shared_ptr<ui::Action> > actions;
            std::map<std::string, std::shared_ptr<ui::ToolButton> > buttons;
            std::shared_ptr<ui::HorizontalLayout> layout;

            std::shared_ptr<observer::ValueObserver<bool> > fullScreenObserver;
            std::shared_ptr<observer::ValueObserver<bool> > secondaryObserver;
        };

        void WindowToolBar::_init(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::play_app::WindowToolBar", context, parent);
            TLRENDER_P();

            p.app = app;
            p.actions = actions;

            p.buttons["FullScreen"] = ui::ToolButton::create(context);
            p.buttons["FullScreen"]->setIcon(p.actions["FullScreen"]->icon);
            p.buttons["FullScreen"]->setCheckable(
                p.actions["FullScreen"]->checkable);
            p.buttons["FullScreen"]->setToolTip(
                p.actions["FullScreen"]->toolTip);

            p.buttons["Secondary"] = ui::ToolButton::create(context);
            p.buttons["Secondary"]->setIcon(p.actions["Secondary"]->icon);
            p.buttons["Secondary"]->setCheckable(
                p.actions["Secondary"]->checkable);
            p.buttons["Secondary"]->setToolTip(p.actions["Secondary"]->toolTip);

            p.layout =
                ui::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(ui::SizeRole::None);
            p.buttons["FullScreen"]->setParent(p.layout);
            p.buttons["Secondary"]->setParent(p.layout);

            auto mainWindowWeak = std::weak_ptr<MainWindow>(mainWindow);
            p.buttons["FullScreen"]->setCheckedCallback(
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->setFullScreen(value);
                    }
                });
            auto appWeak = std::weak_ptr<App>(app);
            p.buttons["Secondary"]->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->setSecondaryWindow(value);
                    }
                });

            p.fullScreenObserver = observer::ValueObserver<bool>::create(
                mainWindow->observeFullScreen(), [this](bool value)
                { _p->buttons["FullScreen"]->setChecked(value); });

            p.secondaryObserver = observer::ValueObserver<bool>::create(
                app->observeSecondaryWindow(), [this](bool value)
                { _p->buttons["Secondary"]->setChecked(value); });
        }

        WindowToolBar::WindowToolBar() :
            _p(new Private)
        {
        }

        WindowToolBar::~WindowToolBar() {}

        std::shared_ptr<WindowToolBar> WindowToolBar::create(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<WindowToolBar>(new WindowToolBar);
            out->_init(actions, mainWindow, app, context, parent);
            return out;
        }

        void WindowToolBar::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void WindowToolBar::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }
    } // namespace play_app
} // namespace tl
