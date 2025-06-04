// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#pragma once

#include <tlUIApp/App.h>

#include <tlTimeline/Player.h>

namespace tl
{
#if defined(TLRENDER_BMD)
    namespace bmd
    {
        class DevicesModel;
        class OutputDevice;
    } // namespace bmd
#endif // TLRENDER_BMD

    namespace ui
    {
        class RecentFilesModel;
    }

    namespace play
    {
        struct FilesModelItem;

        class AudioModel;
        class ColorModel;
        class FilesModel;
        class RenderModel;
        class Settings;
        class ViewportModel;
    } // namespace play

    //! tlplay application
    namespace play_app
    {
        class MainWindow;
        class ToolsModel;

        //! Application.
        class App : public ui_app::App
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

            //! Open a file.
            void open(
                const file::Path& path,
                const file::Path& audioPath = file::Path());

            //! Open a file dialog.
            void openDialog();

            //! Open a file and separate audio dialog.
            void openSeparateAudioDialog();

            //! Get the settings.
            const std::shared_ptr<play::Settings>& getSettings() const;

            //! Get the files model.
            const std::shared_ptr<play::FilesModel>& getFilesModel() const;

            //! Observe the timeline player.
            std::shared_ptr<
                observer::IValue<std::shared_ptr<timeline::Player> > >
            observePlayer() const;

            //! Get the color model.
            const std::shared_ptr<play::ColorModel>& getColorModel() const;

            //! Get the viewport model.
            const std::shared_ptr<play::ViewportModel>&
            getViewportModel() const;

            //! Get the render model.
            const std::shared_ptr<play::RenderModel>& getRenderModel() const;

            //! Get the audio model.
            const std::shared_ptr<play::AudioModel>& getAudioModel() const;

            //! Get the tools model.
            const std::shared_ptr<ToolsModel>& getToolsModel() const;

            //! Get the main window.
            const std::shared_ptr<MainWindow>& getMainWindow() const;

            //! Observe whether the secondary window is active.
            std::shared_ptr<observer::IValue<bool> >
            observeSecondaryWindow() const;

            //! Set whether the secondary window is active.
            void setSecondaryWindow(bool);

#if defined(TLRENDER_BMD)
            //! Get the BMD devices model.
            const std::shared_ptr<bmd::DevicesModel>&
            getBMDDevicesModel() const;

            //! Get the BMD output device.
            const std::shared_ptr<bmd::OutputDevice>&
            getBMDOutputDevice() const;
#endif // TLRENDER_BMD

        protected:
            void _tick() override;

        private:
            void _fileLogInit(const std::string&);
            void _settingsInit(const std::string&);
            void _modelsInit();
            void _devicesInit();
            void _observersInit();
            void _inputFilesInit();
            void _windowsInit();

            io::Options _getIOOptions() const;

            void _settingsUpdate(const std::string&);
            void _filesUpdate(
                const std::vector<std::shared_ptr<play::FilesModelItem> >&);
            void _activeUpdate(
                const std::vector<std::shared_ptr<play::FilesModelItem> >&);
            void _layersUpdate(const std::vector<int>&);
            void _cacheUpdate();
            void
            _viewUpdate(const math::Vector2i& pos, double zoom, bool frame);
            void _audioUpdate();

            TLRENDER_PRIVATE();
        };
    } // namespace play_app
} // namespace tl
