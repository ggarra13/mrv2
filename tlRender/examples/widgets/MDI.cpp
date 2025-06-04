// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "MDI.h"

#include <tlUI/IntEditSlider.h>
#include <tlUI/MDICanvas.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace examples
    {
        namespace widgets
        {
            struct MDI::Private
            {
                std::shared_ptr<ui::MDICanvas> canvas;
            };

            void MDI::_init(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                IExampleWidget::_init(
                    "MDI", "tl::examples::widgets::MDI", context, parent);
                TLRENDER_P();

                p.canvas = ui::MDICanvas::create(context, shared_from_this());

                for (size_t i = 0; i < 10; ++i)
                {
                    auto slider = ui::IntEditSlider::create(context);
                    p.canvas->addWidget(
                        string::Format("Slider {0}").arg(i), slider);
                }
            }

            MDI::MDI() :
                _p(new Private)
            {
            }

            MDI::~MDI() {}

            std::shared_ptr<MDI> MDI::create(
                const std::shared_ptr<system::Context>& context,
                const std::shared_ptr<IWidget>& parent)
            {
                auto out = std::shared_ptr<MDI>(new MDI);
                out->_init(context, parent);
                return out;
            }

            void MDI::setGeometry(const math::Box2i& value)
            {
                IExampleWidget::setGeometry(value);
                _p->canvas->setGeometry(value);
            }

            void MDI::sizeHintEvent(const ui::SizeHintEvent& event)
            {
                IExampleWidget::sizeHintEvent(event);
                _sizeHint = _p->canvas->getSizeHint();
            }
        } // namespace widgets
    } // namespace examples
} // namespace tl
