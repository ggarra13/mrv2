// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlIO/IO.h>

#include "mrvSaveOptions.h"

class ViewerUI;

namespace mrv
{
    void save_movie(
        const std::string& file, const ViewerUI* ui,
        SaveOptions options = SaveOptions());

}
