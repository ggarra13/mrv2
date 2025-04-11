// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/IToolWidget.h>

#include <tlPlayApp/App.h>

#include <tlUI/Icon.h>
#include <tlUI/Label.h>
#include <tlUI/ToolButton.h>
#include <tlUI/RowLayout.h>

namespace tl
{
    namespace play_app
    {
        struct IToolWidget::Private
        {
            Tool tool;
            std::shared_ptr<ui::Icon> icon;
            std::shared_ptr<ui::Label> label;
            std::shared_ptr<ui::ToolButton> closeButton;
            std::shared_ptr<ui::VerticalLayout> toolLayout;
            std::shared_ptr<ui::VerticalLayout> layout;
        };

        void IToolWidget::_init(
            Tool tool, const std::string& objectName,
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(objectName, context, parent);
            TLRENDER_P();

            _app = app;
            p.tool = tool;

            p.icon = ui::Icon::create(getIcon(tool), context);
            p.icon->setMarginRole(ui::SizeRole::MarginSmall);

            p.label = ui::Label::create(getText(tool), context);
            p.label->setMarginRole(ui::SizeRole::MarginSmall);
            p.label->setHStretch(ui::Stretch::Expanding);

            p.closeButton = ui::ToolButton::create(context);
            p.closeButton->setIcon("Close");

            p.layout = ui::VerticalLayout::create(context, shared_from_this());
            p.layout->setSpacingRole(ui::SizeRole::None);
            auto hLayout = ui::HorizontalLayout::create(context, p.layout);
            hLayout->setSpacingRole(ui::SizeRole::None);
            // hLayout->setBackgroundRole(ui::ColorRole::Button);
            p.icon->setParent(hLayout);
            p.label->setParent(hLayout);
            p.closeButton->setParent(hLayout);
            p.toolLayout = ui::VerticalLayout::create(context, p.layout);
            p.toolLayout->setSpacingRole(ui::SizeRole::None);
            p.toolLayout->setHStretch(ui::Stretch::Expanding);
            p.toolLayout->setVStretch(ui::Stretch::Expanding);

            auto appWeak = std::weak_ptr<App>(app);
            p.closeButton->setClickedCallback(
                [appWeak, tool]
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getToolsModel()->setActiveTool(-1);
                    }
                });
        }

        IToolWidget::IToolWidget() :
            _p(new Private)
        {
        }

        IToolWidget::~IToolWidget() {}

        void IToolWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void IToolWidget::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IWidget::sizeHintEvent(event);
            _sizeHint = _p->layout->getSizeHint();
        }

        void IToolWidget::_setWidget(const std::shared_ptr<ui::IWidget>& value)
        {
            value->setHStretch(ui::Stretch::Expanding);
            value->setVStretch(ui::Stretch::Expanding);
            value->setParent(_p->toolLayout);
        }
    } // namespace play_app
} // namespace tl
