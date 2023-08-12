// SPDX-License-Identifier: Apache-2.0
// Copyright Contributors to the OpenTimelineIO project

#include "mrvTimeline/mrvEdit.h"

namespace tl
{
    namespace timelineui
    {
        namespace
        {
            int getChildIndex(const otio::Item* item)
            {
                int out = -1;
                if (item && item->parent())
                {
                    const auto& children = item->parent()->children();
                    for (int i = 0; i < children.size(); ++i)
                    {
                        if (item == children[i].value)
                        {
                            out = i;
                            break;
                        }
                    }
                }
                return out;
            }
        } // namespace

        otio::SerializableObject::Retainer<otio::Timeline> slice(
            const otio::Timeline* timeline, const otio::Item* item,
            const otime::RationalTime& time)
        {
            const std::string s = timeline->to_json_string();
            otio::SerializableObject::Retainer<otio::Timeline> out(
                dynamic_cast<otio::Timeline*>(
                    otio::Timeline::from_json_string(s)));

            const auto tracks = out->tracks()->children();
            const int itemTrackIndex = getChildIndex(item->parent());
            const int itemIndex = getChildIndex(item);
            if (itemTrackIndex >= 0 && itemTrackIndex < tracks.size())
            {
                if (auto track = dynamic_cast<otio::Track*>(
                        tracks[itemTrackIndex].value))
                {
                    const auto children = track->children();
                    if (itemIndex >= 0 && itemIndex < children.size())
                    {
                        // Get item range
                        const otime::TimeRange range =
                            track->trimmed_range_of_child_at_index(itemIndex);

                        if (!range.contains(time))
                            return out;

                        // Calculate the first and second range for each slice.
                        const otime::TimeRange first_source_range(
                            item->trimmed_range().start_time(),
                            time - range.start_time());
                        const otime::TimeRange second_source_range(
                            first_source_range.start_time() +
                                first_source_range.duration(),
                            range.duration() - first_source_range.duration());

                        // Clone the original item.
                        auto first_item =
                            dynamic_cast<otio::Item*>(item->clone());
                        auto second_item =
                            dynamic_cast<otio::Item*>(item->clone());

                        // Remove the original child
                        track->remove_child(itemIndex);

                        // Insert the two items
                        first_item->set_source_range(first_source_range);
                        track->insert_child(itemIndex, first_item);
                        second_item->set_source_range(second_source_range);
                        track->insert_child(itemIndex + 1, second_item);
                    }
                }
            }
            return out;
        }

        otio::SerializableObject::Retainer<otio::Timeline>
        remove(const otio::Timeline* timeline, const otio::Item* item)
        {
            const std::string s = timeline->to_json_string();
            otio::SerializableObject::Retainer<otio::Timeline> out(
                dynamic_cast<otio::Timeline*>(
                    otio::Timeline::from_json_string(s)));

            const auto tracks = out->tracks()->children();
            const int itemTrackIndex = getChildIndex(item->parent());
            const int itemIndex = getChildIndex(item);
            if (itemTrackIndex >= 0 && itemTrackIndex < tracks.size())
            {
                if (auto track = dynamic_cast<otio::Track*>(
                        tracks[itemTrackIndex].value))
                {
                    const auto children = track->children();
                    if (itemIndex >= 0 && itemIndex < children.size())
                    {
                        // Remove the original child
                        track->remove_child(itemIndex);
                    }
                }
            }
            return out;
        }
    } // namespace timelineui
} // namespace tl
