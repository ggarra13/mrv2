
#include "mrvFl/mrvInit.h"

#include <tlTimeline/Init.h>
#include <tlTimelineUI/Init.h>

#include <tlDevice/Init.h>

namespace mrv
{
    void init(const std::shared_ptr<tl::system::Context>& context)
    {
        using namespace tl;

        timelineui::init(context);
        // device::init(context); // @todo:
    }

} // namespace mrv
