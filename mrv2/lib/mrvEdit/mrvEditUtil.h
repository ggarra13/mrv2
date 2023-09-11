
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
