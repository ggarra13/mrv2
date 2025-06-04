// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrViewer.h"

#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/mrvApp.h"

#include "mrvFl/mrvIO.h"

#include "mrvPanels/mrvPanelsAux.h"
#include "mrvPanels/mrvPanelsCallbacks.h"
#include "mrvPanels/mrvFilesPanel.h"

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvPack.h"
#include "mrvWidgets/mrvFileButton.h"
#include "mrvWidgets/mrvButton.h"

#include "mrvNetwork/mrvTCP.h"

#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvFile.h"

#include <string>
#include <vector>
#include <map>

namespace
{
    const char* kModule = "files";
}

namespace mrv
{
    namespace panel
    {

        typedef std::map< FileButton*, size_t > WidgetIndices;

        struct FilesPanel::Private
        {
            std::weak_ptr<system::Context> context;

            std::map< size_t, FileButton* > map;
            WidgetIndices indices;

            std::shared_ptr<
                observer::ListObserver<std::shared_ptr<FilesModelItem> > >
                filesObserver;

            std::shared_ptr< observer::ValueObserver<FilesPanelOptions> >
                filesPanelOptionsObserver;

            std::shared_ptr<observer::ValueObserver<int> > aIndexObserver;
            std::shared_ptr<observer::ListObserver<int> > layerObserver;
        };

        FilesPanel::FilesPanel(ViewerUI* ui) :
            _r(new Private),
            ThumbnailPanel(ui)
        {
            add_group("Files");
            
            g->bind_image(load_svg("Files.svg"));

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
                ui->app->filesModel()->observeAIndex(), [this](int value)
                { redraw(); }, observer::CallbackAction::Suppress);

            _r->layerObserver = observer::ListObserver<int>::create(
                ui->app->filesModel()->observeLayers(),
                [this](const std::vector<int>& value) { redraw(); },
                observer::CallbackAction::Suppress);
        }

        FilesPanel::~FilesPanel() {}

        void FilesPanel::add_controls()
        {
            TLRENDER_P();

            _r->map.clear();
            _r->indices.clear();

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

            file::Path lastPath;

            for (size_t i = 0; i < numFiles; ++i)
            {
                const auto& media = files->getItem(i);
                const auto& path = media->path;

                const bool isEDL = file::isTemporaryEDL(path);
                const bool isNDI = file::isTemporaryNDI(path);

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
                    g->x(), g->y() + 22 + i * size.h + 4, g->w(), size.h + 4);
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

                uint16_t layerId;
                time = media->currentTime;
                if (Aindex == i)
                {
                    b->value(1);
                    layerId = p.ui->uiColorChannel->value();
                    if (player)
                    {
                        time = player->currentTime();
                    }
                }
                else
                {
                    b->value(0);
                    layerId = media->videoLayer;
                }

                const std::string layer = getLayerName(media, layerId);
                std::string text = protocol + dir + "\n" + file + layer;
                b->copy_label(text.c_str());

                _createThumbnail(b, path, time, layerId, isNDI);
            }

            int Y = g->y() + 20 + numFiles * 64;

            Pack* bg = new Pack(g->x(), Y, g->w(), 30);
            bg->type(Pack::HORIZONTAL);
            bg->begin();

            Fl_Button* b;
            auto bW = new Widget< Button >(g->x(), Y, 30, 30);
            b = bW;
            b->bind_image(load_svg("FileOpen.svg"));
            b->tooltip(_("Open a filename"));
            bW->callback([=](auto w) { open_cb(w, p.ui); });

            bW = new Widget< Button >(g->x() + 30, Y, 30, 30);
            b = bW;
            b->bind_image(load_svg("FileOpenSeparateAudio.svg"));
            b->tooltip(_("Open a filename with audio"));
            bW->callback([=](auto w) { open_separate_audio_cb(w, p.ui); });

            bW = new Widget< Button >(g->x() + 60, Y, 30, 30);
            b = bW;
            b->bind_image(load_svg("FileClose.svg"));
            b->tooltip(_("Close current filename"));
            bW->callback([=](auto w) { close_current_cb(w, p.ui); });

            bW = new Widget< Button >(g->x() + 90, Y, 30, 30);
            b = bW;
            b->bind_image(load_svg("FileCloseAll.svg"));
            b->tooltip(_("Close all filenames"));
            bW->callback([=](auto w) { close_all_cb(w, p.ui); });

            bW = new Widget< Button >(g->x() + 120, Y, 30, 30);
            b = bW;
            ;
            b->bind_image(load_svg("Prev.svg"));
            b->tooltip(_("Previous filename"));
            bW->callback([=](auto w) { App::app->filesModel()->prev(); });

            bW = new Widget< Button >(g->x() + 150, Y, 30, 30);
            b = bW;
            b->bind_image(load_svg("Next.svg"));
            b->tooltip(_("Next filename"));
            bW->callback([=](auto w) { App::app->filesModel()->next(); });

            auto btW = new Widget< Fl_Button >(g->x() + 150, Y, 30, 30);
            b = btW;
            b->image(load_svg("Filter.svg"));
            b->selection_color(FL_YELLOW);
            b->value(o.filterEDL);
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

            g->end();
        }

        void FilesPanel::redraw()
        {
            TLRENDER_P();

            otio::RationalTime time = otio::RationalTime(0.0, 1.0);

            const auto player = p.ui->uiView->getTimelinePlayer();
            if (!player)
                return;

            const auto& model = App::app->filesModel();
            auto Aindex = model->observeAIndex()->get();
            const auto files = model->observeFiles();

            for (auto& m : _r->map)
            {
                size_t i = m.first;
                const auto& media = files->getItem(i);
                const auto& path = media->path;
                const bool isNDI = file::isTemporaryNDI(path);

                const std::string& protocol = path.getProtocol();
                const std::string& dir = path.getDirectory();
                const std::string file =
                    path.getBaseName() + path.getNumber() + path.getExtension();
                const std::string& fullfile = protocol + dir + file;
                FileButton* b = m.second;
                uint16_t layerId;

                b->labelcolor(FL_WHITE);
                WidgetIndices::iterator it = _r->indices.find(b);
                time = media->currentTime;
                if (Aindex != i)
                {
                    layerId = media->videoLayer;
                    b->value(0);
                }
                else
                {
                    b->value(1);
                    time = player->currentTime();
                    layerId = p.ui->uiColorChannel->value();
                }

                const std::string layer = getLayerName(media, layerId);
                std::string text = protocol + dir + "\n" + file + layer;
                b->copy_label(text.c_str());

                _createThumbnail(b, path, time, layerId, isNDI);
            }
        }

        void FilesPanel::setFilesPanelOptions(const FilesPanelOptions& value)
        {
            refresh();
        }

        void FilesPanel::refresh()
        {
            _cancelRequests();
            clearCache();
            add_controls();
            end_group();
        }

    } // namespace panel
} // namespace mrv
