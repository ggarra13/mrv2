
#include <opentimelineio/composition.h>
#include <opentimelineio/item.h>
#include <opentimelineio/errorStatus.h>

#include <opentime/rationalTime.h>

namespace mrv
{
    namespace algo
    {
        void
        insert(
            OTIO_NS::Item* const         insert_item,
            OTIO_NS::Composition*        composition,
            OTIO_NS::RationalTime const& time,
            bool const          remove_transitions = true,
            OTIO_NS::Item*               fill_template = nullptr,
            OTIO_NS::ErrorStatus*        error_status = nullptr);
    }
}
