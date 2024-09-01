// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>

class ViewerUI;

namespace mrv
{
    std::string getMatchingLayer(const std::string& layer, const ViewerUI* ui);
}
