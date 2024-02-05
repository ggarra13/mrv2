// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/Path.h>

class ViewerUI;

namespace mrv
{
    //! Remove all temporary EDLs from tmppath().  Used on exiting.
    void removeTemporaryEDLs(ViewerUI* ui);
} // namespace mrv
