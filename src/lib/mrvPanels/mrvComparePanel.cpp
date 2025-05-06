// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "mrViewer.h"

#include "mrvApp/mrvSettingsObject.h"
#include "mrvApp/mrvFilesModel.h"
#include "mrvApp/mrvApp.h"

#include "mrvPanels/mrvPanelsAux.h"
#include "mrvPanels/mrvComparePanel.h"
#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvWidgets/mrvButton.h"
#include "mrvWidgets/mrvClipButton.h"
#include "mrvWidgets/mrvCollapsibleGroup.h"
#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvHorSlider.h"
#include "mrvWidgets/mrvPopupMenu.h"

#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvFile.h"

#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_RGB_Image.H>


#include <string>
#include <vector>
#include <map>

namespace mrv
{
    namespace panel
    {

        typedef std::map< ClipButton*, size_t > WidgetIndices;

        struct ComparePanel::Private
        {
            std::map< size_t, ClipButton* > map;
            WidgetIndices indices;

            std::shared_ptr<
                observer::ListObserver<std::shared_ptr<FilesModelItem> > >
                filesObserver;

            std::shared_ptr<observer::ListObserver<int> > bIndexesObserver;
            std::shared_ptr<observer::ListObserver<int> > layerObserver;

            std::shared_ptr<observer::ValueObserver<timeline::CompareOptions> >
                compareOptionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::CompareTimeMode> >
                compareTimeObserver;
        };

        ComparePanel::ComparePanel(ViewerUI* ui) :
            _r(new Private),
            ThumbnailPanel(ui)
        {
            add_group("Compare");

            Fl_SVG_Image* svg = load_svg("Compare.svg");
            g->bind_image(svg);

            g->callback(
                [](Fl_Widget* w, void* d)
                {
                    ViewerUI* ui = static_cast< ViewerUI* >(d);
                    delete comparePanel;
                    comparePanel = nullptr;
                    ui->uiMain->fill_menu(ui->uiMenuBar);
                },
                ui);

            _r->filesObserver = observer::
                ListObserver<std::shared_ptr<FilesModelItem> >::create(
                    ui->app->filesModel()->observeFiles(),
                    [this](const std::vector< std::shared_ptr<FilesModelItem> >&
                               value) { refresh(); });

            _r->bIndexesObserver = observer::ListObserver<int>::create(
                ui->app->filesModel()->observeBIndexes(),
                [this](const std::vector<int>& value) { redraw(); });

            _r->layerObserver = observer::ListObserver<int>::create(
                ui->app->filesModel()->observeLayers(),
                [this](const std::vector<int>& value) { redraw(); });

            _r->compareOptionsObserver =
                observer::ValueObserver<timeline::CompareOptions>::create(
                    ui->app->filesModel()->observeCompareOptions(),
                    [this](const timeline::CompareOptions& value)
                    { setCompareOptions(value); });

            _r->compareTimeObserver =
                observer::ValueObserver<timeline::CompareTimeMode>::create(
                    ui->app->filesModel()->observeCompareTime(),
                    [this](const timeline::CompareTimeMode& value)
                        { setCompareTime(value); });
        }

        ComparePanel::~ComparePanel() {}

