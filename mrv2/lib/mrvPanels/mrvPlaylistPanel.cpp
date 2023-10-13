// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <string>
#include <vector>
#include <map>

#include <FL/Fl_Box.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_RGB_Image.H>

#include "mrvCore/mrvHome.h"

#include "mrvWidgets/mrvPack.h"
#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvPlaylistButton.h"
#include "mrvWidgets/mrvButton.h"

#include "mrvEdit/mrvEditUtil.h"

#include "mrvPanels/mrvPlaylistPanel.h"
#include "mrvPanels/mrvPanelsAux.h"
#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvFl/mrvIO.h"

#include "mrvEdit/mrvEditCallbacks.h"

#include "mrvUI/mrvAsk.h" // for fl_input

#include "mrvGL/mrvThumbnailCreator.h"

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/mrvPlaylistsModel.h"
#include "mrvApp/App.h"

#include "mrViewer.h"

namespace mrv
{

    namespace panel
    {

        typedef std::map< PlaylistButton*, int64_t > WidgetIds;
        typedef std::map< PlaylistButton*, size_t > WidgetIndices;

        struct PlaylistPanel::Private
        {
            std::weak_ptr<system::Context> context;
            mrv::ThumbnailCreator* thumbnailCreator;

            std::map< size_t, PlaylistButton* > map;
            std::vector< PlaylistButton* > clipButtons;

            WidgetIds ids;
            WidgetIndices indices;

            std::shared_ptr<
                observer::ListObserver<std::shared_ptr<FilesModelItem> > >
                filesObserver;
            std::shared_ptr<
                observer::ListObserver<std::shared_ptr<FilesModelItem> > >
                activeObserver;
            std::shared_ptr<observer::ValueObserver<int> > aIndexObserver;
        };

        struct ThumbnailData
        {
            PlaylistButton* widget;
        };

        void playlistThumbnail_cb(
            const int64_t id,
            const std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> >&
                thumbnails,
            void* opaque)
        {
            ThumbnailData* data = static_cast< ThumbnailData* >(opaque);
            PlaylistButton* w = data->widget;
            if (playlistPanel)
                playlistPanel->playlistThumbnail(id, thumbnails, w);
            delete data;
        }

        void PlaylistPanel::playlistThumbnail(
            const int64_t id,
            const std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> >&
                thumbnails,
            PlaylistButton* w)
        {
            WidgetIds::const_iterator it = _r->ids.find(w);
            if (it == _r->ids.end())
            {
                return;
            }

            if (it->second == id)
            {
                for (const auto& i : thumbnails)
                {
                    Fl_Image* img = w->image();
                    w->image(i.second);
                    delete img;
                    w->redraw();
                }
            }
            else
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

            Fl_SVG_Image* svg = load_svg("Playlist.svg");
            g->image(svg);

            g->callback(
                [](Fl_Widget* w, void* d)
                {
                    ViewerUI* ui = static_cast< ViewerUI* >(d);
                    delete playlistPanel;
                    playlistPanel = nullptr;
                    ui->uiMain->fill_menu(ui->uiMenuBar);
                },
                ui);

            _r->filesObserver = observer::
                ListObserver<std::shared_ptr<FilesModelItem> >::create(
                    ui->app->filesModel()->observeFiles(),
                    [this](const std::vector< std::shared_ptr<FilesModelItem> >&
                               value) { refresh(); });

            _r->activeObserver = observer::
                ListObserver<std::shared_ptr<FilesModelItem> >::create(
                    ui->app->filesModel()->observeActive(),
                    [this](const std::vector< std::shared_ptr<FilesModelItem> >&
                               value) { redraw(); });

            _r->aIndexObserver = observer::ValueObserver<int>::create(
                ui->app->filesModel()->observeAIndex(),
                [this](int value) { redraw(); });
        }

        PlaylistPanel::~PlaylistPanel()
        {
            cancel_thumbnails();
            clear_controls();
        }

        void PlaylistPanel::clear_controls()
        {
            _r->map.clear();
            _r->clipButtons.clear();
            _r->indices.clear();
        }

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

            g->clear();

            g->begin();

            _r->thumbnailCreator = p.ui->uiTimeline->thumbnailCreator();

