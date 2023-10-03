// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

namespace mrv
{
    inline bool FilesPanelOptions::operator==(const FilesPanelOptions& b) const
    {
        return (filterEDL == b.filterEDL);
    }

    inline bool FilesPanelOptions::operator!=(const FilesPanelOptions& b) const
    {
        return !(*this == b);
    }
} // namespace mrv
