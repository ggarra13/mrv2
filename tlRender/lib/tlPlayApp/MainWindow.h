// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUIApp/Window.h>

#include <tlTimeline/Player.h>

namespace tl
{
    namespace timelineui
    {
        class TimelineWidget;
    }

    namespace play_app
    {
        class App;
        class Viewport;

        //! Window options.
        struct WindowOptions
        {
            bool fileToolBar = true;
            bool compareToolBar = true;
            bool windowToolBar = true;
            bool viewToolBar = true;
            bool toolsToolBar = true;
            bool timeline = true;
            bool bottomToolBar = true;
            bool statusToolBar = true;
            float splitter = .7F;
            float splitter2 = .8F;

            bool operator==(const WindowOptions&) const;
            bool operator!=(const WindowOptions&) const;
        };

        //! Main window.
        class MainWindow : public ui_app::Window
        {
            TLRENDER_NON_COPYABLE(MainWindow);

        protected:
            void _init(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

            MainWindow();

        public:
            ~MainWindow();

            //! Create a new main window.
            static std::shared_ptr<MainWindow> create(
                const std::shared_ptr<App>&,
                const std::shared_ptr<system::Context>&);

            //! Get the viewport.
            const std::shared_ptr<Viewport>& getViewport() const;

            //! Get the timeline widget.
            const std::shared_ptr<timelineui::TimelineWidget>&
            getTimelineWidget() const;

            //! Focus the current frame widget.
            void focusCurrentFrame();

            //! Get the window options.
            const WindowOptions& getWindowOptions() const;

            //! Observe the window options.
            std::shared_ptr<observer::IValue<WindowOptions> >
            observeWindowOptions() const;

            //! Set the window options.
            void setWindowOptions(const WindowOptions&);

            void setGeometry(const math::Box2i&) override;
            void keyPressEvent(ui::KeyEvent&) override;
            void keyReleaseEvent(ui::KeyEvent&) override;

        protected:
            void _drop(const std::vector<std::string>&) override;

        private:
            void _playerUpdate(const std::shared_ptr<timeline::Player>&);
            void _showSpeedPopup();
            void _showAudioPopup();
            void _windowOptionsUpdate();
            void _statusUpdate(const std::vector<log::Item>&);
            void _infoUpdate();

            TLRENDER_PRIVATE();
        };

        //! \name Serialize
        ///@{

        void to_json(nlohmann::json&, const WindowOptions&);

        void from_json(const nlohmann::json&, WindowOptions&);

        ///@}
    } // namespace play_app
} // namespace tl
