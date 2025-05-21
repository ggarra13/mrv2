// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrViewer.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrvPanels/mrvPanelsAux.h"
#include "mrvPanels/mrvPanelsCallbacks.h"
#include "mrvPanels/mrvStereo3DPanel.h"

#include "mrvWidgets/mrvClipButton.h"
#include "mrvWidgets/mrvCollapsibleGroup.h"
#include "mrvWidgets/mrvHorSlider.h"
#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvPopupMenu.h"

#include "mrvCore/mrvHome.h"
#include "mrvCore/mrvFile.h"

#include <FL/Fl_Group.H>
#include <FL/Fl_Check_Button.H>




namespace mrv
{
    namespace panel
    {

        typedef std::map< ClipButton*, size_t > WidgetIndices;

        struct Stereo3DPanel::Private
        {
            PopupMenu* input = nullptr;
            PopupMenu* output = nullptr;
            HorSlider* eyeSeparation = nullptr;
            Fl_Check_Button* swapEyes = nullptr;

            std::map< size_t, ClipButton* > map;
            WidgetIndices indices;

            std::shared_ptr<
                observer::ListObserver<std::shared_ptr<FilesModelItem> > >
                filesObserver;
            std::shared_ptr<observer::ValueObserver<int> > stereoIndexObserver;

            std::shared_ptr<observer::ListObserver<int> > layerObserver;
        };

        Stereo3DPanel::Stereo3DPanel(ViewerUI* ui) :
            ThumbnailPanel(ui),
            _r(new Private)
        {
            add_group("Stereo 3D");
            g->bind_image(load_svg("Stereo3D.svg"));

            _r->filesObserver = observer::
                ListObserver<std::shared_ptr<FilesModelItem> >::create(
                    ui->app->filesModel()->observeFiles(),
                    [this](const std::vector< std::shared_ptr<FilesModelItem> >&
                               value) { refresh(); });

            _r->stereoIndexObserver = observer::ValueObserver<int>::create(
                ui->app->filesModel()->observeStereoIndex(),
                [this](int value) { redraw(); });

            _r->layerObserver = observer::ListObserver<int>::create(
                ui->app->filesModel()->observeLayers(),
                [this](const std::vector<int>& value) { redraw(); });

            g->callback(
                [](Fl_Widget* w, void* d)
                {
                    delete stereo3DPanel;
                    stereo3DPanel = nullptr;
                    ViewerUI* ui = static_cast< ViewerUI* >(d);
                    ui->uiMain->fill_menu(ui->uiMenuBar);
                },
                ui);
        }

        Stereo3DPanel::~Stereo3DPanel() {}

        void Stereo3DPanel::add_controls()
        {
            TLRENDER_P();

            _r->map.clear();
            _r->indices.clear();

            auto settings = App::app->settings();
            const std::string& prefix = tab_prefix();

            const auto& model = App::app->filesModel();

            std_any value;
            int v;

            g->clear();

            g->begin();

            const auto& files = model->observeFiles();
            size_t numFiles = files->getSize();
            auto Aindex = model->observeAIndex()->get();
            auto stereoIndex = model->observeStereoIndex()->get();

            auto player = p.ui->uiView->getTimelinePlayer();

            otio::RationalTime time = otio::RationalTime(0.0, 1.0);
            if (player)
                time = player->currentTime();

            image::Size size(128, 64);

            file::Path lastPath;

            const std::string tmpdir = tmppath() + "/";

            for (size_t i = 0; i < numFiles; ++i)
            {
                const auto& media = files->getItem(i);
                const auto& path = media->path;

                const bool isNDI = file::isTemporaryNDI(path);

                // We skip EDLs created in tmp dir here.
                const bool isEDL = file::isTemporaryEDL(path);

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
                    g->x(), g->y() + 20 + i * 68, g->w(), 68);
                ClipButton* b = bW;
                b->tooltip(_("Toggle other eye stereo image."));
                _r->indices[b] = i;

                uint16_t layerId = media->videoLayer;
                if (Aindex == i)
                {
                    layerId = p.ui->uiColorChannel->value();
                }
                
                if (stereoIndex == i)
                {
                    b->value(1);
                }
                else
                {
                    b->value(0);
                }
                bW->callback(
                    [=](auto b)
                    {
                        WidgetIndices::const_iterator it = _r->indices.find(b);
                        if (it == _r->indices.end())
                            return;
                        int index = (*it).second;
                        if (b->value())
                            index = -1;
                        const auto& model = App::app->filesModel();
                        model->setStereo(index);
                    });

                _r->map.insert(std::make_pair(i, b));

                const std::string& layer = getLayerName(media, layerId);
                std::string text = protocol + dir + "\n" + file + layer;
                b->copy_label(text.c_str());

                _createThumbnail(b, path, time, layerId, isNDI);
            }

            Stereo3DOptions o = model->observeStereo3DOptions()->get();

            HorSlider* s;
            Fl_Group* bg;
            PopupMenu* m;
            Fl_Check_Button* c;

            CollapsibleGroup* cg =
                new CollapsibleGroup(g->x(), 20, g->w(), 20, _("Stereo 3D"));
            auto b = cg->button();
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

                    const std::string& prefix = stereo3DPanel->tab_prefix();
                    const std::string key = prefix + "Stereo 3D";

                    App* app = App::ui->app;
                    auto settings = app->settings();
                    settings->setValue(key, static_cast<int>(cg->is_open()));

