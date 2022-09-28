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

    struct FilesModelItem;

    class TimeObject;
    class DevicesModel;
    class FilesModel;
    class FilesAModel;
    class FilesBModel;
    class SettingsObject;

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

        //! Get the time object.
        TimeObject* timeObject() const;

        //! Get the settings object.
        //SettingsObject* settingsObject() const;

        //! Get the files model.
        const std::shared_ptr<FilesModel>& filesModel() const;

        //! Get the LUT options.
        const timeline::LUTOptions& lutOptions() const;

        //! Get the image options.
        const timeline::ImageOptions& imageOptions() const;

        //! Get the display options.
        const timeline::DisplayOptions& displayOptions() const;

        //! Create a new application.
        static std::shared_ptr<App> create(
            int argc,
            char* argv[],
            const std::shared_ptr<system::Context>&);

        //! Run the application.
        int run();

    public:
        //! Open a file.
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

    // // Q_SIGNALS:
    //     //! This signal is emitted when the LUT options are changed.
    //     void lutOptionsChanged(const tl::timeline::LUTOptions&);

    //     //! This signal is emitted when the image options are changed.
    //     void imageOptionsChanged(const tl::timeline::ImageOptions&);

    //     //! This signal is emitted when the display options are changed.
    //     void displayOptionsChanged(const tl::timeline::DisplayOptions&);

    private: //Q_SLOTS:
        void _activeCallback(const std::vector<std::shared_ptr<FilesModelItem> >&);
        void _settingsCallback();
    private:
        otime::RationalTime _cacheReadAhead() const;
        otime::RationalTime _cacheReadBehind() const;

        void _cacheUpdate();

        TLRENDER_PRIVATE();
    };
}
