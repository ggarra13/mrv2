// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl_Group.H>
#include <FL/Fl_Menu_Button.H>

#include "mrvCore/mrvI8N.h"

#include "mrvWidgets/mrvAudioButton.h"

#include "mrvFl/mrvCallbacks.h"

#include "mrvApp/mrvApp.h"

namespace mrv
{

    static void change_audio_track_cb(Fl_Menu_Button* o, void* d)
    {
        auto audioButton = static_cast<AudioButton*>(d);
        int idx = o->value();

        audioButton->current_track(idx);

        refresh_media_cb(o, d);
    }

    AudioButton::AudioButton(int X, int Y, int W, int H, const char* l) :
        Fl_Button(X, Y, W, H, l)
    {
    }

    int AudioButton::current_track() const
    {
        return currentTrack;
    }

    void AudioButton::current_track(int idx)
    {
        currentTrack = idx;
    }

    int AudioButton::handle(int e)
    {
        switch (e)
        {
        case FL_RELEASE:
        case FL_DRAG:
        case FL_PUSH:
            if (Fl::event_button3())
            {
                if (audioTracks.size() > 1)
                {
                    Fl_Group::current(0);

                    Fl_Menu_Button menu(0, 0, 0, 0);
                    menu.type(Fl_Menu_Button::POPUP3);
                    menu.clear();

                    unsigned count = 0;
                    for (const auto& track : audioTracks)
                    {
                        
                        int idx = menu.add(
                            track.c_str(), 0,
                            (Fl_Callback*)change_audio_track_cb, this,
                            FL_MENU_RADIO);
                        Fl_Menu_Item* item = (Fl_Menu_Item*)&(menu.menu()[idx]);
                        if (count == currentTrack)
                            item->set();
                        ++count;
                        
                    }
                    menu.menu_end();

                    menu.popup();
                }
                return 1;
            }
            break;
        }
        return Fl_Button::handle(e);
    }

    void AudioButton::clear_tracks()
    {
        currentTrack = -1;
        audioTracks.clear();
    }

    void AudioButton::add_track(const std::string& name)
    {
        char buf[256];
        unsigned idx = audioTracks.size() + 1;
        snprintf(buf, 256, _("Track #%d - %s"), idx, name.c_str());
        audioTracks.push_back(buf);
    }

} // namespace mrv
