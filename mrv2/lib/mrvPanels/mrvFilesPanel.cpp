// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <string>
#include <vector>
#include <map>

#include "mrvCore/mrvHome.h"

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvPack.h"
#include "mrvWidgets/mrvFileButton.h"
#include "mrvWidgets/mrvButton.h"

#include "mrvEdit/mrvEditUtil.h"

#include "mrvPanels/mrvPanelsAux.h"
#include "mrvPanels/mrvPanelsCallbacks.h"
#include "mrvPanels/mrvFilesPanel.h"

#include "mrvGL/mrvThumbnailCreator.h"

#include "mrvNetwork/mrvTCP.h"

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/App.h"

#include "mrViewer.h"

#include "mrvFl/mrvIO.h"

namespace
{
    const char* kModule = "files";
}

namespace mrv
{
    namespace panel
    {
        typedef std::map< FileButton*, int64_t > WidgetIds;
        typedef std::map< FileButton*, size_t > WidgetIndices;

        struct FilesPanel::Private
        {
            std::weak_ptr<system::Context> context;
            mrv::ThumbnailCreator* thumbnailCreator;
            App* app;
            std::map< size_t, FileButton* > map;
            WidgetIds ids;
            WidgetIndices indices;
            std::vector< Fl_Button* > buttons;

            std::shared_ptr<
                observer::ListObserver<std::shared_ptr<FilesModelItem> > >
                filesObserver;

            std::shared_ptr< observer::ValueObserver<FilesPanelOptions> >
                filesPanelOptionsObserver;

            std::shared_ptr<observer::ValueObserver<int> > aIndexObserver;
            std::shared_ptr<observer::ListObserver<int> > layerObserver;
        };

        struct ThumbnailData
        {
            FileButton* widget;
        };

        void filesThumbnail_cb(
            const int64_t id,
            const std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> >&
                thumbnails,
            void* opaque)
        {
            ThumbnailData* data = static_cast< ThumbnailData* >(opaque);
            FileButton* w = data->widget;
            if (filesPanel)
                filesPanel->filesThumbnail(id, thumbnails, w);
            delete data;
        }

