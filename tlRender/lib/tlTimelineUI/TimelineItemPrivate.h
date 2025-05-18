// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlTimelineUI/ThumbnailSystem.h>
#include <tlTimelineUI/TimelineItem.h>
#include <tlTimelineUI/TransitionItem.h>

#include <tlUI/Label.h>

namespace tl
{
    namespace timelineui
    {
        struct TimelineItem::Private
        {
            std::shared_ptr<timeline::Player> player;
            otime::RationalTime currentTime = time::invalidTime;
            otime::TimeRange inOutRange = time::invalidTimeRange;
            timeline::PlayerCacheInfo cacheInfo;
            bool editable = false;
            bool stopOnScrub = true;
            std::function<void(const std::vector<timeline::MoveData>&)>
                moveCallback;
            std::shared_ptr<observer::Value<bool> > scrub;
            std::shared_ptr<observer::Value<otime::RationalTime> > timeScrub;
            std::vector<int> frameMarkers;
            int minimumHeight = 0;
            std::shared_ptr<timelineui::ThumbnailGenerator> thumbnailGenerator;
            std::unordered_map<std::string,
                               std::vector<std::shared_ptr<image::Glyph>> > labelsCache;
            struct Track
            {
                int index = 0;
                TrackType type = TrackType::kNone;
                otime::TimeRange timeRange;
                std::shared_ptr<ui::Label> label;
                std::shared_ptr<ui::Label> durationLabel;
                std::vector<std::shared_ptr<IItem> > items;
                std::vector<int> otioIndexes;
                std::vector<std::shared_ptr<TransitionItem> > transitions;
                math::Size2i size;
                int clipHeight = 0;
            };
            std::vector<Track> tracks;

            struct SizeData
            {
                bool sizeInit = true;
                int margin = 0;
                int border = 0;
                int handle = 0;
                image::FontInfo fontInfo = image::FontInfo("", 0);
                image::FontMetrics fontMetrics;

                math::Vector2i scrollPos;
            };
            SizeData size;

            struct DrawData
            {
                std::vector<math::Box2i> dropTargets;
            };
            DrawData draw;

            enum class MouseMode { kNone, CurrentTime, Item };
            struct MouseItemData
            {
                MouseItemData();
                MouseItemData(
                    const std::shared_ptr<IItem>&, int index, int track);

                ~MouseItemData();

                std::shared_ptr<IItem> p;
                int index = -1;
                int track = -1;
                math::Box2i geometry;
            };
            struct MouseItemDropTarget
            {
                int index = -1;
                int track = -1;
                math::Box2i mouse;
                math::Box2i draw;
            };
            struct MouseData
            {
                MouseMode mode = MouseMode::kNone;
                std::vector<std::shared_ptr<MouseItemData> > items;
                std::vector<MouseItemDropTarget> dropTargets;
                int currentDropTarget = -1;
            };
            MouseData mouse;

            std::shared_ptr<observer::ValueObserver<otime::RationalTime> >
                currentTimeObserver;
            std::shared_ptr<observer::ValueObserver<otime::TimeRange> >
                inOutRangeObserver;
            std::shared_ptr<observer::ValueObserver<timeline::PlayerCacheInfo> >
                cacheInfoObserver;

            std::shared_ptr<IItem> getAssociated(
                const std::shared_ptr<IItem>&, int& index,
                int& trackIndex) const;

            std::vector<MouseItemDropTarget>
            getDropTargets(const math::Box2i& geometry, int index, int track);
        };
    } // namespace timelineui
} // namespace tl
