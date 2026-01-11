// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include "IItem.h"

#include <tlCore/Error.h>
#include <tlCore/String.h>
#include <tlCore/StringFormat.h>

#include <opentimelineio/marker.h>

namespace tl
{
    namespace TIMELINEUI
    {
        bool ItemOptions::operator==(const ItemOptions& other) const
        {
            return inputEnabled == other.inputEnabled &&
                   editAssociatedClips == other.editAssociatedClips;
        }

        bool ItemOptions::operator!=(const ItemOptions& other) const
        {
            return !(*this == other);
        }

        bool DisplayOptions::operator==(const DisplayOptions& other) const
        {
            return inOutDisplay == other.inOutDisplay &&
                   cacheDisplay == other.cacheDisplay &&
                   tracks == other.tracks && trackInfo == other.trackInfo &&
                   clipInfo == other.clipInfo &&
                   thumbnails == other.thumbnails &&
                   thumbnailHeight == other.thumbnailHeight &&
                   waveformWidth == other.waveformWidth &&
                   waveformHeight == other.waveformHeight &&
                   waveformPrim == other.waveformPrim &&
                   thumbnailFade == other.thumbnailFade &&
                   transitions == other.transitions &&
                   markers == other.markers &&
                   regularFont == other.regularFont &&
                   monoFont == other.monoFont && fontSize == other.fontSize &&
                   clipRectScale == other.clipRectScale && ocio == other.ocio &&
                   lut == other.lut && hdr == other.hdr;
        }

        bool DisplayOptions::operator!=(const DisplayOptions& other) const
        {
            return !(*this == other);
        }

        std::vector<Marker> getMarkers(const otio::Item* item)
        {
            std::vector<Marker> out;
            for (const auto& marker : item->markers())
            {
                out.push_back(
                    {marker->name(), getMarkerColor(marker->color()),
                     marker->marked_range()});
            }
            return out;
        }

        image::Color4f getMarkerColor(const std::string& value)
        {
            const std::map<std::string, image::Color4f> colors = {
                //! \bug The OTIO marker variables are causing undefined
                //! symbol errors on Linux and macOS.
                /*{otio::Marker::Color::pink, image::Color4f(1.F, .752F,
                .796F)}, { otio::Marker::Color::red, image::Color4f(1.F, 0.F,
                0.F) }, { otio::Marker::Color::orange, image::Color4f(1.F, .75F,
                0.F) }, { otio::Marker::Color::yellow, image::Color4f(1.F, 1.F,
                0.F) }, { otio::Marker::Color::green, image::Color4f(0.F, 1.F,
                0.F) }, { otio::Marker::Color::cyan,
                image::Color4f(0.F, 1.F, 1.F) }, { otio::Marker::Color::blue,
                image::Color4f(0.F, 0.F, 1.F) }, { otio::Marker::Color::purple,
                image::Color4f(0.5F, 0.F, .5F) }, {
                otio::Marker::Color::magenta, image::Color4f(1.F, 0.F, 1.F) },
                { otio::Marker::Color::black, image::Color4f(0.F, 0.F, 0.F) },
                { otio::Marker::Color::white, image::Color4f(1.F, 1.F, 1.F) }*/
                {"PINK", image::Color4f(1.F, .752F, .796F)},
                {"RED", image::Color4f(1.F, 0.F, 0.F)},
                {"ORANGE", image::Color4f(1.F, .75F, 0.F)},
                {"YELLOW", image::Color4f(1.F, 1.F, 0.F)},
                {"GREEN", image::Color4f(0.F, 1.F, 0.F)},
                {"CYAN", image::Color4f(0.F, 1.F, 1.F)},
                {"BLUE", image::Color4f(0.F, 0.F, 1.F)},
                {"PURPLE", image::Color4f(0.5F, 0.F, .5F)},
                {"MAGENTA", image::Color4f(1.F, 0.F, 1.F)},
                {"BLACK", image::Color4f(0.F, 0.F, 0.F)},
                {"WHITE", image::Color4f(1.F, 1.F, 1.F)}};
            const auto i = colors.find(value);
            return i != colors.end() ? i->second : image::Color4f();
        }

        DragAndDropData::DragAndDropData(const std::shared_ptr<IItem>& item) :
            _item(item)
        {
        }

