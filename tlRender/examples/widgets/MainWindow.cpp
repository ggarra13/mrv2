// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "MainWindow.h"

#include "BasicWidgets.h"
#include "Charts.h"
#include "DragAndDrop.h"
#include "GridLayouts.h"
#include "MDI.h"
#include "NumericWidgets.h"
#include "RowLayouts.h"
#include "ScrollAreas.h"

#include <tlUI/ButtonGroup.h>
#include <tlUI/ListButton.h>
#include <tlUI/PushButton.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollWidget.h>
#include <tlUI/StackLayout.h>

namespace tl
{
    namespace examples
    {
        namespace widgets
        {
            struct MainWindow::Private
            {
                std::map<std::string, std::shared_ptr<IExampleWidget> > widgets;
                std::map<std::string, std::shared_ptr<ui::ListButton> > buttons;
                std::shared_ptr<ui::ButtonGroup> buttonGroup;
                std::shared_ptr<ui::RowLayout> layout;
                std::shared_ptr<ui::StackLayout> stackLayout;
            };

            void
            MainWindow::_init(const std::shared_ptr<system::Context>& context)
            {
                Window::_init("widgets", context, nullptr);
                TLRENDER_P();

                setBackgroundRole(ui::ColorRole::Window);

                std::shared_ptr<IExampleWidget> widget = nullptr;
                widget = BasicWidgets::create(context);
                p.widgets[widget->getExampleName()] = widget;
                widget = Charts::create(context);
                p.widgets[widget->getExampleName()] = widget;
                widget = DragAndDrop::create(context);
                p.widgets[widget->getExampleName()] = widget;
                widget = GridLayouts::create(context);
                p.widgets[widget->getExampleName()] = widget;
                widget = MDI::create(context);
                p.widgets[widget->getExampleName()] = widget;
                widget = NumericWidgets::create(context);
                p.widgets[widget->getExampleName()] = widget;
                widget = RowLayouts::create(context);
                p.widgets[widget->getExampleName()] = widget;
                widget = ScrollAreas::create(context);
                p.widgets[widget->getExampleName()] = widget;

                p.buttonGroup = ui::ButtonGroup::create(
                    ui::ButtonGroupType::Click, context);
                for (const auto& i : p.widgets)
                {
                    auto button = ui::ListButton::create(context);
                    const std::string& exampleName = i.second->getExampleName();
                    button->setText(exampleName);
                    p.buttons[exampleName] = button;
                    p.buttonGroup->addButton(button);
                }
                p.buttonGroup->setClickedCallback(
                    [this](int value)
                    { _p->stackLayout->setCurrentIndex(value); });

                p.layout =
                    ui::HorizontalLayout::create(context, shared_from_this());
                p.layout->setMarginRole(ui::SizeRole::Margin);
                p.layout->setSpacingRole(ui::SizeRole::Spacing);
                auto scrollWidget = ui::ScrollWidget::create(
                    context, ui::ScrollType::Vertical, p.layout);
                auto buttonLayout = ui::VerticalLayout::create(context);
                buttonLayout->setSpacingRole(ui::SizeRole::None);
                for (auto button : p.buttons)
                {
                    button.second->setParent(buttonLayout);
                }
                scrollWidget->setWidget(buttonLayout);
                p.stackLayout = ui::StackLayout::create(context, p.layout);
                p.stackLayout->setHStretch(ui::Stretch::Expanding);
                for (auto widget : p.widgets)
                {
                    scrollWidget = ui::ScrollWidget::create(
                        context, ui::ScrollType::Both, p.stackLayout);
                    scrollWidget->setWidget(widget.second);
                }

                // p.stackLayout->setCurrentIndex(2);
            }

            MainWindow::MainWindow() :
                _p(new Private)
            {
            }

            MainWindow::~MainWindow() {}

            std::shared_ptr<MainWindow>
            MainWindow::create(const std::shared_ptr<system::Context>& context)
            {
                auto out = std::shared_ptr<MainWindow>(new MainWindow);
                out->_init(context);
                return out;
            }

            void MainWindow::setGeometry(const math::Box2i& value)
            {
                Window::setGeometry(value);
                _p->layout->setGeometry(value);
            }
        } // namespace widgets
    } // namespace examples
} // namespace tl
