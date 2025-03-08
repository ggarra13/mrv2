#ifndef __APPLE__
#    include <cpptrace/cpptrace.hpp>
#    include <csignal>
#    include <iostream>
#    include <thread>
#    include <vector>
#    include "mrvCore/mrvStackTrace.h"
#endif

void printStackTrace()
{
#ifndef __APPLE__
    cpptrace::generate_trace().print();
    std::cout << std::flush;
#endif
}
