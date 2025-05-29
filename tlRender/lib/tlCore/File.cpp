
#include <tlCore/File.h>

#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace tl
{
    namespace file
    {
        bool isNetwork(const std::string& path)
        {
            static const std::string schemes[] = {"crypto", "ftp",       "http",
                                                  "https",  "httpproxy", "rtmp",
                                                  "rtp",    "tcp",       "tls"};

            for (const std::string& scheme : schemes)
            {
                if (path.find(scheme + ":") == 0)
                    return true;
            }
            return false;
        }

        bool isReadable(const std::string& fileName)
        {
            fs::path p(fileName);

            const std::string& filePath = p.generic_string();
            if (filePath.empty())
                return false;

            if (isNetwork(filePath))
                return true;

            std::ifstream f(filePath);
            if (f.is_open())
            {
                f.close();
                return true;
            }

            return false;
        }
    } // namespace file
} // namespace tl
