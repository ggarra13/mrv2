#include <fstream>
#include <string>
#include <algorithm>

#include <tlCore/OS.h>

#include "mrvCore/mrvI8N.h"

namespace mrv
{
    std::string get_os_version()
    {
        tl::os::SystemInfo info = tl::os::getSystemInfo();
        std::string os_version;
#ifdef __linux__
        std::ifstream os_release("/etc/os-release");
        std::string line;

        if (os_release.is_open())
        {
            while (std::getline(os_release, line))
            {
                if (line.find("PRETTY_NAME=") != std::string::npos)
                {
                    os_version = line.substr(line.find("=") + 1);
                    // Remove quotes from the version string
                    os_version.erase(
                        std::remove(os_version.begin(), os_version.end(), '"'),
                        os_version.end());
                    break;
                }
            }
            os_release.close();
        }
        if (os_version.empty())
            os_version = info.name;
#else
        os_version = info.name;
#endif
        os_version = _("Running on: ") + os_version;
        return os_version;
    }
} // namespace mrv
