// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace examples
    {
        namespace widgets
        {
            //! Base class for example widgets.
            class IExampleWidget : public ui::IWidget
            {
                TLRENDER_NON_COPYABLE(IExampleWidget);

            protected:
                void _init(
                    const std::string& exampleName,
                    const std::string& objectName,
                    const std::shared_ptr<system::Context>&,
                    const std::shared_ptr<IWidget>& parent);

                IExampleWidget();

            public:
                virtual ~IExampleWidget() = 0;

                const std::string& getExampleName() const;

            private:
                std::string _exampleName;
            };
        } // namespace widgets
    } // namespace examples
} // namespace tl
