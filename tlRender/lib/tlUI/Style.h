// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlCore/Color.h>
#include <tlCore/Context.h>
#include <tlCore/FontSystem.h>
#include <tlCore/ValueObserver.h>

namespace tl
{
    namespace ui
    {
        //! Size roles.
        enum class SizeRole {
            kNone,
            Margin,
            MarginSmall,
            MarginLarge,
            MarginInside,
            MarginDialog,
            Spacing,
            SpacingSmall,
            SpacingLarge,
            SpacingTool,
            Border,
            ScrollArea,
            Slider,
            Handle,
            HandleSmall,
            Swatch,
            SwatchLarge,
            Shadow,
            DragLength,

            Count,
            First = kNone
        };
        TLRENDER_ENUM(SizeRole);
        TLRENDER_ENUM_SERIALIZE(SizeRole);

        //! Get the default size roles.
        std::map<SizeRole, int> defaultSizeRoles();

        //! Color roles.
        enum class ColorRole {
            kNone,

            Window,
            Base,
            Button,
            Text,
            TextDisabled,
            Border,
            Hover,
            Pressed,
            Checked,
            KeyFocus,
            Overlay,
            ToolTipWindow,
            ToolTipText,

            InOut,
            FrameMarker,
            VideoCache,
            AudioCache,
            VideoClip,
            VideoGap,
            AudioClip,
            AudioGap,
            Transition,

            Red,
            Green,
            Blue,
            Cyan,
            Magenta,
            Yellow,

            Count,
            First = kNone
        };
        TLRENDER_ENUM(ColorRole);
        TLRENDER_ENUM_SERIALIZE(ColorRole);

        //! Get default color roles.
        std::map<ColorRole, image::Color4f> defaultColorRoles();

        //! Font roles.
        enum class FontRole {
            kNone,
            Label,
            Mono,
            Title,

            Count,
            First = kNone
        };
        TLRENDER_ENUM(FontRole);
        TLRENDER_ENUM_SERIALIZE(FontRole);

        //! Get default font roles.
        std::map<FontRole, image::FontInfo> defaultFontRoles();

        //! Style.
        class Style : public std::enable_shared_from_this<Style>
        {
            TLRENDER_NON_COPYABLE(Style);

        protected:
            void _init(const std::shared_ptr<system::Context>&);

            Style();

        public:
            ~Style();

            //! Create a new style.
            static std::shared_ptr<Style>
            create(const std::shared_ptr<system::Context>&);

            //! Get a size role.
            int getSizeRole(SizeRole, float scale) const;

            //! Set a size role.
            void setSizeRole(SizeRole, int);

            //! Set the size roles.
            void setSizeRoles(const std::map<SizeRole, int>&);

            //! Get a color role.
            image::Color4f getColorRole(ColorRole) const;

            //! Set a color role.
            void setColorRole(ColorRole, const image::Color4f&);

            //! Set the color roles.
            void setColorRoles(const std::map<ColorRole, image::Color4f>&);

            //! Get a font role.
            image::FontInfo getFontRole(FontRole, float scale) const;

            //! Set a font role.
            void setFontRole(FontRole, const image::FontInfo&);

            //! Set the font roles.
            void setFontRoles(const std::map<FontRole, image::FontInfo>&);

            //! Observe style changes.
            std::shared_ptr<observer::IValue<bool> > observeChanged() const;

        private:
            std::map<SizeRole, int> _sizeRoles;
            std::map<ColorRole, image::Color4f> _colorRoles;
            std::map<FontRole, image::FontInfo> _fontRoles;

            TLRENDER_PRIVATE();
        };

        //! \name Serialize
        ///@{

        void
        to_json(nlohmann::json&, const std::map<ColorRole, image::Color4f>&);

        void
        from_json(const nlohmann::json&, std::map<ColorRole, image::Color4f>&);

        ///@}
    } // namespace ui
} // namespace tl

#include <tlUI/StyleInline.h>
