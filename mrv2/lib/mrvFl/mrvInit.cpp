
#include "mrvFl/mrvInit.h"

#include <tlTimeline/Init.h>

#include <tlDevice/Init.h>

namespace mrv
{
    void init(const std::shared_ptr<tl::system::Context>& context)
    {
        using namespace tl;

        timeline::init(context);
        // device::init(context); // @todo:
    }

} // namespace mrv
