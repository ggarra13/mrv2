// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "TimelineItem.h"
#include "TimelineItemPrivate.h"

namespace tl
{
    namespace TIMELINEUI
    {
        void TimelineItem::_mouseReleaseEventMove(ui::MouseClickEvent& event)
        {
            TLRENDER_P();
            if (!p.mouse.items.empty() && p.mouse.currentDropTarget != -1)
            {
                const auto& dropTarget =
                    p.mouse.dropTargets[p.mouse.currentDropTarget];
                const int tracksSize = static_cast<int>(p.tracks.size());

                // Time in the timeline where the dragged item is dropped: the
                // start of the item it is dropped in front of, or the end of
                // the last item when dropped at the end of the track.  The
                // associated items (like the audio of a video clip) live in
                // other tracks, whose items do not necessarily have the same
                // indexes (gaps, different clip splits), so they are placed
                // by time instead of by index.
                OTIO_NS::RationalTime dropTime(0.0, 24.0);
                bool hasDropTime = false;
                if (dropTarget.track >= 0 && dropTarget.track < tracksSize)
                {
                    const auto& items = p.tracks[dropTarget.track].items;
                    if (dropTarget.index >= 0 &&
                        dropTarget.index < static_cast<int>(items.size()))
                    {
                        dropTime =
                            items[dropTarget.index]->getTimeRange().start_time();
                        hasDropTime = true;
                    }
                    else if (!items.empty())
                    {
                        dropTime =
                            items.back()->getTimeRange().end_time_exclusive();
                        hasDropTime = true;
                    }
                }

                // Index of the first item in the track starting at or after
                // the drop time (the item count if there is none).
                auto findDropIndex = [&](int track)
                {
                    const auto& items = p.tracks[track].items;
                    const double rate =
                        dropTime.rate() > 0.0 ? dropTime.rate() : 24.0;
                    const double tolerance = 0.5 / rate;
                    const double time = dropTime.to_seconds();
                    for (int i = 0; i < static_cast<int>(items.size()); ++i)
                    {
                        if (!items[i])
                            continue;
                        const double start =
                            items[i]->getTimeRange().start_time().to_seconds();
                        if (start >= time - tolerance)
                            return i;
                    }
                    return static_cast<int>(items.size());
                };

                std::vector<timeline::MoveData> moveData;
                for (const auto& item : p.mouse.items)
                {
                    const int fromTrack = item->track;
                    const int fromIndex = item->index;
                    const int toTrack = dropTarget.track +
                                        (item->track - p.mouse.items[0]->track);
                    if (fromTrack < 0 || fromTrack >= tracksSize ||
                        toTrack < 0 || toTrack >= tracksSize)
                        continue;

                    const auto& fromOtioIndexes =
                        p.tracks[fromTrack].otioIndexes;
                    if (fromIndex < 0 ||
                        fromIndex >= static_cast<int>(fromOtioIndexes.size()))
                        continue;
                    const int fromOtioIndex = fromOtioIndexes[fromIndex];

                    // The clicked item uses the drop target index as is.
                    int toIndex = dropTarget.index;
                    if (hasDropTime && toTrack != dropTarget.track)
                        toIndex = findDropIndex(toTrack);

                    const auto& toOtioIndexes = p.tracks[toTrack].otioIndexes;
                    int toOtioIndex;
                    if (toIndex >= 0 &&
                        toIndex < static_cast<int>(toOtioIndexes.size()))
                    {
                        toOtioIndex = toOtioIndexes[toIndex];
                    }
                    else
                    {
                        // Dropped past the last item: the otio index must be
                        // past the last item's otio index (transitions
                        // included), not just the item count.
                        toOtioIndex =
                            toOtioIndexes.empty() ? 0 : toOtioIndexes.back() + 1;
                    }
                    moveData.push_back(
                        {
                            timeline::MoveType::Clip,
                            fromTrack, fromIndex, fromOtioIndex,
                            toTrack, toIndex, toOtioIndex
                        });
                    item->p->hide();
                }
                if (p.moveCallback)
                    p.moveCallback(moveData);
            }
            else if (!p.mouse.items.empty() &&
                     p.mouse.mode == Private::MouseMode::Transition)
            {
                std::vector<timeline::MoveData> moveData;
                for (const auto& item : p.mouse.items)
                {
                    const std::shared_ptr<IItem> transition = item->p;
                    const int transitionIndex = item->index;
                    int x = transition->getGeometry().x();
                    const OTIO_NS::RationalTime startTime = posToTime(x) - _timeRange.start_time();
                    OTIO_NS::TimeRange timeRange = transition->getTimeRange();
                    const OTIO_NS::RationalTime& duration = timeRange.duration();
                    timeRange = OTIO_NS::TimeRange(startTime, duration);
                    const math::Size2i& sizeHint = transition->getSizeHint();
                    transition->setTimeRange(timeRange);
                    const int y = item->geometry.y();
                    item->p->setGeometry(
                        math::Box2i(
                            _geometry.min.x + timeRange.start_time()
                                                      .rescaled_to(1.0)
                                                      .value() *
                                                  _scale,
                            y, sizeHint.w, sizeHint.h));

                    // Clamp on transition items.
                    std::vector<IBasicItem*> transitionItems;
                    const int transitionTrack = item->track;
                    _getTransitionItems(transitionItems, transitionTrack, timeRange);
                    const OTIO_NS::TimeRange& itemRange = transitionItems[1]->getTimeRange();

                    const int transitionOtioIndex =
                        p.tracks[transitionTrack].otioTransitionIndexes[transitionIndex];
                    const OTIO_NS::RationalTime in_offset = itemRange.start_time() -
                                                          timeRange.start_time();
                    const OTIO_NS::RationalTime out_offset = timeRange.end_time_exclusive() -
                                                           itemRange.start_time();
                    moveData.push_back(
                        {
                            timeline::MoveType::Transition,
                            transitionTrack, transitionIndex, transitionOtioIndex,
                            transitionTrack, transitionIndex, transitionOtioIndex,
                            in_offset, out_offset
                        });
                }
                if (p.moveCallback)
                    p.moveCallback(moveData);
            }
            if (!p.mouse.dropTargets.empty())
            {
                p.mouse.dropTargets.clear();
                _updates |= ui::Update::Draw;
            }
            p.mouse.currentDropTarget = -1;
        }

