// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include "IExampleWidget.h"

namespace tl
{
    namespace examples
    {
        namespace widgets
        {
            //! Basic widgets.
            class BasicWidgets : public IExampleWidget
            {
                TLRENDER_NON_COPYABLE(BasicWidgets);

            protected:
                void _init(
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent);

                BasicWidgets();

            public:
                ~BasicWidgets();

                static std::shared_ptr<BasicWidgets> create(
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent = nullptr);

                void setGeometry(const math::Box2i&) override;
                void sizeHintEvent(const ui::SizeHintEvent&) override;

            private:
                TLRENDER_PRIVATE();
            };
        } // namespace widgets
    } // namespace examples
} // namespace tl
