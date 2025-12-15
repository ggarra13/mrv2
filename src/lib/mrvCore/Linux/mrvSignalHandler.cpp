

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <iostream>
#include <filesystem>
namespace fs = std::filesystem;


#include "mrvCore/mrvLicensing.h"
#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvSignalHandler.h"
#include "mrvCore/mrvStackTrace.h"

namespace mrv
{
    static void signal_callback(int signal)
    {
        printStackTrace();

        release_license();

        exit(signal);
    }

    SignalHandler::SignalHandler()
    {
        install_signal_handler();
    }

    SignalHandler::~SignalHandler()
    {
        restore_signal_handler();
    }

    void SignalHandler::install_signal_handler()
    {
        // Set up signal handlers
        std::signal(SIGSEGV, signal_callback);

        std::signal(SIGABRT, signal_callback);
        std::signal(SIGBUS, signal_callback);
        std::signal(SIGINT, signal_callback);
    }

    void SignalHandler::restore_signal_handler() {}

} // namespace mrv
