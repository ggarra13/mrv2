// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <string>
#include <vector>
#include <map>

#include <FL/Fl_Button.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_RGB_Image.H>

#include "mrvCore/mrvHome.h"

#include "mrvWidgets/mrvPack.h"
#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvFileButton.h"
#include "mrvWidgets/mrvButton.h"

#include "mrvPanels/mrvPlaylistPanel.h"
#include "mrvPanels/mrvPanelsAux.h"
#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvUI/mrvAsk.h" // for fl_input

#include "mrvGL/mrvThumbnailCreator.h"

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/mrvPlaylistsModel.h"
#include "mrvApp/App.h"

#include "mrViewer.h"

namespace mrv
{

    typedef std::map< ClipButton*, int64_t > WidgetIds;
    typedef std::map< ClipButton*, size_t > WidgetIndices;

    struct PlaylistPanel::Private
    {
        std::weak_ptr<system::Context> context;
        mrv::ThumbnailCreator* thumbnailCreator;

        std::map< size_t, ClipButton* > map;
        std::vector< ClipButton* > clipButtons;

        WidgetIds ids;
        WidgetIndices indices;

        std::shared_ptr<
            observer::ListObserver<std::shared_ptr<FilesModelItem> > >
            filesObserver;
        std::shared_ptr<observer::ValueObserver<int> > aIndexObserver;
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
        ClipButton* w = data->widget;
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

        // Fl_SVG_Image* svg = load_svg( "Playlist.svg" );
        // g->image( svg );

        g->callback(
            [](Fl_Widget* w, void* d)
            {
                ViewerUI* ui = static_cast< ViewerUI* >(d);
                delete playlistPanel;
                playlistPanel = nullptr;
                ui->uiMain->fill_menu(ui->uiMenuBar);
            },
            ui);

        _r->filesObserver =
            observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                ui->app->filesModel()->observeFiles(),
                [this](
                    const std::vector< std::shared_ptr<FilesModelItem> >& value)
                { refresh(); });

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

        const auto& model = p.ui->app->filesModel();
        const auto& files = model->observeFiles().get()->get();
        const auto& aIndex = model->observeAIndex()->get();
        const size_t numFiles = files.size();
        const image::Size size(128, 64);

        for (size_t i = 0; i < numFiles; ++i)
        {
            const auto& media = files[i];
            const auto& path = media->path;
            const std::string& dir = path.getDirectory();
            const std::string& base = path.getBaseName();
            const std::string& extension = path.getExtension();
            if (extension != ".otio" || base != "EDL." ||
                (dir != tmppath() + '/' && dir != tmppath() + '\\'))
                continue;

            const std::string& fullfile = path.get();

            auto cbW = new Widget<ClipButton>(
                g->x(), g->y() + 20 + i * 68, g->w(), 68);
            ClipButton* b = cbW;
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

            _r->map[i] = b;

            const std::string file =
                path.getBaseName() + path.getNumber() + path.getExtension();

            const std::string& layer = getLayerName(0, p.ui);
            std::string text = dir + "\n" + file + layer;
            b->copy_label(text.c_str());
            if (i == aIndex)
                b->value(1);
            else
                b->value(0);

            if (auto context = _r->context.lock())
            {
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
            ClipButton* b = m.second;

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
                    auto timeline =
                        timeline::Timeline::create(fullfile, context);
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
                        fullfile, time, size, playlistThumbnail_cb, (void*)data,
                        layerId);
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

} // namespace mrv
