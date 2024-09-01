// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#pragma once

#include <string>
#include <vector>

#include <FL/Fl_Button.H>

namespace mrv
{

    class AudioButton : public Fl_Button
    {
    public:
        AudioButton(int, int, int, int, const char* = 0);
        virtual ~AudioButton() {};
        virtual int handle(int e) override;

        int current_track() const;
        void current_track(int idx);

        void clear_tracks();
        void add_track(const std::string& name);

    protected:
        int currentTrack = -1;
        std::vector<std::string> audioTracks;
    };

} // namespace mrv
