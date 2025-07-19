// SPDX-License-Identifier: BSD-3-Clause
// mrv2
// Copyright Contributors to the mrv2 Project. All rights reserved.


#include "mrViewer.h"

#include "mrvIcons/Devices.h"

#include "mrvPanels/mrvDevicesPanel.h"

#include "mrvPanels/mrvPanelsCallbacks.h"

#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvHorSlider.h"
#include "mrvWidgets/mrvPack.h"
#include "mrvWidgets/mrvDoubleSpinner.h"
#include "mrvWidgets/mrvCollapsibleGroup.h"

#if defined(TLRENDER_BMD)
#    include <tlPlay/BMDDevicesModel.h>
#endif // TLRENDER_BMD

#if defined(TLRENDER_BMD)
#    include <tlDevice/BMDOutputDevice.h>
#endif // TLRENDER_BMD

#include <FL/Fl_Choice.H>

#include <map>

namespace mrv
{
    namespace panel
    {

        struct DevicesPanel::Private
        {
            Fl_Check_Button* enabledCheckBox = nullptr;
            Fl_Choice* deviceComboBox = nullptr;
            Fl_Choice* displayModeComboBox = nullptr;
            Fl_Choice* pixelTypeComboBox = nullptr;
            Fl_Choice* videoLevelsComboBox = nullptr;
            Fl_Choice* hdrModeComboBox = nullptr;
            std::vector<std::pair<DoubleSpinner*, DoubleSpinner*> >
                primariesSpinBoxes;
            std::pair<DoubleSpinner*, DoubleSpinner*>
                masteringLuminanceSpinBoxes = std::make_pair(nullptr, nullptr);
            HorSlider* maxCLLSlider = nullptr;
            HorSlider* maxFALLSlider = nullptr;
        };

        DevicesPanel::DevicesPanel(ViewerUI* ui) :
            _r(new Private),
            PanelWidget(ui)
        {
            add_group("Devices");

            Fl_SVG_Image* svg = MRV2_LOAD_SVG(Devices);
            g->bind_image(svg);

            g->callback(
                [](Fl_Widget* w, void* d)
                {
                    ViewerUI* ui = static_cast< ViewerUI* >(d);
                    delete devicesPanel;
                    devicesPanel = nullptr;
                    ui->uiMain->fill_menu(ui->uiMenuBar);
                },
                ui);
        }

        DevicesPanel::~DevicesPanel() {}

