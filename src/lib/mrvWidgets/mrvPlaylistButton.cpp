// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/mrvApp.h"

#include "mrvFLTK/mrvCallbacks.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvEdit/mrvEditCallbacks.h"

#include "mrvWidgets/mrvPlaylistButton.h"
#include "mrvWidgets/mrvFileDragger.h"

#include "mrvOS/mrvString.h"
#include "mrvOS/mrvI8N.h"

#include <tlCore/Vector.h>

#include <opentimelineio/timeline.h>

#include <FL/Fl.H>
#include <FL/Fl_Menu_Button.H>

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

    PlaylistButton::~PlaylistButton()
    {
    }

    void PlaylistButton::setIndex(size_t value)
    {
        _p->index = value;
    }

    void PlaylistButton::createTimeline(
        const file::Path& path,
        const std::shared_ptr<system::Context>& context)
    {
        TLRENDER_P();
        p.timeline = timeline::create(path, context);
        redraw();
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

        if (p.timeline)
        {
            auto stack  = p.timeline->tracks();
            if (stack)
            {
                auto tracks = stack->children();
                
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
            }
        }

        const std::string text = label();
        auto lines = string::split(text, '\n');
        char buf[4096];
        if (lines.size() > 1)
        {
            snprintf(
                buf, 4096, "%s\n%s\nVideo:%u Audio:%u", lines[0].c_str(),
                lines[1].c_str(), videoClips, audioClips);
        }
        else
        {
            snprintf(
                buf, 4096, "Video:%u Audio:%u", videoClips, audioClips);
        }
        copy_label(buf);
    }

    void PlaylistButton::draw()
    {
        _countVideoAndAudioClips();
        ClipButton::draw();
    }

} // namespace mrv
