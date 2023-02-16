// SPDX-License-Identifier: BSD-3-Clause
// mrv2 (mrViewer2)
// Copyright Contributors to the mrv2 Project. All rights reserved.

#include "mrvDevicesPanel.h"

#include <FL/Fl_Choice.H>

#include <map>

#include "mrViewer.h"
#include "mrvApp/mrvDevicesModel.h"
#include "mrvPanelsCallbacks.h"
#include "mrvWidgets/mrvDoubleSpinner.h"
#include "mrvWidgets/mrvFunctional.h"
#include "mrvWidgets/mrvHorSlider.h"
#include "mrvWidgets/mrvPack.h"

namespace mrv {

struct DevicesPanel::Private {
  std::shared_ptr<observer::ValueObserver<DevicesModelData>> dataObserver;
  Fl_Choice *deviceComboBox = nullptr;
  Fl_Choice *displayModeComboBox = nullptr;
  Fl_Choice *pixelTypeComboBox = nullptr;
  Fl_Choice *videoLevelsComboBox = nullptr;
  Fl_Choice *hdrModeComboBox = nullptr;
  std::pair<DoubleSpinner *, DoubleSpinner *> redPrimariesSpinBoxes =
      std::make_pair(nullptr, nullptr);
  std::pair<DoubleSpinner *, DoubleSpinner *> greenPrimariesSpinBoxes =
      std::make_pair(nullptr, nullptr);
  std::pair<DoubleSpinner *, DoubleSpinner *> bluePrimariesSpinBoxes =
      std::make_pair(nullptr, nullptr);
  std::pair<DoubleSpinner *, DoubleSpinner *> whitePrimariesSpinBoxes =
      std::make_pair(nullptr, nullptr);
  std::pair<DoubleSpinner *, DoubleSpinner *> masteringLuminanceSpinBoxes =
      std::make_pair(nullptr, nullptr);
  HorSlider *maxCLLSlider = nullptr;
  HorSlider *maxFALLSlider = nullptr;
};

DevicesPanel::DevicesPanel(ViewerUI *ui) : _r(new Private), PanelWidget(ui) {
  add_group("Devices");

  Fl_SVG_Image *svg = load_svg("Devices.svg");
  g->image(svg);

  g->callback(
      [](Fl_Widget *w, void *d) {
        ViewerUI *ui = static_cast<ViewerUI *>(d);
        delete devicesPanel;
        devicesPanel = nullptr;
        ui->uiMain->fill_menu(ui->uiMenuBar);
      },
      ui);
}

DevicesPanel::~DevicesPanel() {}

void DevicesPanel::add_controls() {
  TLRENDER_P();
  TLRENDER_R();

  Fl_Box *box;
  Pack *sg;
  Fl_Group *bg;
  CollapsibleGroup *cg;
  Fl_Button *b;
  Fl_Choice *m;

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

  auto mW = new Widget<Fl_Choice>(g->x() + 130, Y, g->w() - g->x() - 130, 20,
                                  "Name:");
  m = r.deviceComboBox = mW;
  m->labelsize(12);
  m->align(FL_ALIGN_LEFT);
  Y += 20;
  mW->callback(
      [=](auto o) { _p->ui->app->devicesModel()->setDeviceIndex(o->value()); });

  mW = new Widget<Fl_Choice>(g->x() + 130, Y, g->w() - g->x() - 130, 20,
                             "Display mode:");
  m = r.displayModeComboBox = mW;
  m->labelsize(12);
  m->align(FL_ALIGN_LEFT);
  Y += 20;
  mW->callback([=](auto o) {
    _p->ui->app->devicesModel()->setDisplayModeIndex(o->value());
  });

  mW = new Widget<Fl_Choice>(g->x() + 130, Y, g->w() - g->x() - 130, 20,
                             "Pixel type:");
  m = r.pixelTypeComboBox = mW;
  m->labelsize(12);
  m->align(FL_ALIGN_LEFT);
  Y += 20;
  mW->callback([=](auto o) {
    _p->ui->app->devicesModel()->setPixelTypeIndex(o->value());
  });

  mW = new Widget<Fl_Choice>(g->x() + 130, Y, g->w() - g->x() - 130, 20,
                             "Video levels:");
  m = r.videoLevelsComboBox = mW;
  m->labelsize(12);
  m->align(FL_ALIGN_LEFT);
  Y += 20;
  mW->callback([=](auto o) {
    _p->ui->app->devicesModel()->setVideoLevels(
        static_cast<imaging::VideoLevels>(o->value()));
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

  mW = new Widget<Fl_Choice>(g->x() + 130, Y, g->w() - g->x() - 130, 20,
                             "Mode:");
  m = r.hdrModeComboBox = mW;
  m->labelsize(12);
  m->align(FL_ALIGN_LEFT);
  Y += 25;

  mW->callback([=](auto o) {
    p.ui->app->devicesModel()->setHDRMode(
        static_cast<device::HDRMode>(o->value()));
  });

  bg->end();

  box = new Fl_Box(X, Y, g->w(), 20);
  Y += 20;

  sg = new Pack(g->x(), Y, g->w(), 25);
  sg->type(Pack::HORIZONTAL);
  sg->spacing(5);
  sg->begin();

  box = new Fl_Box(X, Y, 120, 20, "Red Primaries:");
  box->labelsize(12);
  box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

  auto dW = new Widget<DoubleSpinner>(X, Y, 50, 25);
  r.redPrimariesSpinBoxes.first = dW;
  dW->callback([=](auto o) {
    auto hdrData = _p->ui->app->devicesModel()->observeData()->get().hdrData;
    hdrData.redPrimaries.x = o->value();
    _p->ui->app->devicesModel()->setHDRData(hdrData);
  });
  dW = new Widget<DoubleSpinner>(X, Y, 50, 25);
  r.redPrimariesSpinBoxes.second = dW;
  dW->callback([=](auto o) {
    auto hdrData = _p->ui->app->devicesModel()->observeData()->get().hdrData;
    hdrData.redPrimaries.y = o->value();
    _p->ui->app->devicesModel()->setHDRData(hdrData);
  });

  sg->end();
  Y += 25;

  sg = new Pack(g->x(), Y, g->w(), 25);
  sg->type(Pack::HORIZONTAL);
  sg->spacing(5);
  sg->begin();

  box = new Fl_Box(X, Y, 120, 20, "Green Primaries:");
  box->labelsize(12);
  box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

  dW = new Widget<DoubleSpinner>(X, Y, 50, 25);
  r.greenPrimariesSpinBoxes.first = dW;
  dW->callback([=](auto o) {
    auto hdrData = _p->ui->app->devicesModel()->observeData()->get().hdrData;
    hdrData.greenPrimaries.x = o->value();
    _p->ui->app->devicesModel()->setHDRData(hdrData);
  });
  dW = new Widget<DoubleSpinner>(X, Y, 50, 25);
  r.greenPrimariesSpinBoxes.second = dW;
  dW->callback([=](auto o) {
    auto hdrData = _p->ui->app->devicesModel()->observeData()->get().hdrData;
    hdrData.greenPrimaries.y = o->value();
    _p->ui->app->devicesModel()->setHDRData(hdrData);
  });

  sg->end();
  Y += 25;

  sg = new Pack(g->x(), Y, g->w(), 25);
  sg->type(Pack::HORIZONTAL);
  sg->spacing(5);
  sg->begin();

  box = new Fl_Box(X, Y, 120, 20, "Blue Primaries:");
  box->labelsize(12);
  box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

  dW = new Widget<DoubleSpinner>(X, Y, 50, 25);
  r.bluePrimariesSpinBoxes.first = dW;
  dW->callback([=](auto o) {
    auto hdrData = _p->ui->app->devicesModel()->observeData()->get().hdrData;
    hdrData.bluePrimaries.x = o->value();
    _p->ui->app->devicesModel()->setHDRData(hdrData);
  });
  dW = new Widget<DoubleSpinner>(X, Y, 50, 25);
  r.bluePrimariesSpinBoxes.second = dW;
  dW->callback([=](auto o) {
    auto hdrData = _p->ui->app->devicesModel()->observeData()->get().hdrData;
    hdrData.bluePrimaries.y = o->value();
    _p->ui->app->devicesModel()->setHDRData(hdrData);
  });

  sg->end();
  Y += 25;

  sg = new Pack(g->x(), Y, g->w(), 25);
  sg->type(Pack::HORIZONTAL);
  sg->spacing(5);
  sg->begin();

  box = new Fl_Box(X, Y, 120, 20, "White Primaries:");
  box->labelsize(12);
  box->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

  dW = new Widget<DoubleSpinner>(X, Y, 50, 25);
  r.whitePrimariesSpinBoxes.first = dW;
  dW->callback([=](auto o) {
    auto hdrData = _p->ui->app->devicesModel()->observeData()->get().hdrData;
    hdrData.whitePrimaries.x = o->value();
    _p->ui->app->devicesModel()->setHDRData(hdrData);
  });
  dW = new Widget<DoubleSpinner>(X, Y, 50, 25);
  r.whitePrimariesSpinBoxes.second = dW;
  dW->callback([=](auto o) {
    auto hdrData = _p->ui->app->devicesModel()->observeData()->get().hdrData;
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

  dW = new Widget<DoubleSpinner>(X, Y, 50, 20);
  r.masteringLuminanceSpinBoxes.first = dW;
  r.masteringLuminanceSpinBoxes.first->range(0.0, 10000.0);
  X += r.masteringLuminanceSpinBoxes.first->w() + 5;
  dW->callback([=](auto o) {
    auto hdrData = _p->ui->app->devicesModel()->observeData()->get().hdrData;
    hdrData.displayMasteringLuminance = math::FloatRange(
        o->value(), hdrData.displayMasteringLuminance.getMax());
    _p->ui->app->devicesModel()->setHDRData(hdrData);
  });

  dW = new Widget<DoubleSpinner>(X, Y, 70, 20);
  r.masteringLuminanceSpinBoxes.second = dW;
  r.masteringLuminanceSpinBoxes.second->range(0.0, 10000.0);
  dW->callback([=](auto o) {
    auto hdrData = _p->ui->app->devicesModel()->observeData()->get().hdrData;
    hdrData.displayMasteringLuminance = math::FloatRange(
        hdrData.displayMasteringLuminance.getMin(), o->value());
    _p->ui->app->devicesModel()->setHDRData(hdrData);
  });

  sg->end();

  HorSlider *s;
  auto sW = new Widget<HorSlider>(g->x(), Y, g->w(), 20, "Maximum CLL:");
  s = _r->maxCLLSlider = sW;
  s->range(0.F, 10000.F);
  s->step(0.1);
  sW->callback([=](auto o) {
    auto hdrData = _p->ui->app->devicesModel()->observeData()->get().hdrData;
    hdrData.maxCLL = o->value();
    _p->ui->app->devicesModel()->setHDRData(hdrData);
  });
  Y += s->h();

  sW = new Widget<HorSlider>(g->x(), Y, g->w(), 20, "Maximum FALL:");
  s = _r->maxFALLSlider = sW;
  s->range(0.F, 10000.F);
  s->step(0.1);
  Y += s->h();
  sW->callback([=](auto o) {
    auto hdrData = _p->ui->app->devicesModel()->observeData()->get().hdrData;
    hdrData.maxFALL = o->value();
    _p->ui->app->devicesModel()->setHDRData(hdrData);
  });

  cg->end();

  g->end();

  r.dataObserver = observer::ValueObserver<DevicesModelData>::create(
      p.ui->app->devicesModel()->observeData(),
      [this](const DevicesModelData &value) {
        {
          _r->deviceComboBox->clear();
          for (const auto &i : value.devices) {
            _r->deviceComboBox->add(i.c_str());
          }
          _r->deviceComboBox->value(value.deviceIndex);
        }
        {
          _r->displayModeComboBox->clear();
          for (const auto &i : value.displayModes) {
            _r->displayModeComboBox->add(i.c_str());
          }
          _r->displayModeComboBox->value(value.displayModeIndex);
        }
        {
          _r->pixelTypeComboBox->clear();
          for (const auto &i : value.pixelTypes) {
            std::stringstream ss;
            ss << i;
            _r->pixelTypeComboBox->add(ss.str().c_str());
          }
          _r->pixelTypeComboBox->value(value.pixelTypeIndex);
        }
        {
          _r->videoLevelsComboBox->clear();
          for (const auto &i : imaging::getVideoLevelsEnums()) {
            std::stringstream ss;
            ss << i;
            _r->videoLevelsComboBox->add(ss.str().c_str());
          }
          _r->videoLevelsComboBox->value(static_cast<int>(value.videoLevels));
        }
        {
          _r->hdrModeComboBox->clear();
          for (const auto &i : device::getHDRModeLabels()) {
            _r->hdrModeComboBox->add(i.c_str());
          }
          _r->hdrModeComboBox->value(static_cast<int>(value.hdrMode));
        }
        {
          _r->redPrimariesSpinBoxes.first->value(value.hdrData.redPrimaries.x);
          _r->redPrimariesSpinBoxes.first->setEnabled(device::HDRMode::Custom ==
                                                      value.hdrMode);
        }
        {
          _r->redPrimariesSpinBoxes.second->value(value.hdrData.redPrimaries.y);
          _r->redPrimariesSpinBoxes.second->setEnabled(
              device::HDRMode::Custom == value.hdrMode);
        }
        {
          _r->greenPrimariesSpinBoxes.first->value(
              value.hdrData.greenPrimaries.x);
          _r->greenPrimariesSpinBoxes.first->setEnabled(
              device::HDRMode::Custom == value.hdrMode);
        }
        {
          _r->greenPrimariesSpinBoxes.second->value(
              value.hdrData.greenPrimaries.y);
          _r->greenPrimariesSpinBoxes.second->setEnabled(
              device::HDRMode::Custom == value.hdrMode);
        }
        {
          _r->bluePrimariesSpinBoxes.first->value(
              value.hdrData.bluePrimaries.x);
          _r->bluePrimariesSpinBoxes.first->setEnabled(
              device::HDRMode::Custom == value.hdrMode);
        }
        {
          _r->bluePrimariesSpinBoxes.second->value(
              value.hdrData.bluePrimaries.y);
          _r->bluePrimariesSpinBoxes.second->setEnabled(
              device::HDRMode::Custom == value.hdrMode);
        }
        {
          _r->whitePrimariesSpinBoxes.first->value(
              value.hdrData.whitePrimaries.x);
          _r->whitePrimariesSpinBoxes.first->setEnabled(
              device::HDRMode::Custom == value.hdrMode);
        }
        {
          _r->whitePrimariesSpinBoxes.second->value(
              value.hdrData.whitePrimaries.y);
          _r->whitePrimariesSpinBoxes.second->setEnabled(
              device::HDRMode::Custom == value.hdrMode);
        }
        {
          _r->masteringLuminanceSpinBoxes.first->value(
              value.hdrData.displayMasteringLuminance.getMin());
          _r->masteringLuminanceSpinBoxes.first->setEnabled(
              device::HDRMode::Custom == value.hdrMode);
        }
        {
          _r->masteringLuminanceSpinBoxes.second->value(
              value.hdrData.displayMasteringLuminance.getMax());
          _r->masteringLuminanceSpinBoxes.second->setEnabled(
              device::HDRMode::Custom == value.hdrMode);
        }
        {
          _r->maxCLLSlider->value(value.hdrData.maxCLL);
          _r->maxCLLSlider->setEnabled(device::HDRMode::Custom ==
                                       value.hdrMode);
        }
        {
          _r->maxFALLSlider->value(value.hdrData.maxFALL);
          _r->maxFALLSlider->setEnabled(device::HDRMode::Custom ==
                                        value.hdrMode);
        }
      });
}

} // namespace mrv
