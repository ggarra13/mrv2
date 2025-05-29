// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlBaseApp/BaseApp.h>

#include <tlGL/OffscreenBuffer.h>

#include <tlTimeline/IRender.h>
#include <tlTimeline/Timeline.h>

#include <tlIO/IO.h>

struct GLFWwindow;

namespace tl
{
    //! tlresource application
    namespace resource
    {
        //! Application options.
        struct Options
        {
        };

        //! Application.
        class App : public app::BaseApp
        {
            TLRENDER_NON_COPYABLE(App);

        protected:
            void _init(
                const std::vector<std::string>&,
                const std::shared_ptr<system::Context>&);

            App();

        public:
            ~App();

            //! Create a new application.
            static std::shared_ptr<App> create(
                const std::vector<std::string>&,
                const std::shared_ptr<system::Context>&);

            //! Run the application.
            int run();

        private:
            std::string _input;
            std::string _output;
            std::string _varName;
            Options _options;

            std::chrono::steady_clock::time_point _startTime;
        };
    } // namespace resource
} // namespace tl
