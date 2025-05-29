// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/ToolsWidget.h>

#include <tlPlayApp/App.h>
#include <tlPlayApp/AudioTool.h>
#include <tlPlayApp/ColorTool.h>
#include <tlPlayApp/DevicesTool.h>
#include <tlPlayApp/FilesTool.h>
#include <tlPlayApp/InfoTool.h>
#include <tlPlayApp/MessagesTool.h>
#include <tlPlayApp/SettingsTool.h>
#include <tlPlayApp/SystemLogTool.h>
#include <tlPlayApp/ViewTool.h>

#include <tlUI/RowLayout.h>
#include <tlUI/StackLayout.h>

namespace tl
{
    namespace play_app
    {
        struct ToolsWidget::Private
        {
            std::map<Tool, std::shared_ptr<IToolWidget> > toolWidgets;
            std::shared_ptr<ui::StackLayout> layout;
            std::shared_ptr<observer::ValueObserver<int> > activeObserver;
        };

        void ToolsWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init("tl::play_app::ToolsWidget", context, parent);
            TLRENDER_P();

            p.toolWidgets[Tool::Audio] = AudioTool::create(app, context);
            p.toolWidgets[Tool::Color] = ColorTool::create(app, context);
            p.toolWidgets[Tool::Devices] = DevicesTool::create(app, context);
            p.toolWidgets[Tool::Files] = FilesTool::create(app, context);
            p.toolWidgets[Tool::Info] = InfoTool::create(app, context);
            p.toolWidgets[Tool::Messages] = MessagesTool::create(app, context);
            p.toolWidgets[Tool::Settings] = SettingsTool::create(app, context);
            p.toolWidgets[Tool::SystemLog] =
                SystemLogTool::create(app, context);
            p.toolWidgets[Tool::View] = ViewTool::create(app, context);

            p.layout = ui::StackLayout::create(context, shared_from_this());
            for (const auto& widget : p.toolWidgets)
            {
                widget.second->setParent(p.layout);
            }

            p.activeObserver = observer::ValueObserver<int>::create(
                app->getToolsModel()->observeActiveTool(),
                [this](int value)
                {
                    _p->layout->setCurrentIndex(value);
                    setVisible(value != -1);
                });
        }

        ToolsWidget::ToolsWidget() :
            _p(new Private)
        {
        }

        ToolsWidget::~ToolsWidget() {}

        std::shared_ptr<ToolsWidget> ToolsWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ToolsWidget>(new ToolsWidget);
            out->_init(app, context, parent);
            return out;
        }

        void ToolsWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void ToolsWidget::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        std::map<Tool, bool> ToolsWidget::_getToolsVisible() const
        {
            TLRENDER_P();
            std::map<Tool, bool> out;
            for (const auto& i : p.toolWidgets)
            {
                out[i.first] = i.second->isVisible(false);
            }
            return out;
        }
    } // namespace play_app
} // namespace tl
