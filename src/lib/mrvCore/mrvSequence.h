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
        std::string suffix;
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

            if (a.view < b.view)
                return true;
            else if (a.view > b.view)
                return false;

            int anum = atoi(a.number.c_str());
            int bnum = atoi(b.number.c_str());
            if (anum < bnum)
                return true;
            else if (anum > bnum)
                return false;

            if (a.suffix < b.suffix)
                return true;
            else if(a.suffix > b.suffix)
                return false;

            if (a.ext < b.ext)
                return true;

            return false;
        }
    };

} // namespace mrv
