// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <tlCore/Path.h>

class ViewerUI;

namespace mrv
{
    //! Returns true if the file is an otio file (ie. extension is .otio).
    bool isOtioFile(const tl::file::Path& file);

    //! Returns true or false whether the filename is a temporary EDL.
    bool isTemporaryEDL(const tl::file::Path& path);

    //! Remove all temporary EDLs from tmppath().  Used on exiting.
    void removeTemporaryEDLs(ViewerUI* ui);
} // namespace mrv