            int Y = g->y() + 22;

            const auto& model = p.ui->app->filesModel();
            const auto& files = model->observeFiles().get()->get();
            const auto& aIndex = model->observeAIndex()->get();
            const size_t numFiles = files.size();
            const image::Size size(128, 64);
            const std::string tmpdir = tmppath() + '/';

            file::Path lastPath;

            size_t numValidFiles = 0;
            for (size_t i = 0; i < numFiles; ++i)
            {
                const auto& media = files[i];
                const auto& path = media->path;

                const bool isEDL = isTemporaryEDL(path);
                if (!isEDL)
                    continue;

                // When we refresh the .otio for EDL, we get two clips with the
                // same name, we avoid displaying both with this check.
                if (path == lastPath && isEDL)
                    continue;
                lastPath = path;

                const std::string& fullfile = path.get();

                auto cbW = new Widget<PlaylistButton>(
                    g->x(), Y + numValidFiles * 68, g->w(), 68);
                PlaylistButton* b = cbW;
                _r->clipButtons.push_back(b);
                _r->indices[b] = i;
                cbW->callback(
                    [=](auto b)
                    {
                        WidgetIndices::const_iterator it = _r->indices.find(b);
                        if (it == _r->indices.end())
                            return;
                        int index = (*it).second;
                        auto model = _p->ui->app->filesModel();
                        model->setA(index);
                    });

                ++numValidFiles;

                _r->map[i] = b;

                const std::string dir = path.getDirectory();
                const std::string base = path.getBaseName();
                const std::string extension = path.getExtension();
                const std::string file = base + path.getNumber() + extension;

                std::string text = dir + "\n" + file + "\nColor";
                b->copy_label(text.c_str());
                if (i == aIndex)
                    b->value(1);
                else
                    b->value(0);

                if (auto context = _r->context.lock())
                {
                    b->createTimeline(context);

                    ThumbnailData* data = new ThumbnailData;
                    data->widget = b;

                    const auto& timeRange = media->inOutRange;
                    auto time = timeRange.start_time();

                    _r->thumbnailCreator->initThread();
                    try
                    {
                        int64_t id = _r->thumbnailCreator->request(
                            fullfile, time, size, playlistThumbnail_cb,
                            (void*)data);
                        _r->ids[b] = id;
                    }
                    catch (const std::exception&)
                    {
                    }
                }
            }

            if (numValidFiles == 0)
            {
                Fl_Group* bg = new Fl_Group(g->x(), Y, g->w(), 68);
                Fl_Box* box = new Fl_Box(g->x(), Y, g->w() - 5, 68);
                box->box(FL_ENGRAVED_BOX);
                box->copy_label(_("Drop a clip here to create a playlist."));
                box->labelsize(12);
                box->align(FL_ALIGN_INSIDE | FL_ALIGN_CENTER | FL_ALIGN_WRAP);
                bg->end();

                Y += 70;
            }

            Fl_Group* bg = new Fl_Group(g->x(), Y, g->w(), 30);
            bg->begin();

            Button* b;
            auto bW = new Widget< Button >(g->x() + 10, Y, 30, 30);
            b = bW;
            Fl_Image* svg = load_svg("Tracks.svg");
            b->image(svg);
            b->tooltip(
                _("Create an empty timeline with a video and audio track."));
            bW->callback([=](auto w)
                         { create_empty_timeline_cb(nullptr, p.ui); });

            bW = new Widget< Button >(g->x() + 40, Y, 30, 30);
            b = bW;
            svg = load_svg("TracksFromA.svg");
            b->image(svg);
            b->tooltip(_("Create a timeline from the selected clip."));
            bW->callback([=](auto w)
                         { create_new_timeline_cb(nullptr, p.ui); });

            bW = new Widget< Button >(g->x() + 70, Y, 30, 30);
            b = bW;
            svg = load_svg("Save.svg");
            b->image(svg);
            b->tooltip(
                _("Save current EDL to a permanent location, making paths "
                  "relative if possible."));
            bW->callback([=](auto w)
                         { save_timeline_to_disk_cb(nullptr, p.ui); });

