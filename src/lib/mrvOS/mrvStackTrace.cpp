// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#ifdef MRV2_CPPTRACE
#  ifndef __APPLE__

#    include "mrvOS/mrvStackTrace.h"

#    include <cpptrace/cpptrace.hpp>
#    include <csignal>
#    include <iostream>
#    include <thread>
#    include <vector>
#  endif
#endif

void printStackTrace()
{
#ifdef MRV2_CPPTRACE
#    ifndef __APPLE__
         cpptrace::generate_trace().print();
         std::cout << std::flush;
#    endif
#endif
}
