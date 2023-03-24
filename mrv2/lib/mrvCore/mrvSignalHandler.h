// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

namespace mrv
{

    /** 
     * Class used to install a signal handler to catch crashes, segmentation
     * faults, etc.
     * 
     */
    class SignalHandler
    {
    public:
        SignalHandler();
        ~SignalHandler();

    private:
        void install_signal_handler();
        void restore_signal_handler();
    };

} // namespace mrv
