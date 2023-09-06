

#include "mrvCore/mrvHome.h"
#include "mrvEdit/mrvEditUtil.h"

namespace mrv
{
    using namespace tl;

    bool isTemporaryEDL(const file::Path& path)
    {
        const std::string tmpdir = tmppath() + '/';

        auto dir = path.getDirectory();
        auto base = path.getBaseName();
        auto extension = path.getExtension();
        if (dir != tmpdir || base != "EDL." || extension != ".otio")
        {
            return false;
        }
        return true;
    }

    bool isTemporaryEDL(const std::string& filename)
    {
        file::Path path(filename);
        return isTemporaryEDL(path);
    }
} // namespace mrv
