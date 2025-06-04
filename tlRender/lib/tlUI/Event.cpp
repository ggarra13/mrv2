// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlUI/Event.h>

#include <tlCore/Error.h>
#include <tlCore/String.h>

#include <sstream>

namespace tl
{
    namespace ui
    {
        ChildEvent::ChildEvent() {}

        ChildEvent::ChildEvent(const std::shared_ptr<IWidget>& child) :
            child(child)
        {
        }

        TickEvent::TickEvent() {}

        TickEvent::TickEvent(
            const std::shared_ptr<Style>& style,
            const std::shared_ptr<IconLibrary>& iconLibrary,
            const std::shared_ptr<image::FontSystem>& fontSystem) :
            style(style),
            iconLibrary(iconLibrary),
            fontSystem(fontSystem)
        {
        }

        SizeHintEvent::SizeHintEvent() {}

        SizeHintEvent::SizeHintEvent(
            const std::shared_ptr<Style>& style,
            const std::shared_ptr<IconLibrary>& iconLibrary,
            const std::shared_ptr<image::FontSystem>& fontSystem,
            float displayScale) :
            style(style),
            iconLibrary(iconLibrary),
            fontSystem(fontSystem),
            displayScale(displayScale)
        {
        }

        DrawEvent::DrawEvent() {}

        DrawEvent::DrawEvent(
            const std::shared_ptr<Style>& style,
            const std::shared_ptr<IconLibrary>& iconLibrary,
            const std::shared_ptr<timeline::IRender>& render,
            const std::shared_ptr<image::FontSystem>& fontSystem) :
            style(style),
            iconLibrary(iconLibrary),
            render(render),
            fontSystem(fontSystem)
        {
        }

        DragAndDropData::~DragAndDropData() {}

        MouseMoveEvent::MouseMoveEvent() {}

        MouseMoveEvent::MouseMoveEvent(
            const math::Vector2i& pos, const math::Vector2i& prev) :
            pos(pos),
            prev(prev)
        {
        }

        MouseClickEvent::MouseClickEvent() {}

        MouseClickEvent::MouseClickEvent(
            int button, int modifiers, const math::Vector2i& pos) :
            button(button),
            modifiers(modifiers),
            pos(pos)
        {
        }

        ScrollEvent::ScrollEvent() {}

        ScrollEvent::ScrollEvent(
            const math::Vector2f& value, int modifiers,
            const math::Vector2i& pos) :
            value(value),
            modifiers(modifiers),
            pos(pos)
        {
        }

        std::string getKeyModifierLabel(int value)
        {
            std::vector<std::string> out;
            if (value & static_cast<size_t>(KeyModifier::Shift))
            {
                out.push_back("Shift");
            }
            if (value & static_cast<size_t>(KeyModifier::Control))
            {
                out.push_back("Ctrl");
            }
            if (value & static_cast<size_t>(KeyModifier::Alt))
            {
                out.push_back("Alt");
            }
            if (value & static_cast<size_t>(KeyModifier::Super))
            {
                out.push_back("Cmd");
            }
            return string::join(out, '+');
        }

        TLRENDER_ENUM_IMPL(
            Key, "Unknown", "Space", "Apostrophe", "Comma", "Minus", "Period",
            "Slash", "_0", "_1", "_2", "_3", "_4", "_5", "_6", "_7", "_8", "_9",
            "Semicolon", "Equal", "A", "B", "C", "D", "E", "F", "G", "H", "I",
            "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V",
            "W", "X", "Y", "Z", "LeftBracket", "Backslash", "RightBracket",
            "GraveAccent", "Escape", "Enter", "Tab", "Backspace", "Insert",
            "Delete", "Right", "Left", "Down", "Up", "PageUp", "PageDown",
            "Home", "End", "CapsLock", "ScrollLock", "NumLock", "PrintScreen",
            "Pause", "F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9",
            "F10", "F11", "F12", "LeftShift", "LeftControl", "LeftAlt",
            "LeftSuper", "RightShift", "RightControl", "RightAlt",
            "RightSuper");
        TLRENDER_ENUM_SERIALIZE_IMPL(Key);

        std::string getLabel(Key key, int modifiers)
        {
            std::stringstream ss;
            if (key != Key::Unknown)
            {
                if (modifiers)
                {
                    ss << getKeyModifierLabel(modifiers);
                    ss << "+";
                }
                ss << key;
            }
            return ss.str();
        }

        KeyEvent::KeyEvent() {}

        KeyEvent::KeyEvent(Key key, int modifiers, const math::Vector2i& pos) :
            key(key),
            modifiers(modifiers),
            pos(pos)
        {
        }

        TextEvent::TextEvent() {}

        TextEvent::TextEvent(const std::string& text) :
            text(text)
        {
        }

        TextDragAndDropData::TextDragAndDropData(const std::string& text) :
            _text(text)
        {
        }

        TextDragAndDropData::~TextDragAndDropData() {}

        const std::string& TextDragAndDropData::getText() const
        {
            return _text;
        }

        DragAndDropEvent::DragAndDropEvent() {}

        DragAndDropEvent::DragAndDropEvent(
            const math::Vector2i& pos, const math::Vector2i& prev,
            const std::shared_ptr<DragAndDropData>& data) :
            pos(pos),
            prev(prev),
            data(data)
        {
        }
    } // namespace ui
} // namespace tl
