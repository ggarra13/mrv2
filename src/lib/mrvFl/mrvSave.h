// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlIO/IO.h>

#include "mrvSaveOptions.h"

class ViewerUI;

namespace mrv
{

    //! Save single frame.  Returns 0 if successful, -1 if not.
    int save_single_frame(
        const std::string& file, const ViewerUI* ui,
        SaveOptions options = SaveOptions());

    void save_movie(
        const std::string& file, const ViewerUI* ui,
        SaveOptions options = SaveOptions());

} // namespace mrv
