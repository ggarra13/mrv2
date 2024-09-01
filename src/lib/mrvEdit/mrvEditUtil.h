// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/Path.h>

class ViewerUI;

namespace mrv
{
    //! Return a unique name for a new .otio temp file.
    std::string otioFilename(ViewerUI* ui);
            
    //! Remove all temporary EDLs from tmppath().  Used on exiting.
    void removeTemporaryEDLs(ViewerUI* ui);
} // namespace mrv
