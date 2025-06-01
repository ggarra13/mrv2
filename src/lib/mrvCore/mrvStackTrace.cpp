// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

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
