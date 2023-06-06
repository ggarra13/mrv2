// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include <FL/Fl_Group.H>
#include <FL/Fl_Check_Button.H>

#include "mrvWidgets/mrvClipButton.h"
#include "mrvWidgets/mrvCollapsibleGroup.h"
#include "mrvWidgets/mrvHorSlider.h"
#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvPopupMenu.h"

#include "mrvPanels/mrvPanelsAux.h"
#include "mrvPanels/mrvPanelsCallbacks.h"
#include "mrvPanels/mrvStereo3DPanel.h"

#include "mrvGL/mrvThumbnailCreator.h"

#include "mrvApp/mrvSettingsObject.h"

#include "mrViewer.h"

namespace mrv
{
    typedef std::map< ClipButton*, int64_t > WidgetIds;
    typedef std::map< ClipButton*, size_t > WidgetIndices;

    struct Stereo3DPanel::Private
    {
        PopupMenu* input = nullptr;
        PopupMenu* output = nullptr;
        HorSlider* eyeSeparation = nullptr;
        Fl_Check_Button* swapEyes = nullptr;

        std::weak_ptr<system::Context> context;
        mrv::ThumbnailCreator* thumbnailCreator;

        std::map< size_t, ClipButton* > map;
        WidgetIds ids;
        WidgetIndices indices;

        std::shared_ptr<
            observer::ListObserver<std::shared_ptr<FilesModelItem> > >
            filesObserver;
        std::shared_ptr<observer::ValueObserver<int> > stereoIndexObserver;

        std::shared_ptr<observer::ListObserver<int> > layerObserver;
    };

    struct ThumbnailData
    {
        ClipButton* widget;
    };

    void stereo3DThumbnail_cb(
        const int64_t id,
        const std::vector< std::pair<otime::RationalTime, Fl_RGB_Image*> >&
            thumbnails,
        void* opaque)
    {
        ThumbnailData* data = static_cast< ThumbnailData* >(opaque);
        ClipButton* w = data->widget;
        if (stereo3DPanel)
            stereo3DPanel->stereo3DThumbnail(id, thumbnails, w);
        delete data;
    }

