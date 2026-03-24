
#include <string>

#ifdef _WIN32
#  include <windows.h>
#  include <aclapi.h>

static std::string get_owner_windows(const std::string& path) {
    PSID pSidOwner = NULL;
    PSECURITY_DESCRIPTOR pSD = NULL;
    char accountName[256], domainName[256];
    DWORD dwAcctName = 256, dwDomainName = 256;
    SID_NAME_USE eUse = SidTypeUnknown;

    if (GetNamedSecurityInfoA(path.c_str(), SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION, 
                              &pSidOwner, NULL, NULL, NULL, &pSD) == ERROR_SUCCESS) {
        if (LookupAccountSidA(NULL, pSidOwner, accountName, &dwAcctName, 
                              domainName, &dwDomainName, &eUse)) {
            LocalFree(pSD);
            return std::string(accountName);
        }
    }
    if (pSD) LocalFree(pSD);
    return "Unknown";
}
#else
#  include <sys/stat.h>
#  include <pwd.h>

static std::string get_owner_unix(const std::string& path)
{
    struct stat info;
    if (stat(path.c_str(), &info) == 0) {
        struct passwd *pw = getpwuid(info.st_uid);
        if (pw) return pw->pw_name;
    }
    return "Unknown";
}

#endif

namespace mrv
{
    namespace file
    {
        std::string get_owner(const std::string& path) {
#ifdef _WIN32
            return get_owner_windows(path);
#else
            return get_owner_unix(path);
#endif
        }
    }
}

