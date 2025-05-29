// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlBaseApp/BaseApp.h>

#include <tlUI/Event.h>

#include <tlCore/Size.h>

namespace tl
{
    namespace ui
    {
        class Style;
    }

    //! User interface application
    namespace ui_app
    {
        class Window;

        //! Application options.
        struct Options
        {
            math::Size2i windowSize = math::Size2i(1920, 1080);
            bool fullscreen = false;
        };

        //! Base class for user interface applications.
        class App : public app::BaseApp
        {
            TLRENDER_NON_COPYABLE(App);

        protected:
            void _init(
                const std::vector<std::string>&,
                const std::shared_ptr<system::Context>&,
                const std::string& cmdLineName,
                const std::string& cmdLineSummary,
                const std::vector<std::shared_ptr<app::ICmdLineArg> >& = {},
                const std::vector<std::shared_ptr<app::ICmdLineOption> >& = {});

            App();

        public:
            ~App();

            //! Run the application.
            virtual int run();

            //! Exit the application.
            void exit(int = 0);

            //! Get the style.
            const std::shared_ptr<ui::Style> getStyle() const;

            //! Get the number of screens.
            int getScreenCount() const;

            //! Add a window.
            void addWindow(const std::shared_ptr<Window>&);

            //! Remove a window.
            void removeWindow(const std::shared_ptr<Window>&);

        protected:
            virtual void _tick();

            void _tickRecursive(
                const std::shared_ptr<ui::IWidget>&, bool visible, bool enabled,
                const ui::TickEvent&);

            Options _uiOptions;

        private:
            void _removeWindow(const std::shared_ptr<Window>&);

            TLRENDER_PRIVATE();
        };
    } // namespace ui_app
} // namespace tl
