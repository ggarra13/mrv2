// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlUI/IPopup.h>

namespace tl
{
    namespace ui
    {
        void IPopup::_init(
            const std::string& objectName,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(objectName, context, parent);
        }

        IPopup::IPopup() {}

        IPopup::~IPopup() {}

        void IPopup::keyPressEvent(KeyEvent& event)
        {
            if (0 == event.modifiers)
            {
                if (Key::Escape == event.key)
                {
                    event.accept = true;
                    close();
                }
            }
        }

        void IPopup::keyReleaseEvent(KeyEvent& event)
        {
            event.accept = true;
        }
    } // namespace ui
} // namespace tl
