// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl.H>
#include <FL/Fl_Menu_Button.H>

#include <opentimelineio/item.h>
#include <opentimelineio/stack.h>
#include <opentimelineio/timeline.h>
#include <opentimelineio/track.h>

#include <tlCore/Vector.h>

#include "mrvCore/mrvString.h"
#include "mrvCore/mrvI8N.h"

#include "mrvFl/mrvCallbacks.h"

#include "mrvWidgets/mrvPlaylistButton.h"
#include "mrvWidgets/mrvFileDragger.h"

#include "mrvEdit/mrvEditCallbacks.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/App.h"

#include "mrViewer.h"

namespace
{
} // namespace

namespace mrv
{
    struct PlaylistButton::Private
    {
        size_t index = 0;
        otio::SerializableObject::Retainer<otio::Timeline> timeline;
    };

    PlaylistButton::PlaylistButton(int X, int Y, int W, int H, const char* L) :
        _p(new Private),
        ClipButton(X, Y, W, H, L)
    {
    }

    PlaylistButton::~PlaylistButton() {}

    void PlaylistButton::setIndex(size_t value)
    {
        _p->index = value;
    }

    void PlaylistButton::createTimeline(
        const std::shared_ptr<system::Context>& context)
    {
        TLRENDER_P();
        const std::string text = label();
        stringArray lines;
        split_string(lines, text, "\n");
        std::string filename = lines[0] + lines[1];
        p.timeline = timeline::create(filename, context);
    }

    int PlaylistButton::handle(int event)
    {
        TLRENDER_P();

        switch (event)
        {
        case FL_FOCUS:
        case FL_UNFOCUS:
        case FL_ENTER:
        case FL_LEAVE:
            return 1;
        case FL_KEYDOWN:
        case FL_KEYUP:
        {
            if (value())
            {
                unsigned rawkey = Fl::event_key();
                if (rawkey == FL_Delete || rawkey == FL_BackSpace)
                {
                    close_current_cb(this, App::ui);
                    return 1;
                }
            }
            return 0;
        }
        }
        return ClipButton::handle(event);
    }

    void PlaylistButton::_countVideoAndAudioClips()
    {
        TLRENDER_P();
        unsigned videoClips = 0;
        unsigned audioClips = 0;
        auto tracks = p.timeline->tracks()->children();
        for (auto child : tracks)
        {
            auto track = otio::dynamic_retainer_cast<otio::Track>(child);
            if (!track)
                continue;

            auto clips = track->find_children<otio::Clip>();
            if (track->kind() == otio::Track::Kind::video)
            {
                videoClips += clips.size();
            }
            else if (track->kind() == otio::Track::Kind::audio)
            {
                audioClips += clips.size();
            }
        }

        const std::string text = label();
        stringArray lines;
        split_string(lines, text, "\n");
        char buf[4096];
        snprintf(
            buf, 4096, "%s\n%s\nVideo:%u Audio:%u", lines[0].c_str(),
            lines[1].c_str(), videoClips, audioClips);
        copy_label(buf);
    }

    void PlaylistButton::draw()
    {
        _countVideoAndAudioClips();
        ClipButton::draw();
    }

} // namespace mrv
