// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <string>
#include <vector>
#include <map>

#include <FL/Fl_Button.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_RGB_Image.H>

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvClipButton.h"
#include "mrvWidgets/mrvButton.h"

#include "mrvPanels/mrvPlaylistPanel.h"
#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvGL/mrvThumbnailCreator.h"

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/App.h"

#include "mrViewer.h"

namespace mrv
{

    typedef std::map< ClipButton*, int64_t > WidgetIds;

    struct PlaylistPanel::Private
    {
        std::weak_ptr<system::Context> context;
        mrv::ThumbnailCreator* thumbnailCreator;

        std::vector< ClipButton* > clipButtons;
        std::vector< std::shared_ptr<FilesModelItem> > clips;

        WidgetIds ids;
        std::vector< Fl_Button* > buttons;
    };

    struct ThumbnailData
    {
        ClipButton* widget;
    };

    void playlistThumbnail_cb(
        const int64_t id,
        const std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> >&
            thumbnails,
        void* opaque)
    {
        ThumbnailData* data = static_cast< ThumbnailData* >(opaque);
        ClipButton* w       = data->widget;
        if (playlistPanel)
            playlistPanel->playlistThumbnail(id, thumbnails, w);
        delete data;
    }

    void PlaylistPanel::playlistThumbnail(
        const int64_t id,
        const std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> >&
            thumbnails,
        ClipButton* w)
    {
        WidgetIds::const_iterator it = _r->ids.find(w);
        if (it == _r->ids.end())
            return;

        if (it->second == id)
        {
            for (const auto& i : thumbnails)
            {
                Fl_Image* img = w->image();
                w->image(i.second);
                delete img;
                w->redraw();
            }
        } else
        {
            for (const auto& i : thumbnails)
            {
                delete i.second;
            }
        }
    }

    PlaylistPanel::PlaylistPanel(ViewerUI* ui) :
        _r(new Private),
        PanelWidget(ui)
    {
        _r->context = ui->app->getContext();

        add_group("Playlist");

        // Fl_SVG_Image* svg = load_svg( "Playlist.svg" );
        // g->image( svg );

        g->callback(
            [](Fl_Widget* w, void* d) {
                ViewerUI* ui = static_cast< ViewerUI* >(d);
                delete playlistPanel;
                playlistPanel = nullptr;
                ui->uiMain->fill_menu(ui->uiMenuBar);
            },
            ui);
    }

    PlaylistPanel::~PlaylistPanel()
    {
        cancel_thumbnails();
        clear_controls();
    }

    void PlaylistPanel::clear_controls() { _r->clipButtons.clear(); }

    void PlaylistPanel::cancel_thumbnails()
    {
        for (const auto& it : _r->ids)
        {
            _r->thumbnailCreator->cancelRequests(it.second);
        }

        _r->ids.clear();
    }

    void PlaylistPanel::add_controls()
    {
        TLRENDER_P();

        _r->thumbnailCreator =
            p.ui->uiTimeWindow->uiTimeline->thumbnailCreator();

        g->clear();
        g->begin();

        // Make sure all clips exist in file liest
        const auto& model = p.ui->app->filesModel();
        const auto& files = model->observeFiles().get()->get();

        std::vector< std::shared_ptr<FilesModelItem> > newclips;
        for (size_t i = 0; i < _r->clips.size(); ++i)
        {
            if (std::find(files.begin(), files.end(), _r->clips[i])
                != files.end())
                newclips.push_back(_r->clips[i]);
        }
        _r->clips = newclips;

        size_t numFiles = _r->clips.size();

        imaging::Size size(128, 64);

        for (size_t i = 0; i < numFiles; ++i)
        {
            const auto& media = _r->clips[i];
            const auto& path  = media->path;

            const std::string& dir = path.getDirectory();
            const std::string file =
                path.getBaseName() + path.getNumber() + path.getExtension();
            const std::string& fullfile = path.get();

            auto bW = new Widget<ClipButton>(
                g->x(), g->y() + 20 + i * 68, g->w(), 68);
            ClipButton* b = bW;
            _r->clipButtons.push_back(b);
            bW->callback([=](auto b) { b->value(!b->value()); });

            std::string text = dir + "\n" + file;
            b->copy_label(text.c_str());

            if (auto context = _r->context.lock())
            {
                ThumbnailData* data = new ThumbnailData;
                data->widget        = b;

                WidgetIds::const_iterator it = _r->ids.find(b);
                if (it != _r->ids.end())
                {
                    _r->thumbnailCreator->cancelRequests(it->second);
                    _r->ids.erase(it);
                };
                const auto& timeRange = media->inOutRange;
                const auto& time      = timeRange.start_time();

                _r->thumbnailCreator->initThread();
                try
                {
                    int64_t id = _r->thumbnailCreator->request(
                        fullfile, time, size, playlistThumbnail_cb,
                        (void*)data);
                    _r->ids[b] = id;
                } catch (const std::exception&)
                {}
            }
        }

        int Y = g->y() + 20 + numFiles * 64;

        Fl_Pack* bg = new Fl_Pack(g->x(), Y, g->w(), 30);

        bg->type(Fl_Pack::HORIZONTAL);
        bg->begin();

        auto bW   = new Widget< Button >(g->x() + 150, Y, 50, 30, "Add");
        Button* b = bW;
        b->tooltip(_("Add current file to Playlist"));
        bW->callback([=](auto w) {
            const auto& model = p.ui->app->filesModel();
            const auto& files = model->observeFiles();
            if (files->getSize() == 0)
                return;
            const auto Aindex  = model->observeAIndex()->get();
            const auto& item   = files->getItem(Aindex);
            auto clip          = std::make_shared<FilesModelItem>();
            clip               = item;
            const auto& player = p.ui->uiView->getTimelinePlayer();
            clip->inOutRange   = player->inOutRange();
            _r->clips.push_back(clip);
            refresh();
        });

        bW = new Widget< Button >(g->x() + 150, Y, 70, 30, "Remove");
        b  = bW;
        b->tooltip(_("Remove selected files from Playlist"));
        bW->callback([=](auto w) {
            // Create a new list of new clips not taking into account
            // those selected (to remove)
            std::vector< std::shared_ptr<FilesModelItem> > newclips;
            for (size_t i = 0; i < _r->clips.size(); ++i)
            {
                if (!_r->clipButtons[i]->value())
                    newclips.push_back(_r->clips[i]);
            }
            _r->clips = newclips;
            refresh();
        });

        bW = new Widget< Button >(g->x() + 150, Y, 70, 30, "Playlist");
        b  = bW;
        b->tooltip(_("Create .otio Playlist"));
        bW->callback([=](auto w) {
            if (_r->clips.size() < 2)
                return;
            create_playlist(p.ui, _r->clips);
            _r->clips.clear();
            refresh();
        });

        bg->end();

        g->end();
    }

    void PlaylistPanel::refresh()
    {
        cancel_thumbnails();
        clear_controls();
        add_controls();
        end_group();
    }

} // namespace mrv
