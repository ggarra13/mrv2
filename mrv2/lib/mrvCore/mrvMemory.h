// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <cstdint>

namespace mrv
{
    void memory_information(
        uint64_t& totalVirtualMem, uint64_t& virtualMemUsed,
        uint64_t& virtualMemUsedByMe, uint64_t& totalPhysMem,
        uint64_t& physMemUsed, uint64_t& physMemUsedByMe);
}
