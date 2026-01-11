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
                std::vector<timeline::MoveData> moveData;
                for (const auto& item : p.mouse.items)
                {
                    const int fromTrack = item->track;
                    const int fromIndex = item->index;
                    const int fromOtioIndex =
                        p.tracks[fromTrack].otioIndexes[fromIndex];
                    const int toTrack = dropTarget.track +
                                        (item->track - p.mouse.items[0]->track);
                    const int toIndex = dropTarget.index;
                    int toOtioIndex = toIndex;
                    if (toOtioIndex < p.tracks[toTrack].otioIndexes.size())
                    {
                        toOtioIndex = p.tracks[toTrack].otioIndexes[toIndex];
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
                auto otioTimeline = timeline::move(
                    p.player->getTimeline()->getTimeline().value, moveData);
                p.player->getTimeline()->setTimeline(otioTimeline);
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
                    const otime::RationalTime startTime = posToTime(x) - _timeRange.start_time();
                    otime::TimeRange timeRange = transition->getTimeRange();   
                    const otime::RationalTime& duration = timeRange.duration();
                    timeRange = otime::TimeRange(startTime, duration);
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
                    const otime::TimeRange& itemRange = transitionItems[1]->getTimeRange();

                    const int transitionOtioIndex =
                        p.tracks[transitionTrack].otioTransitionIndexes[transitionIndex];
                    const otime::RationalTime in_offset = itemRange.start_time() -
                                                          timeRange.start_time();
                    const otime::RationalTime out_offset = timeRange.end_time_exclusive() -
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
                auto otioTimeline = timeline::move(
                    p.player->getTimeline()->getTimeline().value, moveData);
                p.player->getTimeline()->setTimeline(otioTimeline);
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
                const otime::RationalTime time = posToTime(event.pos.x);
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
                        otime::TimeRange timeRange = transitionItem->getTimeRange();

                        // Get item time ranges for transitions.
                        const int transitionTrack = item->track;
                        std::vector<otime::TimeRange> itemRanges;
                        _getTransitionTimeRanges(itemRanges, transitionTrack,
                                                 timeRange);
                        
                        move = math::Box2i(
                            g.min + _mouse.pos - _mouse.pressPos,
                            g.getSize() );
                            
                        const otime::RationalTime& startTime = posToTime(move.x());
                        const otime::RationalTime& duration  = timeRange.duration();
                        
                        timeRange = otime::TimeRange(startTime, duration);
                        
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
                        if (p.mouse.dropTargets[i].mouse.contains(event.pos))
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
