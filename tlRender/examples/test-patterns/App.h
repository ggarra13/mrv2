// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlBaseApp/BaseApp.h>

namespace tl
{
    namespace gl
    {
        class GLFWWindow;
    }

    namespace examples
    {
        //! Example test patterns application.
        namespace test_patterns
        {
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
                std::shared_ptr<gl::GLFWWindow> _window;
            };
        } // namespace test_patterns
    } // namespace examples
} // namespace tl