    void Stereo3DPanel::stereo3DThumbnail(
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
        }
        else
        {
            for (const auto& i : thumbnails)
            {
                delete i.second;
            }
        }
    }

    Stereo3DPanel::Stereo3DPanel(ViewerUI* ui) :
        PanelWidget(ui),
        _r(new Private)
    {
        _r->context = ui->app->getContext();

        add_group("Stereo 3D");

        Fl_SVG_Image* svg = load_svg("Stereo3D.svg");
        g->image(svg);

        _r->filesObserver =
            observer::ListObserver<std::shared_ptr<FilesModelItem> >::create(
                ui->app->filesModel()->observeFiles(),
                [this](
                    const std::vector< std::shared_ptr<FilesModelItem> >& value)
                { refresh(); });

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

    Stereo3DPanel::~Stereo3DPanel()
    {
        TLRENDER_P();

        cancel_thumbnails();
        clear_controls();
    }

    void Stereo3DPanel::clear_controls()
    {
        for (const auto& i : _r->map)
        {
            ClipButton* b = i.second;
            delete b->image();
            b->image(nullptr);
            g->remove(b);
            delete b;
        }

        _r->map.clear();
        _r->indices.clear();
    }

    void Stereo3DPanel::add_controls()
    {
        TLRENDER_P();

        auto settingsObject = p.ui->app->settingsObject();

        _r->thumbnailCreator =
            p.ui->uiTimeWindow->uiTimeline->thumbnailCreator();

        const auto& model = p.ui->app->filesModel();

        std_any value;
        int v;

        g->clear();

        g->begin();

        const auto& files = model->observeFiles();
        size_t numFiles = files->getSize();
        auto aIndex = model->observeAIndex()->get();
        auto stereoIndex = model->observeStereoIndex()->get();

        auto player = p.ui->uiView->getTimelinePlayer();

        otio::RationalTime time = otio::RationalTime(0.0, 1.0);
        if (player)
            time = player->currentTime();

        imaging::Size size(128, 64);

        for (size_t i = 0; i < numFiles; ++i)
        {
            const auto& media = files->getItem(i);
            const auto& path = media->path;

            const std::string& dir = path.getDirectory();
            const std::string file =
                path.getBaseName() + path.getNumber() + path.getExtension();
            const std::string fullfile = dir + file;

            auto bW = new Widget<ClipButton>(
                g->x(), g->y() + 20 + i * 68, g->w(), 68);
            ClipButton* b = bW;
            _r->indices[b] = i;
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
                    const auto& model = p.ui->app->filesModel();
                    model->setStereo(index);
                });

            _r->map.insert(std::make_pair(i, b));

            const std::string& layer = getLayerName(media->videoLayer, p.ui);
            std::string text = dir + "\n" + file + layer;
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
                        timeline::Timeline::create(path.get(), context);
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
                        fullfile, time, size, stereo3DThumbnail_cb,
                        (void*)data);
                    _r->ids[b] = id;
                }
                catch (const std::exception&)
                {
                }
            }
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
        cg->layout();
        cg->begin();

        bg = new Fl_Group(g->x(), 20, g->w(), 20);
        bg->begin();

        Fl_Box* box = new Fl_Box(g->x(), 20, 70, 20, _("Input"));
        box->align(FL_ALIGN_RIGHT | FL_ALIGN_INSIDE);

        auto pW = new Widget< PopupMenu >(
            g->x() + 70, 20, g->w() - 70, 20, _("None"));
        m = _r->input = pW;
        m->add(_("None"));
        m->add(_("Image"));
        // m->add(_("Horizontal"));
        // m->add(_("Vertical"));
        pW->callback(
            [=](auto w)
            {
                Stereo3DOptions o = model->observeStereo3DOptions()->get();
                o.input = static_cast<Stereo3DOptions::Input>(w->value());
                model->setStereo3DOptions(o);
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
#if 1
        m->add(_("Checkerboard"));
#endif
        if (p.ui->uiView->can_do(FL_STEREO))
            m->add(_("OpenGL"));
        pW->callback(
            [=](auto w)
            {
                Stereo3DOptions o = model->observeStereo3DOptions()->get();
                o.output = static_cast<Stereo3DOptions::Output>(w->value());
                model->setStereo3DOptions(o);
            });

        bg->end();

        cg->end();

        cg = new CollapsibleGroup(g->x(), 20, g->w(), 20, _("Adjustments"));
        b = cg->button();
        b->labelsize(14);
        b->size(b->w(), 18);
        cg->layout();
        cg->begin();

        auto sV = new Widget< HorSlider >(
            g->x(), 90, g->w(), 20, _("Eye Separation"));
        s = _r->eyeSeparation = sV;
        s->tooltip(_("Separation of left and right eye."));
        s->range(-50.0f, 50.0f);
        s->step(0.1F);
        s->default_value(0.0f);
        sV->callback(
            [=](auto w)
            {
                auto model = p.ui->app->filesModel();
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
                auto model = p.ui->app->filesModel();
                Stereo3DOptions o = model->observeStereo3DOptions()->get();
                o.swapEyes = static_cast<bool>(w->value());
                model->setStereo3DOptions(o);
            });

        bg->end();

        cg->end();

        g->end();
    }

    void Stereo3DPanel::redraw()
    {

        TLRENDER_P();

        otio::RationalTime time = otio::RationalTime(0.0, 1.0);

        const auto player = p.ui->uiView->getTimelinePlayer();
        if (player)
            time = player->currentTime();

        imaging::Size size(128, 64);

        const auto& model = p.ui->app->filesModel();
        auto aIndex = model->observeAIndex()->get();
        auto stereoIndex = model->observeStereoIndex()->get();
        const auto files = model->observeFiles();

        for (auto& m : _r->map)
        {
            size_t i = m.first;
            const auto& media = files->getItem(i);
            const auto& path = media->path;

            const std::string& dir = path.getDirectory();
            const std::string file =
                path.getBaseName() + path.getNumber() + path.getExtension();
            const std::string fullfile = dir + file;
            ClipButton* b = m.second;

            const std::string& layer = getLayerName(media->videoLayer, p.ui);
            std::string text = dir + "\n" + file + layer;
            b->copy_label(text.c_str());

            b->labelcolor(FL_WHITE);
            if (stereoIndex != i)
            {
                b->value(0);
                if (b->image())
                    continue;
                time = otio::RationalTime(0.0, 1.0);
            }
            else
            {
                b->value(1);
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
                        fullfile, time, size, stereo3DThumbnail_cb,
                        (void*)data);
                    _r->ids[b] = id;
                }
                catch (const std::exception& e)
                {
                }
            }
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

    void Stereo3DPanel::cancel_thumbnails()
    {
        for (const auto& it : _r->ids)
        {
            _r->thumbnailCreator->cancelRequests(it.second);
        }

        _r->ids.clear();
    }

    void Stereo3DPanel::refresh()
    {
        cancel_thumbnails();
        clear_controls();
        add_controls();
        end_group();
    }

} // namespace mrv
