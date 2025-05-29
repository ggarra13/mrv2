// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IPopup.h>

namespace tl
{
    namespace ui
    {
        class IWindow;

        //! Popup style.
        enum class MenuPopupStyle { Menu, SubMenu };

        //! Base class for popup menus.
        class IMenuPopup : public IPopup
        {
            TLRENDER_NON_COPYABLE(IMenuPopup);

        protected:
            void _init(
                const std::string& objectName,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            IMenuPopup();

        public:
            virtual ~IMenuPopup() = 0;

            //! Open the popup.
            void open(
                const std::shared_ptr<IWindow>&,
                const math::Box2i& buttonGeometry);

            //! Get whether the popup is open.
            bool isOpen() const;

            //! Close the popup.
            void close() override;

            //! Set the close callback.
            void setCloseCallback(const std::function<void(void)>&);

            //! Set the popup style.
            void setPopupStyle(MenuPopupStyle);

            //! Set the popup color role.
            void setPopupRole(ColorRole);

            //! Set the widget.
            void setWidget(const std::shared_ptr<IWidget>&);

            void setGeometry(const math::Box2i&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void drawEvent(const math::Box2i&, const DrawEvent&) override;

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace ui
} // namespace tl
