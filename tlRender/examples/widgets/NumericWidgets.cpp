// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "NumericWidgets.h"

#include <tlUI/GridLayout.h>
#include <tlUI/GroupBox.h>
#include <tlUI/DoubleEditSlider.h>
#include <tlUI/IntEditSlider.h>
#include <tlUI/FloatEditSlider.h>
#include <tlUI/Label.h>
#include <tlUI/RowLayout.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace examples
    {
        namespace widgets
        {
            struct NumericWidgets::Private
            {
                std::shared_ptr<ui::RowLayout> layout;
            };

            void NumericWidgets::_init(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IExampleWidget::_init(
                    "Numeric Widgets", "tl::examples::widgets::NumericWidgets",
                    context, parent);
                TLRENDER_P();

                const std::vector<math::IntRange> intRanges = {
                    math::IntRange(0, 10),     math::IntRange(0, 100),
                    math::IntRange(0, 1000),   math::IntRange(100, 200),
                    math::IntRange(-100, 200), math::IntRange(-100, -200)};
                std::vector<std::shared_ptr<ui::Label> > intLabels;
                std::vector<std::shared_ptr<ui::IntEditSlider> > intEdits;
                for (const auto& i : intRanges)
                {
                    auto label = ui::Label::create(context);
                    label->setText(string::Format("{0} - {1}:")
                                       .arg(i.getMin())
                                       .arg(i.getMax()));
                    intLabels.push_back(label);
                    auto edit = ui::IntEditSlider::create(context);
                    edit->setRange(i);
                    intEdits.push_back(edit);
                }

                const std::vector<math::FloatRange> floatRanges = {
                    math::FloatRange(0.F, 1.F),
                    math::FloatRange(0.F, 10.F),
                    math::FloatRange(0.F, 100.F),
                    math::FloatRange(0.F, 1000.F),
                    math::FloatRange(100.F, 200.F),
                    math::FloatRange(-100.F, 200.F),
                    math::FloatRange(-100.F, -200.F)};
                std::vector<std::shared_ptr<ui::Label> > floatLabels;
                std::vector<std::shared_ptr<ui::FloatEditSlider> > floatEdits;
                for (const auto& i : floatRanges)
                {
                    auto label = ui::Label::create(context);
                    label->setText(string::Format("{0} - {1}:")
                                       .arg(i.getMin())
                                       .arg(i.getMax()));
                    floatLabels.push_back(label);
                    auto edit = ui::FloatEditSlider::create(context);
                    edit->setRange(i);
                    floatEdits.push_back(edit);
                }

                const std::vector<math::DoubleRange> doubleRanges = {
                    math::DoubleRange(0.0, 1.0),
                    math::DoubleRange(0.0, 10.0),
                    math::DoubleRange(0.0, 100.0),
                    math::DoubleRange(0.0, 1000.0),
                    math::DoubleRange(100.0, 200.0),
                    math::DoubleRange(-100.0, 200.0),
                    math::DoubleRange(-100.0, -200.0)};
                std::vector<std::shared_ptr<ui::Label> > doubleLabels;
                std::vector<std::shared_ptr<ui::DoubleEditSlider> > doubleEdits;
                for (const auto& i : doubleRanges)
                {
                    auto label = ui::Label::create(context);
                    label->setText(string::Format("{0} - {1}:")
                                       .arg(i.getMin())
                                       .arg(i.getMax()));
                    doubleLabels.push_back(label);
                    auto edit = ui::DoubleEditSlider::create(context);
                    edit->setRange(i);
                    doubleEdits.push_back(edit);
                }

                p.layout =
                    ui::VerticalLayout::create(context, shared_from_this());
                p.layout->setMarginRole(ui::SizeRole::Margin);
                auto groupBox = ui::GroupBox::create(context, p.layout);
                groupBox->setText("Integer Values");
                auto gridLayout = ui::GridLayout::create(context, groupBox);
                gridLayout->setSpacingRole(ui::SizeRole::SpacingSmall);
                for (int i = 0; i < intRanges.size(); ++i)
                {
                    intLabels[i]->setParent(gridLayout);
                    gridLayout->setGridPos(intLabels[i], i, 0);
                    intEdits[i]->setParent(gridLayout);
                    gridLayout->setGridPos(intEdits[i], i, 1);
                }
                groupBox = ui::GroupBox::create(context, p.layout);
                groupBox->setText("Floating Point Values");
                gridLayout = ui::GridLayout::create(context, groupBox);
                gridLayout->setSpacingRole(ui::SizeRole::SpacingSmall);
                for (int i = 0; i < floatRanges.size(); ++i)
                {
                    floatLabels[i]->setParent(gridLayout);
                    gridLayout->setGridPos(floatLabels[i], i, 0);
                    floatEdits[i]->setParent(gridLayout);
                    gridLayout->setGridPos(floatEdits[i], i, 1);
                }
                groupBox = ui::GroupBox::create(context, p.layout);
                groupBox->setText("Double Precision Floating Point Values");
                gridLayout = ui::GridLayout::create(context, groupBox);
                gridLayout->setSpacingRole(ui::SizeRole::SpacingSmall);
                for (int i = 0; i < doubleRanges.size(); ++i)
                {
                    doubleLabels[i]->setParent(gridLayout);
                    gridLayout->setGridPos(doubleLabels[i], i, 0);
                    doubleEdits[i]->setParent(gridLayout);
                    gridLayout->setGridPos(doubleEdits[i], i, 1);
                }
            }

            NumericWidgets::NumericWidgets() :
                _p(new Private)
            {
            }

            NumericWidgets::~NumericWidgets() {}

            std::shared_ptr<NumericWidgets> NumericWidgets::create(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<NumericWidgets>(new NumericWidgets);
                out->_init(context, parent);
                return out;
            }

            void NumericWidgets::setGeometry(const math::Box2i& value)
            {
                IExampleWidget::setGeometry(value);
                _p->layout->setGeometry(value);
            }

            void NumericWidgets::sizeHintEvent(const ui::SizeHintEvent& event)
            {
                IExampleWidget::sizeHintEvent(event);
                _sizeHint = _p->layout->getSizeHint();
            }
        } // namespace widgets
    } // namespace examples
} // namespace tl
