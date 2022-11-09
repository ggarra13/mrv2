// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once

#include <mrvFl/mrvTimelinePlayer.h>


namespace mrv
{
    class GLViewport;
    class App;

    //! Secondary window.
    class SecondaryWindow
    {

    public:
        SecondaryWindow( App* );
        ~SecondaryWindow();

        //! Get the viewport.
        GLViewport* viewport() const;

    private:
        TLRENDER_PRIVATE();
    };
}
