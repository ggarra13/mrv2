
#ifdef __linux__
#    include <stdlib.h>
#endif

#ifdef _WIN32
#    include <windows.h>
#    include <Shellapi.h>

#    include <tlCore/Path.h>

#endif

#include <iostream>

#include "mrvCore/mrvHelpers.h"
#include "mrvCore/mrvI8N.h"

#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "helpers";
}

namespace mrv
{
#ifndef __APPLE__
    int file_manager_show_uri(const std::string& file)
    {
        int ret = -1;
#    ifdef __linux__
        char buf[4096];
        const std::string uri = "file://localhost" + file;
        snprintf(
            buf, 4096,
            "dbus-send --session --print-reply "
            "--dest=org.freedesktop.FileManager1 "
            "--type=method_call /org/freedesktop/FileManager1 "
            "org.freedesktop.FileManager1.ShowItems array:string:\"%s\" "
            "string:\"\"",
            uri.c_str());

        FILE* cmdOutput = popen(buf, "r");
        if (cmdOutput)
        {
            char outputBuffer[128];
            while (fgets(outputBuffer, sizeof(outputBuffer), cmdOutput) !=
                   nullptr)
            {
                // Print the output or log it for debugging
                // Example: printf("Command output: %s", outputBuffer);
            }
            pclose(cmdOutput);
        }

#    elif _WIN32

        tl::file::Path fullpath(file);
        const std::string& path = fullpath.getDirectory();

        ret = (INT_PTR)(ShellExecute(
            NULL, "explore", path.c_str(), NULL, NULL, SW_SHOWNORMAL));

        if (ret <= 32)
        {
            // An error occurred.
            switch (ret)
            {
            case ERROR_FILE_NOT_FOUND:
                LOG_ERROR(_("Error: File not found."));
                break;
            case ERROR_PATH_NOT_FOUND:
                LOG_ERROR(_("Error: Path not found."));
                break;
            default:
                LOG_ERROR(_("Error occurred with code: ") << ret);
                break;
            }
            return 1;
        }

        return 0;

#    endif
        return ret;
    }
#endif

} // namespace mrv
