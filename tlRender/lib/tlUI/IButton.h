// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Base class for buttons.
        class IButton : public IWidget
        {
            TLRENDER_NON_COPYABLE(IButton);

        protected:
            void _init(
                const std::string& objectName,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            IButton();

        public:
            virtual ~IButton();

            //! Get whether the button is checkable.
            bool isCheckable() const;

            //! Set whether the button is checkable.
            void setCheckable(bool);

            //! Get whether the button is checked.
            bool isChecked() const;

            //! Set whether the button is checked.
            void setChecked(bool);

            //! Set the text.
            virtual void setText(const std::string&);

            //! Set the font role.
            virtual void setFontRole(FontRole);

            //! Set the icon.
            void setIcon(const std::string&);

            //! Set the checked icon.
            void setCheckedIcon(const std::string&);

            //! Set the button color role.
            void setButtonRole(ColorRole);

            //! Set the checked color role.
            void setCheckedRole(ColorRole);

            //! Set the hovered callback.
            void setHoveredCallback(const std::function<void(bool)>&);

            //! Set the pressed callback.
            void setPressedCallback(const std::function<void(void)>&);

            //! Set whether the button repeats clicks when pressed.
            void setRepeatClick(bool);

            //! Set the clicked callback.
            void setClickedCallback(const std::function<void(void)>&);

            //! Set the checked callback.
            void setCheckedCallback(const std::function<void(bool)>&);

            void tickEvent(bool, bool, const TickEvent&) override;
            void sizeHintEvent(const SizeHintEvent&) override;
            void mouseEnterEvent() override;
            void mouseLeaveEvent() override;
            void mousePressEvent(MouseClickEvent&) override;
            void mouseReleaseEvent(MouseClickEvent&) override;

        protected:
            void _click();

            void _releaseMouse() override;

            std::string _text;
            FontRole _fontRole = FontRole::Label;
            std::string _icon;
            std::shared_ptr<image::Image> _iconImage;
            std::string _checkedIcon;
            std::shared_ptr<image::Image> _checkedIconImage;
            ColorRole _buttonRole = ColorRole::Button;
            ColorRole _checkedRole = ColorRole::Checked;
            bool _checked = false;
            std::function<void(bool)> _hoveredCallback;
            std::function<void(void)> _pressedCallback;
            std::function<void(void)> _clickedCallback;
            std::function<void(bool)> _checkedCallback;

        private:
            TLRENDER_PRIVATE();
        };
    } // namespace ui
} // namespace tl
