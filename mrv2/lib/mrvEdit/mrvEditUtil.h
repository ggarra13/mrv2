// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/Path.h>

class ViewerUI;

namespace mrv
{
    //! Returns true or false whether the filename is a temporary EDL.
    bool isTemporaryEDL(const tl::file::Path& path);

    //! Returns true or false whether the filename is a temporary EDL.
    bool isTemporaryEDL(const std::string& filename);

    //! Remove all temporary EDLs from tmppath().  Used on exiting.
    void removeTemporaryEDLs(ViewerUI* ui);
} // namespace mrv
