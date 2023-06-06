
#pragma once

#include <string>

class ViewerUI;

namespace mrv
{
    std::string getMatchingLayer(const std::string& layer, const ViewerUI* ui);
}
