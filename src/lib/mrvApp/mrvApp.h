// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <mrvBaseApp/mrvBaseApp.h>

#include <tlTimeline/PlayerOptions.h>
#include <tlTimeline/IRender.h>
#include <tlTimeline/TimeUnits.h>

#include <tlIO/IO.h>

#include "mrvApp/mrvFilesModel.h"

namespace
{
    const char* const kProgramName = "mrv2";
}

class ViewerUI;

namespace tl
{
    namespace device
    {
        class IOutput;
        class DevicesModel;
        class DeviceConfig;
    } // namespace device
} // namespace tl

namespace mrv
{
    using namespace tl;

    struct Playlist;

    class TimeObject;
    class FilesModel;
    class PlaylistsModel;
    class SettingsObject;

    //! Application.
    class App : public app::BaseApp
    {
        TLRENDER_NON_COPYABLE(App);

    public:
        App(int argc, char** argv, const std::shared_ptr<system::Context>&);

        ~App() override;

        //! Get the time units model.
        const std::shared_ptr<timeline::TimeUnitsModel>& timeUnitsModel() const;

        //! Get the time object.
        TimeObject* timeObject() const;

        //! Get the settings object.
        SettingsObject* settings() const;

        //! Get the files model.
        const std::shared_ptr<FilesModel>& filesModel() const;

        //! Get the playlists model.
        const std::shared_ptr<PlaylistsModel>& playlistsModel() const;

        //! Get the LUT options.
        const timeline::LUTOptions& lutOptions() const;

        //! Get the image options.
        const timeline::ImageOptions& imageOptions() const;

        //! Get the display options.
        const timeline::DisplayOptions& displayOptions() const;

        //! Get the output device.
        const std::shared_ptr<device::IOutput>& outputDevice() const;

        //! Get the devices model.
        const std::shared_ptr<device::DevicesModel>& devicesModel() const;

#ifdef TLRENDER_NDI
        void beginNDIOutputStream();
        void endNDIOutputStream();
#endif

#ifdef TLRENDER_BMD
        void beginBMDOutputStream();
        void endBMDOutputStream();
#endif

        //! Create a new application.
        static std::shared_ptr<App>
        create(int argc, char* argv[], const std::shared_ptr<system::Context>&);

        //! Run the application.
        int run();

        //! Whether FLTK is running.
        bool isRunning() const;

    public:
#ifdef MRV2_PYBIND11
        const std::vector<std::string>& getPythonArgs() const;
#endif

        //! Open a file (with optional audio) or directory.
        void open(const std::string&, const std::string& = std::string());

        //! Open a file dialog.
        void openDialog();

        //! Open a file with separate audio dialog.
        void openSeparateAudioDialog();

        //! Set the LUT options.
        void setLUTOptions(const tl::timeline::LUTOptions&);

        //! Set the image options.
        void setImageOptions(const tl::timeline::ImageOptions&);

        //! Set the display options.
        void setDisplayOptions(const tl::timeline::DisplayOptions&);

        //! Set all the playlists
        void setPlaylists(const std::vector< Playlist& >);

        //! Set a playlists
        void setPlaylist(const int, const std::vector< Playlist& >);

        //! Set the audio volume.
        void setVolume(float);

        //! Set the audio mute.
        void setMute(bool);

        //! Get the audio volume.
        float volume() const;

        //! Get the audio mute.
        bool isMuted() const;

        //! This signal is emitted when the LUT options are changed.
        void lutOptionsChanged(const tl::timeline::LUTOptions&);

        //! This signal is emitted when the image options are changed.
        void imageOptionsChanged(const tl::timeline::ImageOptions&);

        //! This signal is emitted when the display options are changed.
        void displayOptionsChanged(const tl::timeline::DisplayOptions&);

        //! This signal is emitted when the audio volume is changed.
        void volumeChanged(float);

        //! This signal is emitted when the audio mute is changed.
        void muteChanged(bool);

        //! Create a ComfyUI listener (opens a socket on localport).
        void createComfyUIListener();

        //! Create an image listener (opens a socket on localport).
        void createListener();

        //! Removes an image listener (closes a socket on localport).
        void removeListener();

        //! Tries to clean as many resources as possible.  This is usually done
        //! before calling execv(), for example.
        void cleanResources();

        //! Start playback FLTK callback.
        void startPlayback();

        //! Update the cache (Darby makes this private and uses an observer
        //! with a string, but I think it is simpler to make it public).
        void cacheUpdate();
        void timerUpdate();

    public:
        static ViewerUI* ui;
        static App* app;
        static bool demo_mode;

    private:
        void _settingsCallback();

        io::Options _getIOOptions() const;

        otime::RationalTime _cacheReadAhead() const;
        otime::RationalTime _cacheReadBehind() const;

        void _filesUpdate(const std::vector<std::shared_ptr<FilesModelItem> >&);

        void
        _activeUpdate(const std::vector<std::shared_ptr<FilesModelItem> >&);

        void _layersUpdate(const std::vector<int>& value);

        void _audioUpdate();

        static void _timer_update_cb(App*);

        void _startOutputDeviceTimer();
        void _stopOutputDeviceTimer();

        std::shared_ptr<timeline::Timeline>
        _createTimeline(const std::shared_ptr<FilesModelItem>& item);

        void _playerOptions(
            timeline::PlayerOptions& playerOptions,
            const std::shared_ptr<FilesModelItem>& item);

        TLRENDER_PRIVATE();
    };
} // namespace mrv
