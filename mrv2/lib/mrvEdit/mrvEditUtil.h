
#include <tlCore/Path.h>

namespace mrv
{
    //! Returns true or false whether the filename is a temporary EDL.
    bool isTemporaryEDL(const tl::file::Path& path);

    //! Returns true or false whether the filename is a temporary EDL.
    bool isTemporaryEDL(const std::string& filename);
} // namespace mrv
