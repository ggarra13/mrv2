
#include <tlCore/OS.h>

#include "mrvCore/mrvI8N.h"

namespace mrv
{
    std::string get_os_version()
    {
        tl::os::SystemInfo info = tl::os::getSystemInfo();
        std::string os_version = info.name;
        os_version = _("Running on: ") + os_version;
        return os_version;
    }
}
