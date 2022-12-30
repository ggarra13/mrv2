// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <boost/regex.hpp>

namespace mrv
{
    const boost::regex version_regex( const ViewerUI* ui );
    std::string media_version( const ViewerUI* ui, const file::Path& path,
                               int sum, const bool first_or_last );
}