        void DevicesPanel::add_controls()
        {
            TLRENDER_P();
            MRV2_R();

#if defined(TLRENDER_BMD)
            Fl_Box* box;
            Pack* sg;
            Fl_Group* bg;
            CollapsibleGroup* cg;
            Fl_Button* b;
            Fl_Choice* m;

            g->clear();
            g->begin();

            int X = g->x() + 70;
            int Y = g->y() + 20;

            cg = new CollapsibleGroup(g->x(), Y, g->w(), 20, "Device");
            b = cg->button();
            b->labelsize(14);
            b->size(b->w(), 18);

            Y += 20;

            cg->begin();
            bg = new Fl_Group(g->x(), Y, g->w(), 80);

            auto mW = new Widget< Fl_Choice >(
                g->x() + 130, Y, g->w() - g->x() - 130, 20, "Name:");
            m = r.deviceComboBox = mW;
            m->labelsize(12);
            m->align(FL_ALIGN_LEFT);
            Y += 20;
            mW->callback(
                [=](auto o)
                { _p->ui->app->devicesModel()->setDeviceIndex(o->value()); });

            mW = new Widget< Fl_Choice >(
                g->x() + 130, Y, g->w() - g->x() - 130, 20, "Display mode:");
            m = r.displayModeComboBox = mW;
            m->labelsize(12);
            m->align(FL_ALIGN_LEFT);
            Y += 20;
            mW->callback(
                [=](auto o) {
                    _p->ui->app->devicesModel()->setDisplayModeIndex(
                        o->value());
                });

            mW = new Widget< Fl_Choice >(
                g->x() + 130, Y, g->w() - g->x() - 130, 20, "Pixel type:");
            m = r.pixelTypeComboBox = mW;
            m->labelsize(12);
            m->align(FL_ALIGN_LEFT);
            Y += 20;
            mW->callback(
                [=](auto o) {
                    _p->ui->app->devicesModel()->setPixelTypeIndex(o->value());
                });

            mW = new Widget< Fl_Choice >(
                g->x() + 130, Y, g->w() - g->x() - 130, 20, "Video levels:");
            m = r.videoLevelsComboBox = mW;
            m->labelsize(12);
            m->align(FL_ALIGN_LEFT);
            Y += 20;
            mW->callback(
                [=](auto o)
                {
                    _p->ui->app->devicesModel()->setVideoLevels(
                        static_cast<image::VideoLevels>(o->value()));
                });

            bg->end();

            cg->end();

            cg = new CollapsibleGroup(g->x(), Y, g->w(), 20, "HDR");
            b = cg->button();
            b->labelsize(14);
            b->size(b->w(), 18);

            Y += 20;

            cg->begin();

            bg = new Fl_Group(g->x(), Y, g->w(), 25);

            mW = new Widget< Fl_Choice >(
                g->x() + 130, Y, g->w() - g->x() - 130, 20, "Mode:");
            m = r.hdrModeComboBox = mW;
            m->labelsize(12);
            m->align(FL_ALIGN_LEFT);
            Y += 25;

            mW->callback(
                [=](auto o)
                {
                    App::app->devicesModel()->setHDRMode(
                        static_cast<device::HDRMode>(o->value()));
                });

            bg->end();

            box = new Fl_Box(X, Y, g->w(), 20);
            Y += 20;

            sg = new Pack(g->x(), Y, g->w(), 25);
            sg->type(Pack::HORIZONTAL);
            sg->spacing(5);
            sg->begin();

            const std::array<std::string, image::HDRPrimaries::Count>
                primariesLabels = {
                    "Red primaries:", "Green primaries:", "Blue primaries:",
                    "White primaries:"};
            for (size_t i = 0; i < image::HDRPrimaries::Count; ++i)
            {
                sg = new Pack(g->x(), Y, g->w(), 25);
                sg->type(Pack::HORIZONTAL);
                sg->spacing(5);
                sg->begin();
                box = new Fl_Box(X, Y, 120, 20, _(primariesLabels.c_str()));
                box->labelsize(12);
                box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

                auto min = new Widget< DoubleSpinner >(X, Y, 50, 25);
                min->range(0.0, 1.0);
                min->step(0.01);
                min->callback([=](auto o) {});
                auto max = new Widget< DoubleSpinner >(X, Y, 50, 25);
                max->range(0.0, 1.0);
                max->step(0.01);
                max->callback([=](auto o) {});
                p.primariesSpinBoxes.push_back(std::make_pair(min, max));
                sg->end();

                Y += 75;
            }

            sg = new Pack(g->x(), Y, g->w(), 25);
            sg->type(Pack::HORIZONTAL);
            sg->spacing(5);
            sg->begin();

            box = new Fl_Box(X, Y, 120, 20, "White Primaries:");
            box->labelsize(12);
            box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

            dW = new Widget< DoubleSpinner >(X, Y, 50, 25);
            r.whitePrimariesSpinBoxes.first = dW;
            dW->callback(
                [=](auto o)
                {
                    auto hdrData = _p->ui->app->devicesModel()
                                       ->observeData()
                                       ->get()
                                       .hdrData;
                    hdrData.whitePrimaries.x = o->value();
                    _p->ui->app->devicesModel()->setHDRData(hdrData);
                });
            dW = new Widget< DoubleSpinner >(X, Y, 50, 25);
            r.whitePrimariesSpinBoxes.second = dW;
            dW->callback(
                [=](auto o)
                {
                    auto hdrData = _p->ui->app->devicesModel()
                                       ->observeData()
                                       ->get()
                                       .hdrData;
                    hdrData.whitePrimaries.y = o->value();
                    _p->ui->app->devicesModel()->setHDRData(hdrData);
                });

            sg->end();
            Y += 25;

            sg = new Pack(g->x(), Y, g->w(), 25);
            sg->type(Pack::HORIZONTAL);
            sg->spacing(5);
            sg->begin();

            box = new Fl_Box(X, Y, 120, 25, "Mastering Luminance:");
            box->labelsize(12);
            box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
            X += box->w();

            dW = new Widget< DoubleSpinner >(X, Y, 50, 20);
            r.masteringLuminanceSpinBoxes.first = dW;
            r.masteringLuminanceSpinBoxes.first->range(0.0, 10000.0);
            X += r.masteringLuminanceSpinBoxes.first->w() + 5;
            dW->callback(
                [=](auto o)
                {
                    auto hdrData = _p->ui->app->devicesModel()
                                       ->observeData()
                                       ->get()
                                       .hdrData;
                    hdrData.displayMasteringLuminance = math::FloatRange(
                        o->value(), hdrData.displayMasteringLuminance.getMax());
                    _p->ui->app->devicesModel()->setHDRData(hdrData);
                });

            dW = new Widget< DoubleSpinner >(X, Y, 70, 20);
            r.masteringLuminanceSpinBoxes.second = dW;
            r.masteringLuminanceSpinBoxes.second->range(0.0, 10000.0);
            dW->callback(
                [=](auto o)
                {
                    auto hdrData = _p->ui->app->devicesModel()
                                       ->observeData()
                                       ->get()
                                       .hdrData;
                    hdrData.displayMasteringLuminance = math::FloatRange(
                        hdrData.displayMasteringLuminance.getMin(), o->value());
                    _p->ui->app->devicesModel()->setHDRData(hdrData);
                });

            sg->end();

            HorSlider* s;
            auto sW =
                new Widget< HorSlider >(g->x(), Y, g->w(), 20, "Maximum CLL:");
            s = _r->maxCLLSlider = sW;
            s->range(0.F, 10000.F);
            s->step(0.1);
            sW->callback(
                [=](auto o)
                {
                    auto hdrData = _p->ui->app->devicesModel()
                                       ->observeData()
                                       ->get()
                                       .hdrData;
                    hdrData.maxCLL = o->value();
                    _p->ui->app->devicesModel()->setHDRData(hdrData);
                });
            Y += s->h();

            sW =
                new Widget< HorSlider >(g->x(), Y, g->w(), 20, "Maximum FALL:");
            s = _r->maxFALLSlider = sW;
            s->range(0.F, 10000.F);
            s->step(0.1);
            Y += s->h();
            sW->callback(
                [=](auto o)
                {
                    auto hdrData = _p->ui->app->devicesModel()
                                       ->observeData()
                                       ->get()
                                       .hdrData;
                    hdrData.maxFALL = o->value();
                    _p->ui->app->devicesModel()->setHDRData(hdrData);
                });

            cg->end();

            g->end();
#endif
        }

    } // namespace panel

} // namespace mrv