        void ComparePanel::add_controls()
        {
            TLRENDER_P();

            _r->map.clear();
            _r->indices.clear();

            auto settings = p.ui->app->settings();
            const std::string& prefix = tab_prefix();

            g->clear();
            g->begin();
            const auto& model = p.ui->app->filesModel();
            const auto& files = model->observeFiles();
            size_t numFiles = files->getSize();
            auto Bindices = model->observeBIndexes()->get();
            auto Aindex = model->observeAIndex()->get();

            const auto player = p.ui->uiView->getTimelinePlayer();

            otio::RationalTime time = otio::RationalTime(0.0, 1.0);
            if (player)
                time = player->currentTime();

            const image::Size size(128, 64);

            file::Path lastPath;
            int Y = g->y();

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

                const std::string& protocol = path.getProtocol();
                const std::string& dir = path.getDirectory();
                const std::string& base = path.getBaseName();
                const std::string& extension = path.getExtension();
                const std::string file = base + path.getNumber() + extension;
                const std::string fullfile = protocol + dir + file;

                auto bW = new Widget<ClipButton>(
                    g->x(), g->y() + 20 + i * size.h + 4, g->w(), size.h + 4);
                ClipButton* b = bW;
                b->tooltip(_("Select one or more B images."));
                _r->indices[b] = i;

                uint16_t layerId = media->videoLayer;
                time = media->currentTime;
                if (Aindex == i)
                {
                    layerId = p.ui->uiColorChannel->value();
                    if (player)
                        time = player->currentTime();
                }

                for (auto Bindex : Bindices)
                {
                    if (Bindex == i)
                    {
                        b->value(1);
                        break;
                    }
                }
                bW->callback(
                    [=](auto b)
                    {
                        WidgetIndices::const_iterator it = _r->indices.find(b);
                        if (it == _r->indices.end())
                            return;
                        int index = (*it).second;
                        const auto& model = p.ui->app->filesModel();
                        const auto bIndexes = model->observeBIndexes()->get();
                        const auto i =
                            std::find(bIndexes.begin(), bIndexes.end(), index);
                        model->setB(index, i == bIndexes.end());
                    });

                _r->map[i] = b;

                const std::string& layer = getLayerName(media, layerId);
                std::string text = protocol + dir + "\n" + file + layer;
                b->copy_label(text.c_str());

                _createThumbnail(b, path, time, layerId, isNDI);

                Y += size.h;
            }

            int X = g->x();

            auto cMode = new Widget< PopupMenu >(X, Y, g->w(), 30);
            PopupMenu* pm = compareTimeW = cMode;
            const auto timeLabels = timeline::getCompareTimeModeLabels();
            for (const auto& label : timeLabels)
            {
                pm->add(label.c_str());
            }
            int v = static_cast<int>(model->getCompareTime());
            pm->value(v);
            pm->tooltip(_("Select between Relative or Absolute Compare Time Mode"));

            Fl_Group* bg = new Fl_Group(X, Y, g->w(), 30);
            bg->begin();

            cMode->callback(
                [=](auto w)
                {
                    auto o = static_cast<timeline::CompareTimeMode>(w->value());
                    model->setCompareTime(o);
                });

            
            Fl_Button* b;
            auto bW = new Widget< Button >(X, Y, 30, 30);
            b = bW;
            b->bind_image(load_svg("CompareA.svg"));
            b->tooltip(_("Compare A"));
            bW->callback(
                [=](auto w)
                {
                    compare_a_cb(nullptr, p.ui);
                });

            bW = new Widget< Button >(X + 30, Y, 30, 30);
            b = bW;
            b->bind_image(load_svg("CompareB.svg"));
            b->tooltip(_("Compare B"));

            bW->callback(
                [=](auto w)
                {
                    compare_b_cb(nullptr, p.ui);
                });

            bW = new Widget< Button >(X + 60, Y, 30, 30);
            b = bW;
            b->bind_image(load_svg("CompareWipe.svg"));
            b->tooltip(
#ifdef __APPLE__
                _("Wipe between the A and B files\n\n"
                  "Use the Option key + left mouse button to move the wipe in "
                  "X or "
                  "in Y")
#else
                _("Wipe between the A and B files\n\n"
                  "Use the Alt key + left mouse button to move the wipe in X "
                  "or in "
                  "Y.")
#endif
            );

            bW->callback(
                [=](auto w)
                {
                    compare_wipe_cb(nullptr, p.ui);
                });

            bW = new Widget< Button >(X + 90, Y, 30, 30);
            b = bW;
            b->bind_image(load_svg("CompareOverlay.svg"));
            b->tooltip(
                _("Overlay the A and B files with optional transparencyy"));

            bW->callback(
                [=](auto w)
                {
                    compare_overlay_cb(nullptr, p.ui);
                });

            bW = new Widget< Button >(X + 120, Y, 30, 30);
            b = bW;
            b->bind_image(load_svg("CompareDifference.svg"));
            b->tooltip(_("Difference the A and B files"));

            bW->callback(
                [=](auto w)
                {
                    compare_difference_cb(nullptr, p.ui);
                });

            bW = new Widget< Button >(X + 150, Y, 30, 30);
            b = bW;
            b->bind_image(load_svg("CompareHorizontal.svg"));
            b->tooltip(_("Compare the A and B files side by side"));

