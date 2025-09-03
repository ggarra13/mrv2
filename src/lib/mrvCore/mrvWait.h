// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#pragma once

namespace mrv
{
    namespace wait
    {
        //! This function will wait the amount in milliseconds, while refreshing
        //! the UI.  The difference between this and Fl::wait is that Fl::wait
        //! will exit the loop when it polls an event.
        void milliseconds(float milliseconds);
    }
}
