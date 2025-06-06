// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/MessagesTool.h>

#include <tlUI/IClipboard.h>
#include <tlUI/IWindow.h>
#include <tlUI/Label.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollWidget.h>
#include <tlUI/ToolButton.h>

#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play_app
    {
        namespace
        {
            const int messagesMax = 20;
        }

        struct MessagesTool::Private
        {
            std::list<std::string> messages;
            std::shared_ptr<ui::Label> label;
            std::shared_ptr<ui::ScrollWidget> scrollWidget;
            std::shared_ptr<ui::ToolButton> copyButton;
            std::shared_ptr<ui::ToolButton> clearButton;
            std::shared_ptr<ui::VerticalLayout> layout;
            std::shared_ptr<tl::observer::ListObserver<tl::log::Item> >
                logObserver;
        };

        void MessagesTool::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::Messages, "tl::play_app::MessagesTool", app, context,
                parent);
            TLRENDER_P();

            p.label = ui::Label::create(context);
            p.label->setFontRole(ui::FontRole::Mono);
            p.label->setMarginRole(ui::SizeRole::MarginSmall);
            p.label->setVAlign(ui::VAlign::Top);

            p.scrollWidget = ui::ScrollWidget::create(context);
            p.scrollWidget->setWidget(p.label);
            p.scrollWidget->setVStretch(ui::Stretch::Expanding);

            p.copyButton = ui::ToolButton::create("Copy", context);

            p.clearButton = ui::ToolButton::create("Clear", context);

            p.layout = ui::VerticalLayout::create(context);
            p.layout->setSpacingRole(ui::SizeRole::kNone);
            p.scrollWidget->setParent(p.layout);
            auto hLayout = ui::HorizontalLayout::create(context, p.layout);
            hLayout->setMarginRole(ui::SizeRole::MarginInside);
            hLayout->setSpacingRole(ui::SizeRole::SpacingTool);
            p.copyButton->setParent(hLayout);
            p.clearButton->setParent(hLayout);
            _setWidget(p.layout);

            p.copyButton->setClickedCallback(
                [this]
                {
                    if (auto window = getWindow())
                    {
                        if (auto clipboard = window->getClipboard())
                        {
                            const std::string text = string::join(
                                {std::begin(_p->messages),
                                 std::end(_p->messages)},
                                '\n');
                            clipboard->setText(text);
                        }
                    }
                });

            p.clearButton->setClickedCallback(
                [this]
                {
                    _p->messages.clear();
                    _p->label->setText(std::string());
                });

            p.logObserver = tl::observer::ListObserver<tl::log::Item>::create(
                context->getSystem<tl::log::System>()->observeLog(),
                [this](const std::vector<log::Item>& value)
                {
                    for (const auto& i : value)
                    {
                        switch (i.type)
                        {
                        case log::Type::Warning:
                        case log::Type::Error:
                            _p->messages.push_back(log::toString(i));
                            break;
                        default:
                            break;
                        }
                    }
                    while (_p->messages.size() > messagesMax)
                    {
                        _p->messages.pop_front();
                    }
                    _p->label->setText(string::join(
                        {std::begin(_p->messages), std::end(_p->messages)},
                        '\n'));
                });
        }

        MessagesTool::MessagesTool() :
            _p(new Private)
        {
        }

        MessagesTool::~MessagesTool() {}

        std::shared_ptr<MessagesTool> MessagesTool::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<MessagesTool>(new MessagesTool);
            out->_init(app, context, parent);
            return out;
        }
    } // namespace play_app
} // namespace tl
