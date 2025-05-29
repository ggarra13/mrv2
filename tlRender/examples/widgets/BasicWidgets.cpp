// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "BasicWidgets.h"

#include <tlUI/ButtonGroup.h>
#include <tlUI/CheckBox.h>
#include <tlUI/ComboBox.h>
#include <tlUI/FileEdit.h>
#include <tlUI/GroupBox.h>
#include <tlUI/LineEdit.h>
#include <tlUI/PushButton.h>
#include <tlUI/TimeEdit.h>
#include <tlUI/TimeLabel.h>
#include <tlUI/ToolButton.h>
#include <tlUI/RowLayout.h>

#include <tlTimeline/TimeUnits.h>

namespace tl
{
    namespace examples
    {
        namespace widgets
        {
            struct BasicWidgets::Private
            {
                std::shared_ptr<ui::ButtonGroup> buttonGroup;
                std::shared_ptr<ui::RowLayout> layout;
            };

            void BasicWidgets::_init(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IExampleWidget::_init(
                    "Basic Widgets", "tl::examples::widgets::BasicWidgets",
                    context, parent);
                TLRENDER_P();

                auto pushButton0 = ui::PushButton::create(context);
                pushButton0->setText("Click");
                pushButton0->setClickedCallback(
                    []() { std::cout << "Click" << std::endl; });
                auto pushButton1 = ui::PushButton::create(context);
                pushButton1->setCheckable(true);
                pushButton1->setChecked(true);
                pushButton1->setText("Toggle");
                pushButton1->setIcon("Settings");
                pushButton1->setCheckedCallback(
                    [](bool value)
                    { std::cout << "Toggle: " << value << std::endl; });
                auto pushButton2 = ui::PushButton::create(context);
                pushButton2->setText("Click");
                pushButton2->setEnabled(false);

                p.buttonGroup = ui::ButtonGroup::create(
                    ui::ButtonGroupType::Radio, context);
                auto toolButton0 = ui::ToolButton::create(context);
                toolButton0->setChecked(true);
                toolButton0->setIcon("PlaybackStop");
                p.buttonGroup->addButton(toolButton0);
                auto toolButton1 = ui::ToolButton::create(context);
                toolButton1->setText("Forward");
                toolButton1->setIcon("PlaybackForward");
                p.buttonGroup->addButton(toolButton1);
                auto toolButton2 = ui::ToolButton::create(context);
                toolButton2->setIcon("PlaybackReverse");
                p.buttonGroup->addButton(toolButton2);
                p.buttonGroup->setCheckedCallback(
                    [](int index, bool value) {
                        std::cout << "Radio: " << index << " " << value
                                  << std::endl;
                    });
                auto toolButton3 = ui::ToolButton::create(context);
                toolButton3->setIcon("Audio");
                toolButton3->setEnabled(false);

                auto checkBox0 = ui::CheckBox::create(context);
                checkBox0->setText("CheckBox 1");
                checkBox0->setCheckedCallback(
                    [](bool value)
                    { std::cout << "CheckBox 1: " << value << std::endl; });
                auto checkBox1 = ui::CheckBox::create(context);
                checkBox1->setChecked(true);
                checkBox1->setText("CheckBox 2");
                checkBox1->setCheckedCallback(
                    [](bool value)
                    { std::cout << "CheckBox 2: " << value << std::endl; });
                auto checkBox2 = ui::CheckBox::create(context);
                checkBox2->setText("CheckBox 3");
                checkBox2->setEnabled(false);

                auto lineEdit0 = ui::LineEdit::create(context);
                auto lineEdit1 = ui::LineEdit::create(context);
                lineEdit1->setText("Hello world");
                auto lineEdit2 = ui::LineEdit::create(context);
                lineEdit2->setText("Hello world");
                lineEdit2->setEnabled(false);

                auto comboBox0 = ui::ComboBox::create(context);
                comboBox0->setItems(
                    std::vector<std::string>({"Stop", "Forward", "Reverse"}));
                comboBox0->setIndexCallback(
                    [](int value)
                    { std::cout << "Index: " << value << std::endl; });
                auto comboBox1 = ui::ComboBox::create(context);
                comboBox1->setItems(std::vector<ui::ComboBoxItem>(
                    {{"Stop", "PlaybackStop"},
                     {"Forward", "PlaybackForward"},
                     {"Reverse", "PlaybackReverse"}}));
                comboBox1->setIndexCallback(
                    [](int value)
                    { std::cout << "Index: " << value << std::endl; });
                auto comboBox2 = ui::ComboBox::create(context);
                comboBox2->setItems(std::vector<ui::ComboBoxItem>(
                    {{std::string(), "PlaybackStop"},
                     {std::string(), "PlaybackForward"},
                     {std::string(), "PlaybackReverse"}}));
                comboBox2->setIndexCallback(
                    [](int value)
                    { std::cout << "Index: " << value << std::endl; });
                auto comboBox3 = ui::ComboBox::create(context);
                comboBox3->setItems(
                    std::vector<std::string>({"Stop", "Forward", "Reverse"}));
                comboBox3->setEnabled(false);

                auto timeUnitsModel = timeline::TimeUnitsModel::create(context);
                auto timeEdit0 = ui::TimeEdit::create(timeUnitsModel, context);
                timeEdit0->setValue(otime::RationalTime(0.0, 24.0));
                auto timeEdit1 = ui::TimeEdit::create(timeUnitsModel, context);
                timeEdit1->setValue(otime::RationalTime(240.0, 24.0));
                auto timeEdit2 = ui::TimeEdit::create(timeUnitsModel, context);
                timeEdit2->setValue(otime::RationalTime(240.0, 24.0));
                timeEdit2->setEnabled(false);
                auto timeLabel0 =
                    ui::TimeLabel::create(timeUnitsModel, context);
                timeLabel0->setValue(otime::RationalTime(240.0, 24.0));
                auto timeUnitsComboBox = ui::ComboBox::create(context);
                timeUnitsComboBox->setItems(timeline::getTimeUnitsLabels());
                timeUnitsComboBox->setCurrentIndex(
                    static_cast<int>(timeUnitsModel->getTimeUnits()));
                timeUnitsComboBox->setIndexCallback(
                    [timeUnitsModel](int value) {
                        timeUnitsModel->setTimeUnits(
                            static_cast<timeline::TimeUnits>(value));
                    });

                auto fileEdit = ui::FileEdit::create(context);
                fileEdit->setCallback(
                    [](const file::Path& value)
                    { std::cout << value.get() << std::endl; });

                p.layout =
                    ui::VerticalLayout::create(context, shared_from_this());
                p.layout->setMarginRole(ui::SizeRole::Margin);
                auto groupBox =
                    ui::GroupBox::create("Push Buttons", context, p.layout);
                auto hLayout = ui::HorizontalLayout::create(context, groupBox);
                pushButton0->setParent(hLayout);
                pushButton1->setParent(hLayout);
                pushButton2->setParent(hLayout);
                groupBox =
                    ui::GroupBox::create("Tool Buttons", context, p.layout);
                hLayout = ui::HorizontalLayout::create(context, groupBox);
                hLayout->setSpacingRole(ui::SizeRole::SpacingTool);
                toolButton2->setParent(hLayout);
                toolButton0->setParent(hLayout);
                toolButton1->setParent(hLayout);
                toolButton3->setParent(hLayout);
                groupBox =
                    ui::GroupBox::create("Check Boxes", context, p.layout);
                hLayout = ui::HorizontalLayout::create(context, groupBox);
                checkBox0->setParent(hLayout);
                checkBox1->setParent(hLayout);
                checkBox2->setParent(hLayout);
                groupBox =
                    ui::GroupBox::create("Line Edits", context, p.layout);
                hLayout = ui::HorizontalLayout::create(context, groupBox);
                lineEdit0->setParent(hLayout);
                lineEdit1->setParent(hLayout);
                lineEdit2->setParent(hLayout);
                groupBox =
                    ui::GroupBox::create("Combo Boxes", context, p.layout);
                hLayout = ui::HorizontalLayout::create(context, groupBox);
                comboBox0->setParent(hLayout);
                comboBox1->setParent(hLayout);
                comboBox2->setParent(hLayout);
                comboBox3->setParent(hLayout);
                groupBox =
                    ui::GroupBox::create("Time Widgets", context, p.layout);
                hLayout = ui::HorizontalLayout::create(context, groupBox);
                timeEdit0->setParent(hLayout);
                timeEdit1->setParent(hLayout);
                timeEdit2->setParent(hLayout);
                timeLabel0->setParent(hLayout);
                timeUnitsComboBox->setParent(hLayout);
                groupBox = ui::GroupBox::create("File Edit", context, p.layout);
                fileEdit->setParent(groupBox);
            }

            BasicWidgets::BasicWidgets() :
                _p(new Private)
            {
            }

            BasicWidgets::~BasicWidgets() {}

            std::shared_ptr<BasicWidgets> BasicWidgets::create(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<BasicWidgets>(new BasicWidgets);
                out->_init(context, parent);
                return out;
            }

            void BasicWidgets::setGeometry(const math::Box2i& value)
            {
                IExampleWidget::setGeometry(value);
                _p->layout->setGeometry(value);
            }

            void BasicWidgets::sizeHintEvent(const ui::SizeHintEvent& event)
            {
                IExampleWidget::sizeHintEvent(event);
                _sizeHint = _p->layout->getSizeHint();
            }
        } // namespace widgets
    } // namespace examples
} // namespace tl
