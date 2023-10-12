// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>
#include <vector>

#include <mrvCore/mrvString.h>

namespace mrv
{

    struct Sequence
    {
        std::string root;
        std::string number;
        std::string view;
        std::string ext;
    };

    typedef std::vector< Sequence > SequenceList;

    struct SequenceSort
    {
        // true if a < b, else false
        bool operator()(const Sequence& a, const Sequence& b) const
        {
            if (a.root < b.root)
                return true;
            else if (a.root > b.root)
                return false;

            if (a.ext < b.ext)
                return true;
            else if (a.ext > b.ext)
                return false;

            if (a.view < b.view)
                return true;
            else if (a.view > b.view)
                return false;

            if (atoi(a.number.c_str()) < atoi(b.number.c_str()))
                return true;
            return false;
        }
    };

    std::string get_short_view(bool left);
    std::string get_long_view(bool left);

    //! Parse a %v or %V fileroot and return the appropiate view name.
    std::string parse_view(const std::string& fileroot, bool left = true);

} // namespace mrv
