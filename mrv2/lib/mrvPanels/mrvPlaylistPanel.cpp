// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <string>
#include <vector>
#include <map>

#include <FL/Fl_Box.H>
#include <FL/Fl_Pack.H>

#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvFile.h"

#include "mrvWidgets/mrvPack.h"
#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvPlaylistButton.h"
#include "mrvWidgets/mrvButton.h"

#include "mrvPanels/mrvPlaylistPanel.h"
#include "mrvPanels/mrvPanelsAux.h"
#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvFl/mrvIO.h"

#include "mrvEdit/mrvEditCallbacks.h"

#include "mrvUI/mrvAsk.h" // for fl_input

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/mrvPlaylistsModel.h"
#include "mrvApp/mrvApp.h"

#include "mrViewer.h"

namespace mrv
{

    namespace panel
    {

        typedef std::map< PlaylistButton*, size_t > WidgetIndices;

        struct PlaylistPanel::Private
        {
            std::map< size_t, PlaylistButton* > map;

            WidgetIndices indices;

            std::shared_ptr<
                observer::ListObserver<std::shared_ptr<FilesModelItem> > >
                filesObserver;
            std::shared_ptr<
                observer::ListObserver<std::shared_ptr<FilesModelItem> > >
                activeObserver;
            std::shared_ptr<observer::ValueObserver<int> > aIndexObserver;
        };

        PlaylistPanel::PlaylistPanel(ViewerUI* ui) :
            _r(new Private),
            ThumbnailPanel(ui)
        {
            add_group("Playlist");
            g->image(load_svg("Playlist.svg"));

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
        }

        void PlaylistPanel::add_controls()
        {
            TLRENDER_P();

            _r->map.clear();
            _r->indices.clear();
            
            g->clear();

            g->begin();

            int Y = g->y() + 22;

            otio::RationalTime time = otio::RationalTime(0.0, 1.0);
            const auto player = p.ui->uiView->getTimelinePlayer();

            const auto& model = App::app->filesModel();
            const auto& files = model->observeFiles().get()->get();
            const auto& aIndex = model->observeAIndex()->get();
            const size_t numFiles = files.size();
            const std::string tmpdir = tmppath() + '/';

            file::Path lastPath;
            
            size_t numValidFiles = 0;
            for (size_t i = 0; i < numFiles; ++i)
            {
                const auto& media = files[i];
                const auto& path = media->path;

                const bool isEDL = file::isTemporaryEDL(path);
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
                {
                    b->value(1);
                    if (player)
                    {
                        time = player->currentTime();
                    }
                }
                else
                {
                    b->value(0);
                    time = media->currentTime;
                }
                
                _createThumbnail(b, path, time);
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
            bW->callback([=](auto w) { create_new_timeline_cb(p.ui); });

            bW = new Widget< Button >(g->x() + 40, Y, 30, 30);
            b = bW;
            svg = load_svg("TracksFromA.svg");
            b->image(svg);
            b->tooltip(_("Create a timeline from the selected clip."));
            bW->callback(
                [=](auto w)
                {
                    int index = model->observeAIndex()->get();
                    add_clip_to_new_timeline_cb(index, p.ui);
                });

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
                    const auto& model = App::app->filesModel();
                    const auto& Aitem = model->observeA()->get();
                    std::string extension = Aitem->path.getExtension();
                    if (extension != ".otio")
                        return;
                    close_current_cb(w, p.ui);
                });

            bg->end();

            Y += 30;
        }

        void PlaylistPanel::redraw()
        {

            TLRENDER_P();

            otio::RationalTime time = otio::RationalTime(0.0, 1.0);

            const auto player = p.ui->uiView->getTimelinePlayer();
            const auto& model = App::app->filesModel();
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

                time = media->currentTime;
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
                    }
                }

                b->createTimeline(App::app->getContext());
                
                _createThumbnail(b, path, time);
                
            }
        }

        void PlaylistPanel::refresh()
        {
            _cancelRequests();
            add_controls();
            end_group();
        }

        void PlaylistPanel::add(
            const math::Vector2i& pos, const size_t index, ViewerUI* ui)
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
                    create_new_timeline_cb(ui);
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
                        auto model = ui->app->filesModel();
                        model->setA(aIndex);
                        validDrop = true;
                        break;
                    }
                }
            }

            if (validDrop)
            {
                add_clip_to_timeline_cb(index, ui);
            }
        }
    } // namespace panel

} // namespace mrv
