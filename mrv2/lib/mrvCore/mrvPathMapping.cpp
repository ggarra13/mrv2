// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <iostream>

#include "FL/Enumerations.H"

#include "mrvCore/mrvI8N.h"

#include "mrvWidgets/mrvBrowser.h"

namespace mrv
{

    void fill_path_mappings(mrv::Browser* b)
    {
        int w2 = b->w() / 4;
        int w1 = w2 * 3;

        static int widths[] = {w1 - 20, w2, 0};
        b->column_widths(widths);
        b->column_char('\t'); // tabs as column delimiters

        // Labels
        b->add(_("@B12@C7@b@.Remote\t@B12@C7@b@.Local"));
    }

} // namespace mrv
