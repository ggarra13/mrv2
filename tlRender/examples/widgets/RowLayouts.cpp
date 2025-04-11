// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "RowLayouts.h"

#include <tlUI/Label.h>
#include <tlUI/RowLayout.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace examples
    {
        namespace widgets
        {
            struct RowLayouts::Private
            {
                std::shared_ptr<ui::RowLayout> layout;
            };

            void RowLayouts::_init(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IExampleWidget::_init(
                    "Row Layouts", "tl::examples::widgets::RowLayouts", context,
                    parent);
                TLRENDER_P();

                std::vector<std::shared_ptr<ui::Label> > labels;
                for (size_t i = 0; i < 6; ++i)
                {
                    auto label = ui::Label::create(context);
                    label->setText(string::Format("Label {0}").arg(i));
                    label->setMarginRole(ui::SizeRole::Margin);
                    label->setBackgroundRole(ui::ColorRole::Base);
                    labels.push_back(label);
                }
                labels[2]->setHStretch(ui::Stretch::Expanding);
                labels[3]->setFontRole(ui::FontRole::Title);
                labels[5]->setHStretch(ui::Stretch::Expanding);

                p.layout =
                    ui::VerticalLayout::create(context, shared_from_this());
                p.layout->setMarginRole(ui::SizeRole::Margin);
                auto hLayout = ui::HorizontalLayout::create(context, p.layout);
                labels[0]->setParent(hLayout);
                labels[1]->setParent(hLayout);
                hLayout = ui::HorizontalLayout::create(context, p.layout);
                labels[2]->setParent(hLayout);
                labels[3]->setParent(hLayout);
                hLayout = ui::HorizontalLayout::create(context, p.layout);
                hLayout->setVStretch(ui::Stretch::Expanding);
                labels[4]->setParent(hLayout);
                labels[5]->setParent(hLayout);
            }

            RowLayouts::RowLayouts() :
                _p(new Private)
            {
            }

            RowLayouts::~RowLayouts() {}

            std::shared_ptr<RowLayouts> RowLayouts::create(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<RowLayouts>(new RowLayouts);
                out->_init(context, parent);
                return out;
            }

            void RowLayouts::setGeometry(const math::Box2i& value)
            {
                IExampleWidget::setGeometry(value);
                _p->layout->setGeometry(value);
            }

            void RowLayouts::sizeHintEvent(const ui::SizeHintEvent& event)
            {
                IExampleWidget::sizeHintEvent(event);
                _sizeHint = _p->layout->getSizeHint();
            }
        } // namespace widgets
    } // namespace examples
} // namespace tl
