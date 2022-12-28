#pragma once

namespace mrv
{
    std::string media_version( const ViewerUI* ui, const file::Path& path,
                               int sum, const bool first_or_last );
}
