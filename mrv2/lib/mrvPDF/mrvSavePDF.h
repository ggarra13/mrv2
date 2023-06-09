
#pragma once

#include <string>
#include <vector>
#include <memory>

#include "mrvDraw/Annotation.h"

class ViewerUI;

namespace mrv
{
    using namespace tl;

    void save_pdf(
        const std::string& file,
        const std::vector< std::shared_ptr< draw::Annotation >>& annotations,
        const ViewerUI* ui);
} // namespace mrv
