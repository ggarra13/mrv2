// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <cstdint>

namespace mrv
{
    /**
     * Memory querying rouinte on all OSes.
     *
     * @param totalVirtualMem    Total Virtual Memory
     * @param virtualMemUsed     Total Virual Memory Used
     * @param virtualMemUsedByMe Virtual Memory Used by this process
     * @param totalPhysMem       Total Physical Memory
     * @param physMemUsed        Total Physical Memory Used
     * @param physMemUsedByMe    Physical Memory used by thie process
     */
    void memory_information(
        uint64_t& totalVirtualMem, uint64_t& virtualMemUsed,
        uint64_t& virtualMemUsedByMe, uint64_t& totalPhysMem,
        uint64_t& physMemUsed, uint64_t& physMemUsedByMe);
} // namespace mrv
