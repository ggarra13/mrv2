// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvEdit/mrvEditAlgorithm.h"

#include "opentimelineio/effect.h"
#include "opentimelineio/gap.h"
#include "opentimelineio/track.h"
#include "opentimelineio/transition.h"


namespace
{

// We are not testing values outside of one million seconds.
// At one million second, and double precision, the smallest
// resolvable number that can be added to one million and return
// a new value one million + epsilon is 5.82077e-11.
//
// This was calculated by searching iteratively for epsilon
// around 1,000,000, with epsilon starting from 1 and halved
// at every iteration, until epsilon when added to 1,000,000
// resulted in 1,000,000.
    constexpr double double_epsilon = 5.82077e-11;

    inline bool
    isEqual(double a, double b)
    {
        return (std::abs(a - b) <= double_epsilon);
    }

} // namespace

namespace mrv
{
    namespace algo
    {

        void
        insert(
            OTIO_NS::Item* const         insert_item,
            OTIO_NS::Composition*        composition,
            OTIO_NS::RationalTime const& time,
            const bool                   remove_transitions,
            OTIO_NS::Item*               fill_template,
            OTIO_NS::ErrorStatus*        error_status)
        {
            // Check for transitions to remove first.
            if (remove_transitions)
            {
                OTIO_NS::TimeRange range(time, OTIO_NS::RationalTime(1.0, time.rate()));
                auto transitions = composition->find_children<OTIO_NS::Transition>(
                    error_status,
                    range,
                    true);
                if (!transitions.empty())
                {
                    for (const auto& transition : transitions)
                    {
                        int index = composition->index_of_child(transition);
                        if (index < 0
                            || static_cast<size_t>(index)
                            >= composition->children().size())
                            continue;
                        composition->remove_child(transition);
                    }
                }
            }

            const OTIO_NS::TimeRange composition_range = composition->trimmed_range();

            // Find the item to insert into.
            auto item = OTIO_NS::dynamic_retainer_cast<OTIO_NS::Item>(
                composition->child_at_time(time, error_status));
            if (!item)
            {
                if (time >= composition_range.end_time_exclusive())
                {
                    // Append the item and a possible fill (gap).
                    const OTIO_NS::RationalTime fill_duration =
                        time - composition_range.end_time_exclusive();
                    if (!isEqual(fill_duration.value(), 0.0))
                    {
                        const OTIO_NS::TimeRange fill_range = OTIO_NS::TimeRange(
                            OTIO_NS::RationalTime(0.0, fill_duration.rate()),
                            fill_duration);
                        if (!fill_template)
                            fill_template = new OTIO_NS::Gap(fill_range);
                        composition->append_child(fill_template);
                    }
                    composition->append_child(insert_item);
                }
                else if (time < composition_range.start_time())
                {
                    composition->insert_child(0, insert_item);
                }
                else
                {
                    if (error_status)
                        *error_status = OTIO_NS::ErrorStatus::INTERNAL_ERROR;
                }
                return;
            }

            const int       index = composition->index_of_child(item);
            const OTIO_NS::TimeRange range = composition->trimmed_range_of_child_at_index(index);
            int insert_index = index;

            // Item is partially split
            bool split = false;
            const OTIO_NS::TimeRange first_source_range(
                item->trimmed_range().start_time(),
                time - range.start_time());
            if (!isEqual(first_source_range.duration().value(), 0.0))
            {
                split = true;
                item->set_source_range(first_source_range);
                ++insert_index;
            }

            // Insert the new item
            composition->insert_child(insert_index, insert_item);
            const OTIO_NS::TimeRange insert_range =
                composition->trimmed_range_of_child_at_index(insert_index);

            // Second item from splitting item
            if (split)
            {
                const OTIO_NS::TimeRange second_source_range(
                    first_source_range.start_time() +
                    first_source_range.duration(),
                    range.end_time_exclusive() - time);
                // Clone the item for the second partially overwritten item.
                if (!isEqual(second_source_range.duration().value(), 0.0))
                {
                    auto second_item = dynamic_cast<OTIO_NS::Item*>(item->clone());
                    second_item->set_source_range(second_source_range);
                    composition->insert_child(insert_index + 1, second_item);
                }
            }
        }
    }
}
