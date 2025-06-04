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
            //! Numeric widgets.
            class NumericWidgets : public IExampleWidget
            {
                TLRENDER_NON_COPYABLE(NumericWidgets);

            protected:
                void _init(
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent);

                NumericWidgets();

            public:
                ~NumericWidgets();

                static std::shared_ptr<NumericWidgets> create(
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