            bW->callback(
                [=](auto w)
                {
                    compare_horizontal_cb(nullptr, p.ui);
                });

            bW = new Widget< Button >(X + 180, Y, 30, 30);
            b = bW;
            b->bind_image(load_svg("CompareVertical.svg"));
            b->tooltip(_("Show the A file above the B file"));

            bW->callback(
                [=](auto w)
                {
                    compare_vertical_cb(nullptr, p.ui);
                });

            bW = new Widget< Button >(X + 210, Y, 30, 30);
            b = bW;
            b->bind_image(load_svg("CompareTile.svg"));
            b->tooltip(_("Tile the A and B files"));

            bW->callback(
                [=](auto w)
                {
                    compare_tile_cb(nullptr, p.ui);
                });

            bW = new Widget< Button >(X + 240, Y, 30, 30);
            b = bW;
            b->bind_image(load_svg("Prev.svg"));
            b->tooltip(_("Previous filename"));
            bW->callback(
                [=](auto w)
                {
                    if (p.ui->app->filesModel()->observeFiles()->getSize() > 0)
                        p.ui->app->filesModel()->prevB();
                });

            bW = new Widget< Button >(X + 270, Y, 30, 30);
            b = bW;
            b->bind_image(load_svg("Next.svg"));
            b->tooltip(_("Next filename"));
            bW->callback(
                [=](auto w)
                {
                    if (p.ui->app->filesModel()->observeFiles()->getSize() > 0)
                        p.ui->app->filesModel()->nextB();
                });

            bg->resizable(0);
            bg->end();

            HorSlider* s;
            CollapsibleGroup* cg =
                new CollapsibleGroup(g->x(), 20, g->w(), 20, _("Wipe"));
            b = cg->button();
            b->labelsize(14);
            b->size(b->w(), 18);
            b->callback(
                [](Fl_Widget* w, void* d)
                {
                    CollapsibleGroup* cg = static_cast<CollapsibleGroup*>(d);
                    if (cg->is_open())
                        cg->close();
                    else
                        cg->open();

                    const std::string& prefix = comparePanel->tab_prefix();
                    const std::string key = prefix + "Wipe";

                    App* app = App::ui->app;
                    auto settings = app->settings();
                    settings->setValue(key, static_cast<int>(cg->is_open()));

                    comparePanel->refresh();
                },
                cg);

            cg->begin();

            auto sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, "X");
            s = wipeX = sV;
            s->tooltip(
#ifdef __APPLE__
                _("Use the Option key + left mouse button to move the wipe in "
                  "X.")
#else
                _("Use the Alt key + left mouse button to move the wipe in X.")
#endif
            );
            s->range(0.f, 1.0f);
            s->step(0.01F);
            s->default_value(0.5f);
            auto o = model->observeCompareOptions()->get();
            s->value(o.wipeCenter.x);
            sV->callback(
                [=](auto w)
                {
                    auto o = model->observeCompareOptions()->get();
                    o.wipeCenter.x = w->value();
                    model->setCompareOptions(o);
                });

            sV = new Widget< HorSlider >(g->x(), 110, g->w(), 20, "Y");
            s = wipeY = sV;
            s->tooltip(
#ifdef __APPLE__
                _("Use the Option key + left mouse button to move the wipe in "
                  "Y.")
#else
                _("Use the Alt key + left mouse button to move the wipe in Y.")
#endif
            );
            s->range(0.f, 1.0f);
            s->step(0.01F);
            s->default_value(0.5f);
            s->value(o.wipeCenter.y);
            sV->callback(
                [=](auto w)
                {
                    auto o = model->observeCompareOptions()->get();
                    o.wipeCenter.y = w->value();
                    model->setCompareOptions(o);
                });

            sV =
                new Widget< HorSlider >(g->x(), 130, g->w(), 20, _("Rotation"));
            s = wipeRotation = sV;
            s->tooltip(
                _("Wipe Rotation.  Use Shift + left mouse button along X to "
                  "rotate wipe."));
            s->range(0.f, 360.0f);
            s->default_value(0.0f);
            s->value(o.wipeRotation);
            sV->callback(
                [=](auto w)
                {
                    auto o = model->observeCompareOptions()->get();
                    o.wipeRotation = w->value();
                    model->setCompareOptions(o);
                });