        DragAndDropData::~DragAndDropData() {}

        const std::shared_ptr<IItem>& DragAndDropData::getItem() const
        {
            return _item;
        }

        struct IItem::Private
        {
            ui::ColorRole selectRole = ui::ColorRole::kNone;
            std::shared_ptr<observer::ValueObserver<bool> > timeUnitsObserver;
        };

        void IItem::_init(
            const std::string& objectName, const otime::TimeRange& timeRange,
            const otime::TimeRange& trimmedRange, double scale,
            const ItemOptions& options, const DisplayOptions& displayOptions,
            const std::shared_ptr<ItemData>& data,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidget::_init(objectName, context, parent);
            TLRENDER_P();

            _timeRange = timeRange;
            _trimmedRange = trimmedRange;
            _scale = scale;
            _options = options;
            _displayOptions = displayOptions;
            _data = data;

            p.timeUnitsObserver = observer::ValueObserver<bool>::create(
                data->timeUnitsModel->observeTimeUnitsChanged(),
                [this](bool) { _timeUnitsUpdate(); });
        }

        IItem::IItem() :
            _p(new Private)
        {
        }

        IItem::~IItem() {}

        const otime::TimeRange& IItem::getTimeRange() const
        {
            return _timeRange;
        }

        void IItem::setTimeRange(const otime::TimeRange& value)
        {
            if (_timeRange == value)
                return;
            _timeRange = value;
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }
        
        const otime::TimeRange& IItem::getTrimmedRange() const
        {
            return _trimmedRange;
        }

        void IItem::setTrimmedRange(const otime::TimeRange& value)
        {
            if (_trimmedRange == value)
                return;
            _trimmedRange = value;
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }

        void IItem::setScale(double value)
        {
            if (value == _scale)
                return;
            _scale = value;
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }

        void IItem::setOptions(const ItemOptions& value)
        {
            _options = value;
        }

        void IItem::setDisplayOptions(const DisplayOptions& value)
        {
            if (value == _displayOptions)
                return;
            _displayOptions = value;
            _updates |= ui::Update::Size;
            _updates |= ui::Update::Draw;
        }

        ui::ColorRole IItem::getSelectRole() const
        {
            return _p->selectRole;
        }

        void IItem::setSelectRole(ui::ColorRole value)
        {
            TLRENDER_P();
            if (value == p.selectRole)
                return;
            p.selectRole = value;
            _updates |= ui::Update::Draw;
        }

        otime::RationalTime IItem::posToTime(float value) const
        {
            otime::RationalTime out = time::invalidTime;
            if (_geometry.w() > 0)
            {
                const double normalized =
                    (value - _geometry.min.x) /
                    static_cast<double>(
                        _timeRange.duration().rescaled_to(1.0).value() *
                        _scale);
                out = otime::RationalTime(
                          _timeRange.start_time() +
                          otime::RationalTime(
                              _timeRange.duration().value() * normalized,
                              _timeRange.duration().rate()))
                          .round();
                out = math::clamp(
                    out, _timeRange.start_time(),
                    _timeRange.end_time_inclusive());
            }
            return out;
        }

        int IItem::timeToPos(const otime::RationalTime& value) const
        {
            const otime::RationalTime t = value - _timeRange.start_time();
            return _geometry.min.x + t.rescaled_to(1.0).value() * _scale;
        }

        math::Box2i IItem::_getClipRect(const math::Box2i& value, double scale)
        {
            math::Box2i out;
            const math::Vector2i c = value.getCenter();
            out.min.x = (value.min.x - c.x) * scale + c.x;
            out.min.y = (value.min.y - c.y) * scale + c.y;
            out.max.x = (value.max.x - c.x) * scale + c.x;
            out.max.y = (value.max.y - c.y) * scale + c.y;
            return out;
        }

        std::string IItem::_getDurationLabel(const otime::RationalTime& value)
        {
            const otime::RationalTime rescaled =
                value.rescaled_to(_data->speed);
            return string::Format("{0}").arg(
                _data->timeUnitsModel->getLabel(rescaled));
        }

        void IItem::_timeUnitsUpdate() {}
    } // namespace TIMELINEUI
} // namespace tl
