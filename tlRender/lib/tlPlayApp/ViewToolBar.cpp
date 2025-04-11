// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/ViewToolBar.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/MainWindow.h>
#include <tlPlayApp/Viewport.h>

#include <tlUI/RowLayout.h>
#include <tlUI/ToolButton.h>

namespace tl
{
    namespace play_app
    {
        struct ViewToolBar::Private
        {
            std::weak_ptr<App> app;

            std::map<std::string, std::shared_ptr<ui::Action> > actions;
            std::map<std::string, std::shared_ptr<ui::ToolButton> > buttons;
            std::shared_ptr<ui::HorizontalLayout> layout;

            std::shared_ptr<observer::ValueObserver<bool> > frameViewObserver;
        };

        void ViewToolBar::_init(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::play_app::ViewToolBar", context, parent);
            TLRENDER_P();

            p.app = app;
            p.actions = actions;

            p.buttons["Frame"] = ui::ToolButton::create(context);
            p.buttons["Frame"]->setIcon(p.actions["Frame"]->icon);
            p.buttons["Frame"]->setCheckable(p.actions["Frame"]->checkable);
            p.buttons["Frame"]->setToolTip(p.actions["Frame"]->toolTip);

            p.buttons["Zoom1To1"] = ui::ToolButton::create(context);
            p.buttons["Zoom1To1"]->setIcon(p.actions["Zoom1To1"]->icon);
            p.buttons["Zoom1To1"]->setToolTip(p.actions["Zoom1To1"]->toolTip);

            p.layout =
                ui::HorizontalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(ui::SizeRole::None);
            p.buttons["Frame"]->setParent(p.layout);
            p.buttons["Zoom1To1"]->setParent(p.layout);

            auto mainWindowWeak = std::weak_ptr<MainWindow>(mainWindow);
            p.buttons["Frame"]->setCheckedCallback(
                [mainWindowWeak](bool value)
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getViewport()->setFrameView(value);
                    }
                });

            p.buttons["Zoom1To1"]->setClickedCallback(
                [mainWindowWeak]
                {
                    if (auto mainWindow = mainWindowWeak.lock())
                    {
                        mainWindow->getViewport()->viewZoom1To1();
                    }
                });

            p.frameViewObserver = observer::ValueObserver<bool>::create(
                mainWindow->getViewport()->observeFrameView(),
                [this](bool value)
                { _p->buttons["Frame"]->setChecked(value); });
        }

        ViewToolBar::ViewToolBar() :
            _p(new Private)
        {
        }

        ViewToolBar::~ViewToolBar() {}

        std::shared_ptr<ViewToolBar> ViewToolBar::create(
            const std::map<std::string, std::shared_ptr<ui::Action> >& actions,
            const std::shared_ptr<MainWindow>& mainWindow,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ViewToolBar>(new ViewToolBar);
            out->_init(actions, mainWindow, app, context, parent);
            return out;
        }

        void ViewToolBar::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void ViewToolBar::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }
    } // namespace play_app
} // namespace tl
