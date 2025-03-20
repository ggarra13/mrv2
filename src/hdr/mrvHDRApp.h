// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <mrvBaseApp/mrvBaseApp.h>

namespace
{
    const char* const kProgramName = "hdr";
}

class HDRUI;

namespace mrv
{
    using namespace tl;

    //! Application.
    class HDRApp : public app::BaseApp
    {
        TLRENDER_NON_COPYABLE(HDRApp);

    public:
        HDRApp(int argc, char** argv, const std::shared_ptr<system::Context>&);

        ~HDRApp() override;

        //! Create a new application.
        static std::shared_ptr<HDRApp>
        create(int argc, char* argv[], const std::shared_ptr<system::Context>&);

        //! Run the application.
        int run();

    public:
        //! Set the audio volume.
        void setVolume(float);

        //! Set the audio mute.
        void setMute(bool);

        //! Get the audio volume.
        float volume() const;

        //! Get the audio mute.
        bool isMuted() const;

    public:
        static HDRUI* ui;
        static HDRApp* app;

    private:
        void _cleanResources();
        void _volumeChanged(float);
        void _muteChanged(bool);

        TLRENDER_PRIVATE();
    };
} // namespace mrv
