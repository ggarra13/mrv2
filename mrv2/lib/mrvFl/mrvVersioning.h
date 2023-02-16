// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <tlCore/Path.h>

#include <boost/regex.hpp>

namespace mrv {
const boost::regex version_regex(const ViewerUI *ui,
                                 const bool verbose = false);
std::string media_version(const ViewerUI *ui, const tl::file::Path &path,
                          int sum, const bool first_or_last);
} // namespace mrv
