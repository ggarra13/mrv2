// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the toucan project.

#include "MarkerItem.h"

#include <tlUI/DrawUtil.h>

namespace tl
{
    namespace TIMELINEUI
    {
        image::Color4f getMarkerColor(const std::string& color)
        {
            image::Color4f out(1.F, 0.F, 0.F);
            if (color == otio::Marker::Color::pink)
            {
                out = image::Color4f(1.F, 0.F, .5F);
            }
            else if (color == otio::Marker::Color::red)
            {
                out = image::Color4f(1.F, 0.F, 0.F);
            }
            else if (color == otio::Marker::Color::orange)
            {
                out = image::Color4f(1.F, .6F, 0.F);
            }
            else if (color == otio::Marker::Color::yellow)
            {
                out = image::Color4f(1.F, 1.F, 0.F);
            }
            else if (color == otio::Marker::Color::green)
            {
                out = image::Color4f(0.F, 1.F, 0.F);
            }
            else if (color == otio::Marker::Color::cyan)
            {
                out = image::Color4f(0.F, 1.F, 1.F);
            }
            else if (color == otio::Marker::Color::blue)
            {
                out = image::Color4f(0.F, 0.F, 1.F);
            }
            else if (color == otio::Marker::Color::purple)
            {
                out = image::Color4f(.5F, 0.F, 1.F);
            }
            else if (color == otio::Marker::Color::magenta)
            {
                out = image::Color4f(1.F, 0.F, 1.F);
            }
            else if (color == otio::Marker::Color::black)
            {
                out = image::Color4f(0.F, 0.F, 0.F);
            }
            else if (color == otio::Marker::Color::white)
            {
                out = image::Color4f(1.F, 1.F, 1.F);
            }
            return out;
        }

        void MarkerItem::_init(
            const std::shared_ptr<system::Context>& context,
            double scale, const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const otio::Marker* marker,
            const otio::TimeRange& timeRange,
            const std::shared_ptr<IWidget>& parent)
        {
            IItem::_init(
                "tl::TIMELINEUI::MarkerItem",
                timeRange,
                timeRange,
                scale,
                options,
                displayOptions,
                itemData,
                context,
                parent);

            _marker = marker;
            _text = !marker->name().empty() ? marker->name() : "Marker";
            _color = getMarkerColor(marker->color());

            _label = ui::Label::create(context, shared_from_this());
            _label->setText(_text);

            _textUpdate();
        }
    
        MarkerItem::~MarkerItem()
        {}

        std::shared_ptr<MarkerItem> MarkerItem::create(
            const std::shared_ptr<system::Context>& context,
            double scale, const ItemOptions& options,
            const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& itemData,
            const otio::Marker* marker,
            const otio::TimeRange& timeRange,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::make_shared<MarkerItem>();
            out->_init(context, scale, options, displayOptions, itemData,
                       marker, timeRange, parent);
            return out;
        }

        void MarkerItem::setGeometry(const math::Box2i& value)
        {
            IItem::setGeometry(value);
            const math::Box2i g(
                value.min.x + value.h(),
                value.min.y,
                value.w() - value.h(),
                value.h());
            _label->setGeometry(g);
        }

        void MarkerItem::sizeHintEvent(const ui::SizeHintEvent& event)
        {
            IItem::sizeHintEvent(event);
            const bool displayScaleChanged = event.displayScale != _size.displayScale;
            if (_size.init || displayScaleChanged)
            {
                _size.init = false;
                _size.displayScale = event.displayScale;
                _size.margin = event.style->getSizeRole(ui::SizeRole::MarginInside, event.displayScale);
                _size.border = event.style->getSizeRole(ui::SizeRole::Border, event.displayScale);
            }
            math::Size2i sizeHint = _label->getSizeHint();
            sizeHint.h += _size.border * 2;
            _sizeHint = sizeHint;
        }

        void MarkerItem::drawEvent(
            const math::Box2i& drawRect,
            const ui::DrawEvent& event)
        {
            IItem::drawEvent(drawRect, event);
            const math::Box2i& g = getGeometry();
            const math::Box2i g2 = math::margin(g, -_size.border, 0,
                                                -_size.border, 0);
            event.render->drawRect(
                g2,
                _selected ? event.style->getColorRole(ui::ColorRole::Yellow) :
                image::Color4f(.3F, .3F, .3F));

            const math::Box2i g3(g.min.x, g.min.y, g.h(), g.h());
            event.render->drawMesh(ui::circle(math::center(g3), g3.h() / 4),
                                   math::Vector2i(), _color);
        }

        void MarkerItem::_timeUnitsUpdate()
        {
            _textUpdate();
        }

        void MarkerItem::_textUpdate()
        {
            if (_label)
            {
                std::string text = _getDurationLabel(_timeRange.duration());
                _label->setText(text);
            }
        }
    }
}
