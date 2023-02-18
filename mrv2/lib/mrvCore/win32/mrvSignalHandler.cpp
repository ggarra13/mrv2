

#include <csignal>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

#include <windows.h>

#include <boost/filesystem.hpp>
namespace fs = boost::filesystem;

#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvSignalHandler.h"
#include "mrvCore/mrvStackTrace.h"

namespace mrv
{

    SignalHandler::SignalHandler()
    {
        install_signal_handler();
    }

    SignalHandler::~SignalHandler()
    {
        restore_signal_handler();
    }

    void removeLockFile()
    {
        std::string lockfile = mrv::lockfile();
        if (fs::exists(lockfile))
        {
            if (!fs::remove(lockfile))
                std::cerr << "Could not remove lockfile " << lockfile
                          << std::endl;
        }
    }

    void callback(int signal)
    {
        removeLockFile();

        std::cerr << "GOT SIGNAL " << signal << std::endl;

        printStackTrace();

        exit(1);
    }

    LONG WINAPI exceptionHandler(EXCEPTION_POINTERS* exceptionInfo)
    {
        removeLockFile();

        DWORD exceptionCode = exceptionInfo->ExceptionRecord->ExceptionCode;
        std::cerr << "Exception " << exceptionCode << " caught" << std::endl;
        // Do any necessary cleanup or logging here
        printStackTrace();

        return EXCEPTION_EXECUTE_HANDLER;
    }

    void SignalHandler::install_signal_handler()
    {

        // Set up exception handler
        SetUnhandledExceptionFilter(exceptionHandler);

        // Set up signal handlers
        std::signal(SIGSEGV, callback);
#if 1
        std::signal(SIGABRT, callback);
#endif
    }

    void SignalHandler::restore_signal_handler() {}

} // namespace mrv
