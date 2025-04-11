// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "GridLayouts.h"

#include <tlUI/Label.h>
#include <tlUI/GridLayout.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace examples
    {
        namespace widgets
        {
            struct GridLayouts::Private
            {
                std::shared_ptr<ui::GridLayout> layout;
            };

            void GridLayouts::_init(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IExampleWidget::_init(
                    "Grid Layouts", "tl::examples::widgets::GridLayouts",
                    context, parent);
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
                labels[1]->setFontRole(ui::FontRole::Title);
                // labels[2]->setHStretch(ui::Stretch::Expanding);
                labels[3]->setHStretch(ui::Stretch::Expanding);
                labels[4]->setHStretch(ui::Stretch::Expanding);
                labels[4]->setVStretch(ui::Stretch::Expanding);
                labels[5]->setFontRole(ui::FontRole::Title);
                labels[5]->setHStretch(ui::Stretch::Expanding);

                p.layout = ui::GridLayout::create(context, shared_from_this());
                p.layout->setMarginRole(ui::SizeRole::Margin);
                // p.layout->setSpacingRole(ui::SizeRole::None);
                labels[0]->setParent(p.layout);
                labels[1]->setParent(p.layout);
                labels[2]->setParent(p.layout);
                labels[3]->setParent(p.layout);
                labels[4]->setParent(p.layout);
                labels[5]->setParent(p.layout);
                p.layout->setGridPos(labels[0], 0, 0);
                p.layout->setGridPos(labels[1], 0, 1);
                p.layout->setGridPos(labels[2], 0, 2);
                p.layout->setGridPos(labels[3], 1, 0);
                p.layout->setGridPos(labels[4], 1, 1);
                p.layout->setGridPos(labels[5], 3, 4);
            }

            GridLayouts::GridLayouts() :
                _p(new Private)
            {
            }

            GridLayouts::~GridLayouts() {}

            std::shared_ptr<GridLayouts> GridLayouts::create(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<GridLayouts>(new GridLayouts);
                out->_init(context, parent);
                return out;
            }

            void GridLayouts::setGeometry(const math::Box2i& value)
            {
                IExampleWidget::setGeometry(value);
                _p->layout->setGeometry(value);
            }

            void GridLayouts::sizeHintEvent(const ui::SizeHintEvent& event)
            {
                IExampleWidget::sizeHintEvent(event);
                _sizeHint = _p->layout->getSizeHint();
            }
        } // namespace widgets
    } // namespace examples
} // namespace tl
