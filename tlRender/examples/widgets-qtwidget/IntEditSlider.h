// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlQtWidget/IntEditSlider.h>

namespace tl
{
    namespace examples
    {
        namespace widgets_qtwidget
        {
            class IntEditSlider : public QWidget
            {
                Q_OBJECT

            public:
                IntEditSlider(QWidget* parent = nullptr);

            private:
                std::vector<qtwidget::IntEditSlider*> _sliders;
            };
        } // namespace widgets_qtwidget
    } // namespace examples
} // namespace tl
