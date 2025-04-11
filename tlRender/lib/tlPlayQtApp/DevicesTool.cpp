// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/DevicesTool.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/DockTitleBar.h>

#include <tlQtWidget/FloatEditSlider.h>

#if defined(TLRENDER_BMD)
#    include <tlDevice/BMDDevicesModel.h>
#    include <tlDevice/BMDOutputDevice.h>
#endif // TLRENDER_BMD

#include <QAction>
#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QSignalBlocker>

#include <sstream>

namespace tl
{
    namespace play_qt
    {
        struct DevicesTool::Private
        {
            App* app = nullptr;

#if defined(TLRENDER_BMD)
            QCheckBox* enabledCheckBox = nullptr;
            QComboBox* deviceComboBox = nullptr;
            QComboBox* displayModeComboBox = nullptr;
            QComboBox* pixelTypeComboBox = nullptr;
            QCheckBox* _444SDIVideoOutputCheckBox = nullptr;
            QComboBox* videoLevelsComboBox = nullptr;
            QComboBox* hdrModeComboBox = nullptr;
            std::vector<std::pair<QDoubleSpinBox*, QDoubleSpinBox*> >
                primariesSpinBoxes;
            std::pair<QDoubleSpinBox*, QDoubleSpinBox*>
                masteringLuminanceSpinBoxes = std::make_pair(nullptr, nullptr);
            qtwidget::FloatEditSlider* maxCLLSlider = nullptr;
            qtwidget::FloatEditSlider* maxFALLSlider = nullptr;

            std::shared_ptr<observer::ValueObserver<bmd::DevicesModelData> >
                dataObserver;
#endif // TLRENDER_BMD
        };