                    stereo3DPanel->refresh();
                },
                cg);

            cg->begin();

            bg = new Fl_Group(g->x(), 20, g->w(), 20);
            bg->begin();

            Fl_Box* box = new Fl_Box(g->x(), 20, 70, 20, _("Input"));
            box->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);

            auto pW = new Widget< PopupMenu >(
                g->x() + 70, 20, g->w() - 70, 20, "None");
            m = _r->input = pW;
            m->add("None");
            m->add(_("Image"));
            m->value(static_cast<int>(o.input));
            // m->add(_("Horizontal"));
            // m->add(_("Vertical"));
            pW->callback(
                [=](auto w)
                {
                    Stereo3DOptions o = model->observeStereo3DOptions()->get();
                    o.input = static_cast<Stereo3DInput>(w->value());
                    model->setStereo3DOptions(o);
                    set_stereo_cb(nullptr, nullptr);
                });

            bg->end();

            bg = new Fl_Group(g->x(), 20, g->w(), 20);
            bg->begin();

            box = new Fl_Box(g->x(), 20, 70, 20, _("Output"));
            box->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);

            pW = new Widget< PopupMenu >(
                g->x() + 70, 20, g->w() - 70, 20, _("Anaglyph"));
            m = _r->output = pW;
            m->add(_("Anaglyph"));
            m->add(_("Scanlines"));
            m->add(_("Columns"));
            m->add(_("Checkerboard"));
            m->value(static_cast<int>(o.output));
            if (p.ui->uiView->can_do(FL_STEREO))
                m->add(_("Graphics Card"));

            pW->callback(
                [=](auto w)
                {
                    Stereo3DOptions o = model->observeStereo3DOptions()->get();
                    o.output = static_cast<Stereo3DOutput>(w->value());
                    model->setStereo3DOptions(o);
                });

            bg->end();

            cg->end();

            std::string key = prefix + "Stereo 3D";
            value = settings->getValue<std::any>(key);
            int open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
            if (!open)
                cg->close();

            cg = new CollapsibleGroup(g->x(), 20, g->w(), 20, _("Adjustments"));
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

                    const std::string& prefix = stereo3DPanel->tab_prefix();
                    const std::string key = prefix + "Adjustments";

                    App* app = App::ui->app;
                    auto settings = app->settings();
                    settings->setValue(key, static_cast<int>(cg->is_open()));

                    stereo3DPanel->refresh();
                },
                cg);

            cg->begin();

            auto sV = new Widget< HorSlider >(
                g->x(), 90, g->w(), 20, _("Eye Separation"));
            s = _r->eyeSeparation = sV;
            s->tooltip(_("Separation of left and right eye."));
            s->range(-50.0f, 50.0f);
            s->step(0.1F);
            s->default_value(0.0f);
            s->value(o.eyeSeparation);
            sV->callback(
                [=](auto w)
                {
                    auto model = App::app->filesModel();
                    Stereo3DOptions o = model->observeStereo3DOptions()->get();
                    o.eyeSeparation = w->value();
                    model->setStereo3DOptions(o);
                });

            bg = new Fl_Group(g->x(), 110, g->w(), 20);
            bg->begin();

            auto cV = new Widget< Fl_Check_Button >(
                g->x() + 100, 110, g->w() - 100, 20, _("Swap Eyes"));
            c = _r->swapEyes = cV;
            c->tooltip(_("Swap left and right eyes."));
            c->labelsize(12);
            c->align(FL_ALIGN_LEFT);
            c->value(o.swapEyes);
            cV->callback(
                [=](auto w)
                {
                    auto model = App::app->filesModel();
                    Stereo3DOptions o = model->observeStereo3DOptions()->get();
                    o.swapEyes = static_cast<bool>(w->value());
                    model->setStereo3DOptions(o);
                });

            bg->end();

            cg->end();

            key = prefix + "Adjustments";
            value = settings->getValue<std::any>(key);
            open = std_any_empty(value) ? 1 : std_any_cast<int>(value);
            if (!open)
                cg->close();

            g->end();
        }

        void Stereo3DPanel::redraw()
        {

            TLRENDER_P();

            otio::RationalTime time = otio::RationalTime(0.0, 1.0);

            const auto player = p.ui->uiView->getTimelinePlayer();
            if (!player)
                return;

            image::Size size(128, 64);

            const auto& model = App::app->filesModel();
            auto Aindex = model->observeAIndex()->get();
            auto stereoIndex = model->observeStereoIndex()->get();
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
                const std::string fullfile = protocol + dir + file;
                ClipButton* b = m.second;

                uint16_t layerId = media->videoLayer;
                bool found = false;
                if (Aindex == i)
                {
                    layerId = p.ui->uiColorChannel->value();
                    found = true;
                }
                
                if (stereoIndex != i)
                {
                    b->value(0);
                }
                else
                {
                    found = true;
                    b->value(1);
                }

                if (found)
                {
                    time = player->currentTime();
                }
                else
                {
                    time = media->currentTime;
                }
                
                const std::string& layer = getLayerName(media, layerId);
                std::string text = protocol + dir + "\n" + file + layer;
                b->copy_label(text.c_str());
                b->labelcolor(FL_WHITE);

                _createThumbnail(b, path, time, layerId, isNDI);
            }
        }

        void Stereo3DPanel::setStereo3DOptions(const Stereo3DOptions& value)
        {
            MRV2_R();
            r.input->value(static_cast<int>(value.input));
            r.output->value(static_cast<int>(value.output));
            r.eyeSeparation->value(value.eyeSeparation);
            r.swapEyes->value(value.swapEyes);
        }

        void Stereo3DPanel::refresh()
        {
            _cancelRequests();
            clearCache();
            add_controls();
            end_group();
        }

    } // namespace panel

} // namespace mrv