        void TimelineItem::_mouseMoveEventMove(ui::MouseMoveEvent& event)
        {
            TLRENDER_P();

            switch (p.mouse.mode)
            {
            case Private::MouseMode::kNone:
                break;
            case Private::MouseMode::CurrentTime:
            {
                const OTIO_NS::RationalTime time = posToTime(event.pos.x);
                p.timeScrub->setIfChanged(time);
                p.player->seek(time);
                break;
            }
            case Private::MouseMode::Transition:
            {
                if (!p.mouse.items.empty())
                {
                    _mouse.pos.y = _mouse.pressPos.y;
                    const int offset = _mouse.pos.x - _mouse.pressPos.x;
                    math::Box2i move;

                    for (const auto& item : p.mouse.items)
                    {
                        const math::Box2i& g = item->geometry;
                        auto transitionItem = dynamic_cast<TransitionItem*>(item->p.get());
                        OTIO_NS::TimeRange timeRange = transitionItem->getTimeRange();

                        // Get item time ranges for transitions.
                        const int transitionTrack = item->track;
                        std::vector<otime::TimeRange> itemRanges;
                        _getTransitionTimeRanges(itemRanges, transitionTrack,
                                                 timeRange);

                        move = math::Box2i(
                            g.min + _mouse.pos - _mouse.pressPos,
                            g.getSize() );

                        const OTIO_NS::RationalTime& startTime = posToTime(move.x());
                        const OTIO_NS::RationalTime& duration  = timeRange.duration();

                        timeRange = OTIO_NS::TimeRange(startTime, duration);

                        // Clamp on clips.
                        if (timeRange.start_time() - _timeRange.start_time() >=
                            itemRanges[0].end_time_exclusive())
                        {
                            continue;
                        }

                        if (timeRange.end_time_inclusive() - _timeRange.start_time() <
                            itemRanges[1].start_time())
                        {
                            continue;
                        }

                        // Clamp on other transitions.
                        if (_transitionIntersects(item->p, transitionTrack, timeRange))
                        {
                            continue;
                        }

                        item->p->setGeometry(move);
                    }
                }
                break;
            }
            case Private::MouseMode::Item:
            {
                if (!p.mouse.items.empty())
                {
                    for (const auto& item : p.mouse.items)
                    {
                        const math::Box2i& g = item->geometry;
                        item->p->setGeometry(math::Box2i(
                            g.min + _mouse.pos - _mouse.pressPos, g.getSize()));
                    }

                    int dropTarget = -1;
                    for (size_t i = 0; i < p.mouse.dropTargets.size(); ++i)
                    {
                        if (math::contains(p.mouse.dropTargets[i].mouse,
                                           event.pos))
                        {
                            dropTarget = i;
                            break;
                        }
                    }
                    if (dropTarget != p.mouse.currentDropTarget)
                    {
                        for (const auto& item : p.mouse.items)
                        {
                            item->p->setSelectRole(
                                dropTarget != -1 ? ui::ColorRole::Green
                                                 : ui::ColorRole::Checked);
                        }
                        p.mouse.currentDropTarget = dropTarget;
                        _updates |= ui::Update::Draw;
                    }
                }
                break;
            }
            default:
                break;
            }
        }
    }
}
