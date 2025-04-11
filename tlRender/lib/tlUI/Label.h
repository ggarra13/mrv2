// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        //! Text label.
        //!
        //! \todo Add text wrapping.
        class Label : public IWidget
        {
            TLRENDER_NON_COPYABLE(Label);

        protected:
            void _init(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent);

            Label();

        public:
            virtual ~Label();

            //! Create a new widget.
            static std::shared_ptr<Label> create(
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Create a new widget.
            static std::shared_ptr<Label> create(
                const std::string& text,
                const std::shared_ptr<system::Context>&,
                const std::shared_ptr<IWidget>& parent = nullptr);

            //! Set the text.
            void setText(const std::string&);

            //! Set the text color role.
            void setTextRole(ColorRole);

            //! Set the margin role.
            void setMarginRole(SizeRole);

            //! Set the font role.
            void setFontRole(FontRole);

            void sizeHintEvent(const SizeHintEvent&) override;
            void clipEvent(const math::Box2i&, bool) override;
            void drawEvent(const math::Box2i&, const DrawEvent&) override;

        private:
            void _textUpdate();
            TLRENDER_PRIVATE();
        };
    } // namespace ui
} // namespace tl
