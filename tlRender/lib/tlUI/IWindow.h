// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUI/IWidget.h>

namespace tl
{
    namespace ui
    {
        class IClipboard;

        //! Base class for windows.
        class IWindow : public IWidget
        {
            TLRENDER_NON_COPYABLE(IWindow);

        protected:
            IWindow();

        public:
            virtual ~IWindow() = 0;

            //! Set the widget with key focus.
            void setKeyFocus(const std::shared_ptr<IWidget>&);

            //! Get the clipboard.
            const std::shared_ptr<IClipboard>& getClipboard() const;

            //! Set the clipboard.
            void setClipboard(const std::shared_ptr<IClipboard>&);

            void setVisible(bool) override;
            void tickEvent(
                bool parentsVisible, bool parentsEnabled,
                const TickEvent&) override;
            void
            drawOverlayEvent(const math::Box2i&, const DrawEvent&) override;

        protected:
            bool _key(Key, bool press, int modifiers);
            void _text(const std::string&);
            void _cursorEnter(bool enter);
            void _cursorPos(const math::Vector2i&);
            void _mouseButton(int button, bool press, int modifiers);
            void _scroll(const math::Vector2f&, int modifiers);

            void _clipEventRecursive(
                const std::shared_ptr<IWidget>&, const math::Box2i&,
                bool clipped);

            virtual void _drop(const std::vector<std::string>&);

        private:
            std::list<std::shared_ptr<IWidget> >
            _getUnderCursor(const math::Vector2i&);
            void _getUnderCursor(
                const std::shared_ptr<IWidget>&, const math::Vector2i&,
                std::list<std::shared_ptr<IWidget> >&);

            void _setHover(const std::shared_ptr<IWidget>&);
            void _hoverUpdate(MouseMoveEvent&);

            std::shared_ptr<IWidget>
            _keyFocusNext(const std::shared_ptr<IWidget>&);
            std::shared_ptr<IWidget>
            _keyFocusPrev(const std::shared_ptr<IWidget>&);
            void _getKeyFocus(
                const std::shared_ptr<IWidget>&,
                std::list<std::shared_ptr<IWidget> >&);

            TLRENDER_PRIVATE();
        };
    } // namespace ui
} // namespace tl
