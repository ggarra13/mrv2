

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <csignal>
#include <iostream>

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

    void callback(int signal)
    {

        std::string lockfile = mrv::lockfile();
        if (fs::exists(lockfile))
        {
            if (!fs::remove(lockfile))
                std::cerr << "Could not remove lockfile " << lockfile
                          << std::endl;
        }

        std::cerr << "GOT SIGNAL " << signal << std::endl;

        printStackTrace();

        exit(1);
    }

    void SignalHandler::install_signal_handler()
    {
        // Set up signal handlers
        std::signal(SIGSEGV, callback);
        std::signal(SIGABRT, callback);

        std::signal(SIGBUS, callback);
#ifndef NDEBUG
        std::signal(SIGINT, callback);
#endif
    }

    void SignalHandler::restore_signal_handler() {}

} // namespace mrv
