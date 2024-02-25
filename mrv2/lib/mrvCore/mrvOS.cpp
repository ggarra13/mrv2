
#include <fstream>
#include <algorithm>

#include "mrvCore/mrvI8N.h"
#include "mrvCore/mrvOS.h"

namespace mrv
{
    std::string get_os_version()
    {
        std::string os_version = "Unknown OS";

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

#elif __APPLE__

        int mib[2];
        size_t len;
        char* version;

        mib[0] = CTL_KERN;
        mib[1] = KERN_OSRELEASE;
        sysctl(mib, 2, NULL, &len, NULL, 0);
        version = new char[len];
        sysctl(mib, 2, version, &len, NULL, 0);

        os_version = "macOS " + std::string(version);
        delete[] version;

#elif __WIN32__
        OSVERSIONINFOEX osvi;
        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

        if (!GetVersionEx((OSVERSIONINFO*)&osvi))
        {
            std::cerr << "Failed to retrieve OS version." << std::endl;
            return "";
        }

        switch (osvi.dwPlatformId)
        {
        case VER_PLATFORM_WIN32_NT:
            if (osvi.dwMajorVersion <= 4)
            {
                os_version = "Windows NT ";
            }
            else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
            {
                os_version = "Windows 2000 ";
            }
            else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1)
            {
                os_version = "Windows XP ";
            }
            else if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 2)
            {
                os_version = "Windows Server 2003 ";
            }
            else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 0)
            {
                os_version = "Windows Vista ";
            }
            else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 1)
            {
                os_version = "Windows 7 ";
            }
            else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 2)
            {
                os_version = "Windows 8 ";
            }
            else if (osvi.dwMajorVersion == 6 && osvi.dwMinorVersion == 3)
            {
                os_version = "Windows 8.1 ";
            }
            else if (osvi.dwMajorVersion == 10 && osvi.dwMinorVersion == 0)
            {
                os_version = "Windows 10 ";
            }
            else
            {
                os_version = "Unknown Windows ";
            }

            // Display version, service pack (if any), and build number.
            os_version += std::to_string(osvi.dwMajorVersion) + "." +
                          std::to_string(osvi.dwMinorVersion);
            if (osvi.wServicePackMajor != 0 || osvi.wServicePackMinor != 0)
            {
                os_version += " SP" + std::to_string(osvi.wServicePackMajor);
            }
            os_version += ", Build " + std::to_string(osvi.dwBuildNumber);
            break;
        default:
            os_version = "Unknown Windows";
            break;
        }

#endif
        os_version = _("Running on: ") + os_version;
        return os_version;
    }
} // namespace mrv
