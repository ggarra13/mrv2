// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2022 Darby Johnston
// All rights reserved.

#pragma once
#include "Util.h"

#include <tlApp/IApp.h>

#include <tlTimeline/IRender.h>
#include <tlTimeline/TimelinePlayer.h>

#include <tlCore/Util.h>
#include <tlCore/FontSystem.h>

namespace {
    const char* const kProgramName = "mrv2";
}


namespace mrv
{
    using namespace tl;

    //! Application.
    class App : public app::IApp
    {
        TLRENDER_NON_COPYABLE(App);

    protected:
        App();

    public:
        void _init(
            int argc,
            char* argv[],
            const std::shared_ptr<system::Context>&);
        ~App();

        //! Create a new application.
        static std::shared_ptr<App> create(
            int argc,
            char* argv[],
            const std::shared_ptr<system::Context>&);

        //! Run the application.
        int run();


    private:

        TLRENDER_PRIVATE();
    };
}