        void FilesPanel::filesThumbnail(
            const int64_t id,
            const std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> >&
                thumbnails,
            FileButton* w)
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
            }
            else
            {

                for (const auto& i : thumbnails)
                {

                    delete i.second;
                }
            }
        }

        FilesPanel::FilesPanel(ViewerUI* ui) :
            _r(new Private),
            PanelWidget(ui)
        {
            _r->context = ui->app->getContext();

            add_group("Files");

            Fl_SVG_Image* svg = load_svg("Files.svg");
            g->image(svg);

            g->callback(
                [](Fl_Widget* w, void* d)
                {
                    ViewerUI* ui = static_cast< ViewerUI* >(d);
                    delete filesPanel;
                    filesPanel = nullptr;
                    ui->uiMain->fill_menu(ui->uiMenuBar);
                },
                ui);

            _r->filesObserver = observer::
                ListObserver<std::shared_ptr<FilesModelItem> >::create(
                    ui->app->filesModel()->observeFiles(),
                    [this](const std::vector< std::shared_ptr<FilesModelItem> >&
                               value) { refresh(); },
                    observer::CallbackAction::Suppress);

            _r->filesPanelOptionsObserver =
                observer::ValueObserver<FilesPanelOptions>::create(
                    ui->app->filesModel()->observeFilesPanelOptions(),
                    [this](const FilesPanelOptions& value) { refresh(); },
                    observer::CallbackAction::Suppress);

            _r->aIndexObserver = observer::ValueObserver<int>::create(
                ui->app->filesModel()->observeAIndex(),
                [this](int value) { redraw(); },
                observer::CallbackAction::Suppress);

            _r->layerObserver = observer::ListObserver<int>::create(
                ui->app->filesModel()->observeLayers(),
                [this](const std::vector<int>& value) { redraw(); },
                observer::CallbackAction::Suppress);
        }

        FilesPanel::~FilesPanel()
        {
            cancel_thumbnails();
            clear_controls();
        }

        void FilesPanel::cancel_thumbnails()
        {
            for (const auto& it : _r->ids)
            {
                _r->thumbnailCreator->cancelRequests(it.second);
            }

            _r->ids.clear();
        }

        void FilesPanel::clear_controls()
        {
            for (const auto& i : _r->map)
            {
                Fl_Button* b = i.second;

                delete b->image();
                b->image(nullptr);
                g->remove(b);
                delete b;
            }

            // Clear buttons' SVG images
            for (const auto& b : _r->buttons)
            {
                delete b->image();
                b->image(nullptr);
            }

            _r->buttons.clear();
            _r->map.clear();
            _r->indices.clear();
        }

        void FilesPanel::add_controls()
        {
            TLRENDER_P();

            Fl_SVG_Image* svg;
            _r->thumbnailCreator = p.ui->uiTimeline->thumbnailCreator();
            if (!_r->thumbnailCreator)
                return;

            g->clear();

            g->begin();

            const auto model = App::app->filesModel();

            const auto files = model->observeFiles();

            const auto o = model->observeFilesPanelOptions()->get();

            size_t numFiles = files->getSize();

            auto Aindex = model->observeAIndex()->get();

            const auto player = p.ui->uiView->getTimelinePlayer();

            otio::RationalTime time = otio::RationalTime(0.0, 1.0);
            if (player)
                time = player->currentTime();

            image::Size size(128, 64);

            file::Path lastPath;

            for (size_t i = 0; i < numFiles; ++i)
            {
                const auto& media = files->getItem(i);
                const auto& path = media->path;

                const bool isEDL = isTemporaryEDL(path);

                // When we refresh the .otio for EDL, we get two clips with the
                // same name, we avoid displaying both with this check.
                if (path == lastPath && isEDL)
                    continue;
                lastPath = path;

                // We skip EDLs created in tmp dir here.
                if (o.filterEDL && isEDL)
                    continue;

                const std::string protocol = path.getProtocol();
                const std::string dir = path.getDirectory();
                const std::string base = path.getBaseName();
                const std::string extension = path.getExtension();
                const std::string file = base + path.getNumber() + extension;
                const std::string fullfile = protocol + dir + file;

                auto bW = new Widget<FileButton>(
                    g->x(), g->y() + 22 + i * 68, g->w(), 68);
                FileButton* b = bW;
                b->setIndex(i);
                _r->indices[b] = i;
                b->tooltip(_("Select main A image."));
                bW->callback(
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

                time = media->currentTime;
                uint16_t layerId = media->videoLayer;
                if (Aindex == i)
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
                }

                const std::string layer = getLayerName(media, layerId);
                std::string text = protocol + dir + "\n" + file + layer;
                b->copy_label(text.c_str());

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
                            fullfile, time, size, filesThumbnail_cb,
                            (void*)data, layerId);
                        _r->ids[b] = id;
                    }
                    catch (const std::exception& e)
                    {
                    }
                }
            }

            int Y = g->y() + 20 + numFiles * 64;

            Pack* bg = new Pack(g->x(), Y, g->w(), 30);
            bg->type(Pack::HORIZONTAL);
            bg->begin();

            Fl_Button* b;
            auto bW = new Widget< Button >(g->x(), Y, 30, 30);
            b = bW;

            svg = load_svg("FileOpen.svg");
            b->image(svg);

            _r->buttons.push_back(b);

            b->tooltip(_("Open a filename"));
            bW->callback([=](auto w) { open_cb(w, p.ui); });

            bW = new Widget< Button >(g->x() + 30, Y, 30, 30);
            b = bW;
            svg = load_svg("FileOpenSeparateAudio.svg");
            b->image(svg);
            _r->buttons.push_back(b);
            b->tooltip(_("Open a filename with audio"));
            bW->callback([=](auto w) { open_separate_audio_cb(w, p.ui); });

            bW = new Widget< Button >(g->x() + 60, Y, 30, 30);
            b = bW;
            svg = load_svg("FileClose.svg");
            b->image(svg);
            _r->buttons.push_back(b);
            b->tooltip(_("Close current filename"));
            bW->callback([=](auto w) { close_current_cb(w, p.ui); });

            bW = new Widget< Button >(g->x() + 90, Y, 30, 30);
            b = bW;
            svg = load_svg("FileCloseAll.svg");
            b->image(svg);
            _r->buttons.push_back(b);
            b->tooltip(_("Close all filenames"));
            bW->callback([=](auto w) { close_all_cb(w, p.ui); });

            bW = new Widget< Button >(g->x() + 120, Y, 30, 30);
            b = bW;
            svg = load_svg("Prev.svg");
            b->image(svg);
            _r->buttons.push_back(b);
            b->tooltip(_("Previous filename"));
            bW->callback([=](auto w) { App::app->filesModel()->prev(); });

            bW = new Widget< Button >(g->x() + 150, Y, 30, 30);
            b = bW;
            svg = load_svg("Next.svg");
            b->image(svg);
            _r->buttons.push_back(b);
            b->tooltip(_("Next filename"));
            bW->callback([=](auto w) { App::app->filesModel()->next(); });

            auto btW = new Widget< Fl_Button >(g->x() + 150, Y, 30, 30);
            b = btW;
            svg = load_svg("Filter.svg");
            b->image(svg);
            b->selection_color(FL_YELLOW);
            b->value(o.filterEDL);
            _r->buttons.push_back(b);
            b->tooltip(_("Filter EDLs"));
            btW->callback(
                [=](auto w)
                {
                    auto model = App::app->filesModel();
                    FilesPanelOptions o =
                        model->observeFilesPanelOptions()->get();
                    o.filterEDL ^= true;
                    model->setFilesPanelOptions(o);
                });
            bg->end();
            g->layout();

            g->end();
        }

        void FilesPanel::redraw()
        {
            TLRENDER_P();

            otio::RationalTime time = otio::RationalTime(0.0, 1.0);

            const auto player = p.ui->uiView->getTimelinePlayer();

            image::Size size(128, 64);

            const auto& model = App::app->filesModel();
            auto Aindex = model->observeAIndex()->get();
            const auto files = model->observeFiles();

            for (auto& m : _r->map)
            {
                size_t i = m.first;
                const auto& media = files->getItem(i);
                const auto& path = media->path;

                const std::string protocol = path.getProtocol();
                const std::string& dir = path.getDirectory();
                const std::string file =
                    path.getBaseName() + path.getNumber() + path.getExtension();
                const std::string fullfile = protocol + dir + file;
                FileButton* b = m.second;

                uint16_t layerId = media->videoLayer;
                const std::string layer = getLayerName(media, layerId);
                std::string text = protocol + dir + "\n" + file + layer;
                b->copy_label(text.c_str());

                b->labelcolor(FL_WHITE);
                WidgetIndices::iterator it = _r->indices.find(b);
                time = media->currentTime;
                if (Aindex != i)
                {
                    b->value(0);
                    auto bIndexes = model->observeBIndexes()->get();
                    auto stereoIndex = model->observeStereoIndex()->get();

                    bool doThumbnail = false;
                    for (const auto& bIndex : bIndexes)
                    {
                        if (bIndex == i)
                        {
                            doThumbnail = true;
                            break;
                        }
                    }

                    if (i == stereoIndex)
                    {
                        doThumbnail = true;
                    }

                    if (!doThumbnail)
                        continue;
                }
                else
                {
                    b->value(1);
                }

                if (player)
                {
                    time = player->currentTime();
                }
                layerId = p.ui->uiColorChannel->value();

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
                            fullfile, time, size, filesThumbnail_cb,
                            (void*)data, layerId);
                        _r->ids[b] = id;
                    }
                    catch (const std::exception& e)
                    {
                    }
                }
            }
        }

        void FilesPanel::setFilesPanelOptions(const FilesPanelOptions& value)
        {
            refresh();
        }

        void FilesPanel::refresh()
        {
            cancel_thumbnails();
            clear_controls();
            add_controls();
            end_group();
        }

    } // namespace panel
} // namespace mrv