        DevicesTool::DevicesTool(App* app, QWidget* parent) :
            IToolWidget(app, parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.app = app;

#if defined(TLRENDER_BMD)
            p.enabledCheckBox = new QCheckBox(tr("Enabled"));

            p.deviceComboBox = new QComboBox;
            p.displayModeComboBox = new QComboBox;
            p.pixelTypeComboBox = new QComboBox;

            p._444SDIVideoOutputCheckBox =
                new QCheckBox(tr("444 SDI video output"));

            p.videoLevelsComboBox = new QComboBox;

            p.hdrModeComboBox = new QComboBox;

            for (size_t i = 0; i < image::HDRPrimaries::Count; ++i)
            {
                auto min = new QDoubleSpinBox;
                min->setRange(0.0, 1.0);
                min->setSingleStep(0.01);
                auto max = new QDoubleSpinBox;
                max->setRange(0.0, 1.0);
                max->setSingleStep(0.01);
                p.primariesSpinBoxes.push_back(std::make_pair(min, max));
            }

            p.masteringLuminanceSpinBoxes.first = new QDoubleSpinBox;
            p.masteringLuminanceSpinBoxes.first->setRange(0.0, 10000.0);
            p.masteringLuminanceSpinBoxes.second = new QDoubleSpinBox;
            p.masteringLuminanceSpinBoxes.second->setRange(0.0, 10000.0);

            p.maxCLLSlider = new qtwidget::FloatEditSlider;
            p.maxCLLSlider->setRange(math::FloatRange(0.F, 10000.F));

            p.maxFALLSlider = new qtwidget::FloatEditSlider;
            p.maxFALLSlider->setRange(math::FloatRange(0.F, 10000.F));

            auto layout = new QFormLayout;
            layout->addRow(p.enabledCheckBox);
            layout->addRow(tr("Device:"), p.deviceComboBox);
            layout->addRow(tr("Display mode:"), p.displayModeComboBox);
            layout->addRow(tr("Pixel type:"), p.pixelTypeComboBox);
            layout->addRow(p._444SDIVideoOutputCheckBox);
            layout->addRow(tr("Video levels:"), p.videoLevelsComboBox);
            auto widget = new QWidget;
            widget->setLayout(layout);
            addBellows(tr("Output"), widget);

            layout = new QFormLayout;
            layout->addRow(tr("Mode:"), p.hdrModeComboBox);
            const std::array<QString, image::HDRPrimaries::Count>
                primariesLabels = {
                    tr("Red primaries:"), tr("Green primaries:"),
                    tr("Blue primaries:"), tr("White primaries:")};
            for (size_t i = 0; i < image::HDRPrimaries::Count; ++i)
            {
                auto hLayout = new QHBoxLayout;
                hLayout->addWidget(p.primariesSpinBoxes[i].first);
                hLayout->addWidget(p.primariesSpinBoxes[i].second);
                layout->addRow(primariesLabels[i], hLayout);
            }
            auto hLayout = new QHBoxLayout;
            hLayout->addWidget(p.masteringLuminanceSpinBoxes.first);
            hLayout->addWidget(p.masteringLuminanceSpinBoxes.second);
            layout->addRow(tr("Mastering luminance:"), hLayout);
            layout->addRow(tr("Maximum CLL:"), p.maxCLLSlider);
            layout->addRow(tr("Maximum FALL:"), p.maxFALLSlider);
            widget = new QWidget;
            widget->setLayout(layout);
            addBellows(tr("HDR"), widget);

            addStretch();

            connect(
                p.enabledCheckBox, &QCheckBox::toggled, [this](bool value)
                { _p->app->bmdDevicesModel()->setDeviceEnabled(value); });

            connect(
                p.deviceComboBox, QOverload<int>::of(&QComboBox::activated),
                [this](int value)
                { _p->app->bmdDevicesModel()->setDeviceIndex(value); });
            connect(
                p.displayModeComboBox,
                QOverload<int>::of(&QComboBox::activated), [this](int value)
                { _p->app->bmdDevicesModel()->setDisplayModeIndex(value); });
            connect(
                p.pixelTypeComboBox, QOverload<int>::of(&QComboBox::activated),
                [this](int value)
                { _p->app->bmdDevicesModel()->setPixelTypeIndex(value); });

            connect(
                p._444SDIVideoOutputCheckBox, &QCheckBox::toggled,
                [this](bool value)
                {
                    auto options = _p->app->bmdDevicesModel()
                                       ->observeData()
                                       ->get()
                                       .boolOptions;
                    options[bmd::Option::_444SDIVideoOutput] = value;
                    _p->app->bmdDevicesModel()->setBoolOptions(options);
                });

            connect(
                p.videoLevelsComboBox,
                QOverload<int>::of(&QComboBox::activated),
                [this](int value)
                {
                    _p->app->bmdDevicesModel()->setVideoLevels(
                        static_cast<image::VideoLevels>(value));
                });

            connect(
                p.hdrModeComboBox, QOverload<int>::of(&QComboBox::activated),
                [this](int value) {
                    _p->app->bmdDevicesModel()->setHDRMode(
                        static_cast<bmd::HDRMode>(value));
                });

            for (size_t i = 0; i < image::HDRPrimaries::Count; ++i)
            {
                connect(
                    p.primariesSpinBoxes[i].first,
                    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                    [this, i](double value)
                    {
                        auto hdrData = _p->app->bmdDevicesModel()
                                           ->observeData()
                                           ->get()
                                           .hdrData;
                        hdrData.primaries[i].x = value;
                        _p->app->bmdDevicesModel()->setHDRData(hdrData);
                    });
                connect(
                    p.primariesSpinBoxes[i].second,
                    QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                    [this, i](double value)
                    {
                        auto hdrData = _p->app->bmdDevicesModel()
                                           ->observeData()
                                           ->get()
                                           .hdrData;
                        hdrData.primaries[i].y = value;
                        _p->app->bmdDevicesModel()->setHDRData(hdrData);
                    });
            }

            connect(
                p.masteringLuminanceSpinBoxes.first,
                QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [this](double value)
                {
                    auto hdrData = _p->app->bmdDevicesModel()
                                       ->observeData()
                                       ->get()
                                       .hdrData;
                    hdrData.displayMasteringLuminance = math::FloatRange(
                        value, hdrData.displayMasteringLuminance.getMax());
                    _p->app->bmdDevicesModel()->setHDRData(hdrData);
                });
            connect(
                p.masteringLuminanceSpinBoxes.second,
                QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                [this](double value)
                {
                    auto hdrData = _p->app->bmdDevicesModel()
                                       ->observeData()
                                       ->get()
                                       .hdrData;
                    hdrData.displayMasteringLuminance = math::FloatRange(
                        hdrData.displayMasteringLuminance.getMin(), value);
                    _p->app->bmdDevicesModel()->setHDRData(hdrData);
                });

            connect(
                p.maxCLLSlider, &qtwidget::FloatEditSlider::valueChanged,
                [this](float value)
                {
                    auto hdrData = _p->app->bmdDevicesModel()
                                       ->observeData()
                                       ->get()
                                       .hdrData;
                    hdrData.maxCLL = value;
                    _p->app->bmdDevicesModel()->setHDRData(hdrData);
                });
            connect(
                p.maxFALLSlider, &qtwidget::FloatEditSlider::valueChanged,
                [this](float value)
                {
                    auto hdrData = _p->app->bmdDevicesModel()
                                       ->observeData()
                                       ->get()
                                       .hdrData;
                    hdrData.maxFALL = value;
                    _p->app->bmdDevicesModel()->setHDRData(hdrData);
                });

            p.dataObserver =
                observer::ValueObserver<bmd::DevicesModelData>::create(
                    app->bmdDevicesModel()->observeData(),
                    [this](const bmd::DevicesModelData& value)
                    {
                        TLRENDER_P();
                        {
                            QSignalBlocker blocker(p.enabledCheckBox);
                            p.enabledCheckBox->setChecked(value.deviceEnabled);
                        }
                        {
                            QSignalBlocker blocker(p.deviceComboBox);
                            p.deviceComboBox->clear();
                            for (const auto& i : value.devices)
                            {
                                p.deviceComboBox->addItem(
                                    QString::fromUtf8(i.c_str()));
                            }
                            p.deviceComboBox->setCurrentIndex(
                                value.deviceIndex);
                        }
                        {
                            QSignalBlocker blocker(p.displayModeComboBox);
                            p.displayModeComboBox->clear();
                            for (const auto& i : value.displayModes)
                            {
                                p.displayModeComboBox->addItem(
                                    QString::fromUtf8(i.c_str()));
                            }
                            p.displayModeComboBox->setCurrentIndex(
                                value.displayModeIndex);
                        }
                        {
                            QSignalBlocker blocker(p.pixelTypeComboBox);
                            p.pixelTypeComboBox->clear();
                            for (const auto& i : value.pixelTypes)
                            {
                                std::stringstream ss;
                                ss << i;
                                p.pixelTypeComboBox->addItem(
                                    QString::fromUtf8(ss.str().c_str()));
                            }
                            p.pixelTypeComboBox->setCurrentIndex(
                                value.pixelTypeIndex);
                        }
                        {
                            QSignalBlocker blocker(
                                p._444SDIVideoOutputCheckBox);
                            const auto i = value.boolOptions.find(
                                bmd::Option::_444SDIVideoOutput);
                            p._444SDIVideoOutputCheckBox->setChecked(
                                i != value.boolOptions.end() ? i->second
                                                             : false);
                        }
                        {
                            QSignalBlocker blocker(p.videoLevelsComboBox);
                            p.videoLevelsComboBox->clear();
                            for (const auto& i : image::getVideoLevelsEnums())
                            {
                                std::stringstream ss;
                                ss << i;
                                p.videoLevelsComboBox->addItem(
                                    QString::fromUtf8(ss.str().c_str()));
                            }
                            p.videoLevelsComboBox->setCurrentIndex(
                                static_cast<int>(value.videoLevels));
                        }
                        {
                            QSignalBlocker blocker(p.hdrModeComboBox);
                            p.hdrModeComboBox->clear();
                            for (const auto& i : bmd::getHDRModeLabels())
                            {
                                p.hdrModeComboBox->addItem(
                                    QString::fromUtf8(i.c_str()));
                            }
                            p.hdrModeComboBox->setCurrentIndex(
                                static_cast<int>(value.hdrMode));
                        }
                        for (size_t i = 0; i < image::HDRPrimaries::Count; ++i)
                        {
                            {
                                QSignalBlocker blocker(
                                    p.primariesSpinBoxes[i].first);
                                p.primariesSpinBoxes[i].first->setValue(
                                    value.hdrData.primaries[i].x);
                                p.primariesSpinBoxes[i].first->setEnabled(
                                    bmd::HDRMode::Custom == value.hdrMode);
                            }
                            {
                                QSignalBlocker blocker(
                                    p.primariesSpinBoxes[i].second);
                                p.primariesSpinBoxes[i].second->setValue(
                                    value.hdrData.primaries[i].y);
                                p.primariesSpinBoxes[i].second->setEnabled(
                                    bmd::HDRMode::Custom == value.hdrMode);
                            }
                        }
                        {
                            QSignalBlocker blocker(
                                p.masteringLuminanceSpinBoxes.first);
                            p.masteringLuminanceSpinBoxes.first->setValue(
                                value.hdrData.displayMasteringLuminance
                                    .getMin());
                            p.masteringLuminanceSpinBoxes.first->setEnabled(
                                bmd::HDRMode::Custom == value.hdrMode);
                        }
                        {
                            QSignalBlocker blocker(
                                p.masteringLuminanceSpinBoxes.second);
                            p.masteringLuminanceSpinBoxes.second->setValue(
                                value.hdrData.displayMasteringLuminance
                                    .getMax());
                            p.masteringLuminanceSpinBoxes.second->setEnabled(
                                bmd::HDRMode::Custom == value.hdrMode);
                        }
                        {
                            QSignalBlocker blocker(p.maxCLLSlider);
                            p.maxCLLSlider->setValue(value.hdrData.maxCLL);
                            p.maxCLLSlider->setEnabled(
                                bmd::HDRMode::Custom == value.hdrMode);
                        }
                        {
                            QSignalBlocker blocker(p.maxFALLSlider);
                            p.maxFALLSlider->setValue(value.hdrData.maxFALL);
                            p.maxFALLSlider->setEnabled(
                                bmd::HDRMode::Custom == value.hdrMode);
                        }
                    });
#endif // TLRENDER_BMD
        }

        DevicesTool::~DevicesTool() {}

        DevicesDockWidget::DevicesDockWidget(
            DevicesTool* devicesTool, QWidget* parent)
        {
            setObjectName("DevicesTool");
            setWindowTitle(tr("Devices"));
            setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

            auto dockTitleBar = new DockTitleBar;
            dockTitleBar->setText(tr("Devices"));
            dockTitleBar->setIcon(QIcon(":/Icons/Devices.svg"));
            auto dockWidget = new QDockWidget;
            setTitleBarWidget(dockTitleBar);

            setWidget(devicesTool);

            toggleViewAction()->setIcon(QIcon(":/Icons/Devices.svg"));
            toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F6));
            toggleViewAction()->setToolTip(tr("Show devices"));
        }
    } // namespace play_qt
} // namespace tl