            cg->end();

            std::string key = prefix + "Wipe";
            std_any value = settings->getValue<std::any>(key);
            int open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
            if (!open)
                cg->close();

            cg = new CollapsibleGroup(g->x(), 20, g->w(), 20, _("Overlay"));
            b = cg->button();
            b->labelsize(14);
            b->size(b->w(), 18);
            b->callback(
                [](Fl_Widget* w, void* d)
                {
                    CollapsibleGroup* cg = static_cast<CollapsibleGroup*>(d);
                    if (cg->is_open())
                        cg->close();
                    else
                        cg->open();

                    const std::string& prefix = comparePanel->tab_prefix();
                    const std::string key = prefix + "Overlay";

                    App* app = App::ui->app;
                    auto settings = app->settings();
                    settings->setValue(key, static_cast<int>(cg->is_open()));

                    comparePanel->refresh();
                },
                cg);

            cg->begin();

            sV = new Widget< HorSlider >(g->x(), 90, g->w(), 20, _("Overlay"));
            s = overlay = sV;
            s->range(0.f, 1.0f);
            s->step(0.01F);
            s->default_value(0.5f);
            s->value(o.overlay);
            s->tooltip(
#ifdef __APPLE__
                _("Use the Option key + left mouse button to change "
                  "transparency.")
#else
                _("Use the Alt key + left mouse button to change transparency.")
#endif
            );
            sV->callback(
                [=](auto w)
                {
                    auto o = model->observeCompareOptions()->get();
                    o.overlay = w->value();
                    model->setCompareOptions(o);
                });

            cg->end();

            key = prefix + "Overlay";
            value = settings->getValue<std::any>(key);
            open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
            if (!open)
                cg->close();

            g->end();
        }

        void ComparePanel::redraw()
        {
            TLRENDER_P();

            const auto player = p.ui->uiView->getTimelinePlayer();
            if (!player)
                return;
            otio::RationalTime time;

            const image::Size size(128, 64);

            const auto& model = p.ui->app->filesModel();
            const auto& files = model->observeFiles();
            size_t numFiles = files->getSize();

            auto Aindex = model->observeAIndex()->get();
            auto Bindices = model->observeBIndexes()->get();
            auto o = model->observeCompareOptions()->get();

            for (int i = 0; i < numFiles; ++i)
            {
                const auto& media = files->getItem(i);
                const auto& path = media->path;
                const bool isNDI = file::isTemporaryNDI(path);

                const std::string& protocol = path.getProtocol();
                const std::string& dir = path.getDirectory();
                const std::string file =
                    path.getBaseName() + path.getNumber() + path.getExtension();
                const std::string fullfile = protocol + dir + file;

                auto m = _r->map.find(i);
                ClipButton* b = (*m).second;

                uint16_t layerId = media->videoLayer;
                bool found = false;
                if (Aindex == i)
                {
                    b->value(0);
                    found = true;
                    layerId = p.ui->uiColorChannel->value();
                }

                for (auto Bindex : Bindices)
                {
                    if (Bindex == i)
                    {
                        found = true;
                        b->value(1);
                        break;
                    }
                }
                if (found)
                {
                    time = player->currentTime();
                }
                else
                {
                    b->value(0);
                    time = media->currentTime;
                }
                b->redraw();

                const std::string& layer = getLayerName(media, layerId);
                std::string text = protocol + dir + "\n" + file + layer;
                b->copy_label(text.c_str());

                _createThumbnail(b, path, time, layerId, isNDI);
            }
        }

        void
        ComparePanel::setCompareOptions(const timeline::CompareOptions& value)
        {
            wipeX->value(value.wipeCenter.x);
            wipeY->value(value.wipeCenter.y);
            wipeRotation->value(value.wipeRotation);
            overlay->value(value.overlay);
        }

        void
        ComparePanel::setCompareTime(const timeline::CompareTimeMode& value)
        {
            compareTimeW->value(static_cast<int>(value));
        }
        
        void ComparePanel::refresh()
        {
            _cancelRequests();
            clearCache();
            add_controls();
            end_group();
        }

    } // namespace panel

} // namespace mrv