            bW = new Widget< Button >(g->x() + 100, Y, 30, 30);
            b = bW;
            svg = load_svg("FileClose.svg");
            b->image(svg);
            b->tooltip(_("Close current EDL."));
            bW->callback(
                [=](auto w)
                {
                    const auto& model = p.ui->app->filesModel();
                    const auto& Aitem = model->observeA()->get();
                    std::string extension = Aitem->path.getExtension();
                    if (extension != ".otio")
                        return;
                    close_current_cb(w, p.ui);
                });

            bg->end();

            Y += 30;
            // Y += 30 + numFiles * 64;
        }

        void PlaylistPanel::redraw()
        {

            TLRENDER_P();

            otio::RationalTime time = otio::RationalTime(0.0, 1.0);

            const auto player = p.ui->uiView->getTimelinePlayer();

            image::Size size(128, 64);

            const auto& model = p.ui->app->filesModel();
            auto Aindex = model->observeAIndex()->get();
            const auto files = model->observeFiles();

            for (auto& m : _r->map)
            {
                size_t i = m.first;
                const auto& media = files->getItem(i);
                const auto& path = media->path;

                const std::string fullfile = path.get();
                PlaylistButton* b = m.second;

                b->labelcolor(FL_WHITE);
                WidgetIndices::iterator it = _r->indices.find(b);
                time = media->currentTime;
                uint16_t layerId = media->videoLayer;
                if (Aindex != i)
                {
                    b->value(0);
                    if (b->image())
                        continue;
                }
                else
                {
                    b->value(1);
                    if (player)
                    {
                        time = player->currentTime();
                        layerId = p.ui->uiColorChannel->value();
                    }
                }

                if (auto context = _r->context.lock())
                {
                    b->createTimeline(context);

                    ThumbnailData* data = new ThumbnailData;
                    data->widget = b;

                    WidgetIds::const_iterator it = _r->ids.find(b);
                    if (it != _r->ids.end())
                    {
                        _r->thumbnailCreator->cancelRequests(it->second);
                        _r->ids.erase(it);
                    }

                    try
                    {
                        file::Path path(fullfile);
                        auto timeline =
                            timeline::Timeline::create(path, context);
                        auto timeRange = timeline->getTimeRange();

                        if (time::isValid(timeRange))
                        {
                            auto startTime = timeRange.start_time();
                            auto endTime = timeRange.end_time_inclusive();

                            if (time < startTime)
                                time = startTime;
                            else if (time > endTime)
                                time = endTime;
                        }

                        _r->thumbnailCreator->initThread();

                        int64_t id = _r->thumbnailCreator->request(
                            fullfile, time, size, playlistThumbnail_cb,
                            (void*)data, layerId);
                        _r->ids[b] = id;
                    }
                    catch (const std::exception& e)
                    {
                    }
                }
            }
        }

        void PlaylistPanel::refresh()
        {
            cancel_thumbnails();
            clear_controls();
            add_controls();
            end_group();
        }

        void PlaylistPanel::add(
            const math::Vector2i& pos, const std::string& filename,
            const size_t index, ViewerUI* ui)
        {
            int aIndex = -1;
            bool validDrop = false;
            math::Vector2i win;
            if (!is_panel())
            {
                auto window = g->get_window();
                win.x = window->x();
                win.y = window->y();
            }
            if (_r->map.empty())
            {
                math::Box2i box(g->x() + win.x, g->y() + win.y, g->w(), 68);
                if (box.contains(pos))
                {
                    validDrop = true;
                }
                if (validDrop)
                {
                    auto model = ui->app->filesModel();
                    model->setA(index);
                    create_new_timeline_cb(nullptr, ui);
                }
            }
            else
            {
                for (auto& m : _r->map)
                {
                    PlaylistButton* b = m.second;
                    math::Box2i box(
                        b->x() + win.x, b->y() + win.y, b->w(), b->h());
                    if (box.contains(pos))
                    {
                        aIndex = static_cast<int>(m.first);
                        validDrop = true;
                        break;
                    }
                }

                if (validDrop && aIndex >= 0)
                {
                    auto model = ui->app->filesModel();
                    model->setA(aIndex);
                    add_clip_to_timeline(filename, index, ui);
                }
            }
        }
    } // namespace panel

} // namespace mrv
