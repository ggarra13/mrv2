// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/DevicesTool.h>

#include <tlPlayApp/App.h>

#include <tlUI/Bellows.h>
#include <tlUI/CheckBox.h>
#include <tlUI/ComboBox.h>
#include <tlUI/FloatEdit.h>
#include <tlUI/FloatEditSlider.h>
#include <tlUI/GridLayout.h>
#include <tlUI/Label.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollWidget.h>

#if defined(TLRENDER_BMD)
#    include <tlDevice/BMDDevicesModel.h>
#    include <tlDevice/BMDOutputDevice.h>
#endif // TLRENDER_BMD

namespace tl
{
    namespace play_app
    {
        struct DevicesTool::Private
        {
#if defined(TLRENDER_BMD)
            std::shared_ptr<ui::CheckBox> enabledCheckBox;
            std::shared_ptr<ui::ComboBox> deviceComboBox;
            std::shared_ptr<ui::ComboBox> displayModeComboBox;
            std::shared_ptr<ui::ComboBox> pixelTypeComboBox;
            std::shared_ptr<ui::CheckBox> _444SDIVideoOutputCheckBox;
            std::shared_ptr<ui::ComboBox> videoLevelsComboBox;
            std::shared_ptr<ui::ComboBox> hdrModeComboBox;
            std::vector<std::pair<
                std::shared_ptr<ui::FloatEdit>,
                std::shared_ptr<ui::FloatEdit> > >
                primariesFloatEdits;
            std::pair<
                std::shared_ptr<ui::FloatEdit>, std::shared_ptr<ui::FloatEdit> >
                masteringLuminanceFloatEdits;
            std::shared_ptr<ui::FloatEditSlider> maxCLLSlider;
            std::shared_ptr<ui::FloatEditSlider> maxFALLSlider;

            std::shared_ptr<observer::ValueObserver<bmd::DevicesModelData> >
                dataObserver;
#endif // TLRENDER_BMD
        };

        void DevicesTool::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::Devices, "tl::play_app::DevicesTool", app, context,
                parent);
            TLRENDER_P();

#if defined(TLRENDER_BMD)
            p.enabledCheckBox = ui::CheckBox::create(context);

            p.deviceComboBox = ui::ComboBox::create(context);
            p.displayModeComboBox = ui::ComboBox::create(context);
            p.pixelTypeComboBox = ui::ComboBox::create(context);

            p._444SDIVideoOutputCheckBox = ui::CheckBox::create(context);

            p.videoLevelsComboBox =
                ui::ComboBox::create(image::getVideoLevelsLabels(), context);

            p.hdrModeComboBox =
                ui::ComboBox::create(bmd::getHDRModeLabels(), context);

            for (size_t i = 0; i < image::HDRPrimaries::Count; ++i)
            {
                auto min = ui::FloatEdit::create(context);
                min->setRange(math::FloatRange(0.F, 1.F));
                min->setStep(.01F);
                min->setLargeStep(.1F);
                auto max = ui::FloatEdit::create(context);
                max->setRange(math::FloatRange(0.F, 1.F));
                max->setStep(.01F);
                max->setLargeStep(.1F);
                p.primariesFloatEdits.push_back(std::make_pair(min, max));
            }

            p.masteringLuminanceFloatEdits.first =
                ui::FloatEdit::create(context);
            p.masteringLuminanceFloatEdits.first->setRange(
                math::FloatRange(0.F, 10000.F));
            p.masteringLuminanceFloatEdits.second =
                ui::FloatEdit::create(context);
            p.masteringLuminanceFloatEdits.second->setRange(
                math::FloatRange(0.F, 10000.F));

            p.maxCLLSlider = ui::FloatEditSlider::create(context);
            p.maxCLLSlider->setRange(math::FloatRange(0.F, 10000.F));

            p.maxFALLSlider = ui::FloatEditSlider::create(context);
            p.maxFALLSlider->setRange(math::FloatRange(0.F, 10000.F));

            auto layout = ui::VerticalLayout::create(context);
            layout->setSpacingRole(ui::SizeRole::kNone);

            auto bellows = ui::Bellows::create("Output", context, layout);
            auto gridLayout = ui::GridLayout::create(context);
            gridLayout->setMarginRole(ui::SizeRole::MarginSmall);
            gridLayout->setSpacingRole(ui::SizeRole::SpacingSmall);
            auto label = ui::Label::create("Enabled:", context, gridLayout);
            gridLayout->setGridPos(label, 0, 0);
            p.enabledCheckBox->setParent(gridLayout);
            gridLayout->setGridPos(p.enabledCheckBox, 0, 1);
            label = ui::Label::create("Device:", context, gridLayout);
            gridLayout->setGridPos(label, 1, 0);
            p.deviceComboBox->setParent(gridLayout);
            gridLayout->setGridPos(p.deviceComboBox, 1, 1);
            label = ui::Label::create("Display mode:", context, gridLayout);
            gridLayout->setGridPos(label, 2, 0);
            p.displayModeComboBox->setParent(gridLayout);
            gridLayout->setGridPos(p.displayModeComboBox, 2, 1);
            label = ui::Label::create("Pixel type:", context, gridLayout);
            gridLayout->setGridPos(label, 3, 0);
            p.pixelTypeComboBox->setParent(gridLayout);
            gridLayout->setGridPos(p.pixelTypeComboBox, 3, 1);
            label =
                ui::Label::create("444 SDI video output:", context, gridLayout);
            gridLayout->setGridPos(label, 4, 0);
            p._444SDIVideoOutputCheckBox->setParent(gridLayout);
            gridLayout->setGridPos(p._444SDIVideoOutputCheckBox, 4, 1);
            label = ui::Label::create("Video levels:", context, gridLayout);
            gridLayout->setGridPos(label, 5, 0);
            p.videoLevelsComboBox->setParent(gridLayout);
            gridLayout->setGridPos(p.videoLevelsComboBox, 5, 1);
            bellows->setWidget(gridLayout);

            bellows = ui::Bellows::create("HDR", context, layout);
            gridLayout = ui::GridLayout::create(context);
            gridLayout->setMarginRole(ui::SizeRole::MarginSmall);
            gridLayout->setSpacingRole(ui::SizeRole::SpacingSmall);
            label = ui::Label::create("Mode:", context, gridLayout);
            gridLayout->setGridPos(label, 0, 0);
            p.hdrModeComboBox->setParent(gridLayout);
            gridLayout->setGridPos(p.hdrModeComboBox, 0, 1);
            const std::array<std::string, image::HDRPrimaries::Count>
                primariesLabels = {
                    "Red primaries:", "Green primaries:", "Blue primaries:",
                    "White primaries:"};
            for (size_t i = 0; i < image::HDRPrimaries::Count; ++i)
            {
                label =
                    ui::Label::create(primariesLabels[i], context, gridLayout);
                gridLayout->setGridPos(label, 1 + i, 0);
                auto hLayout =
                    ui::HorizontalLayout::create(context, gridLayout);
                hLayout->setSpacingRole(ui::SizeRole::SpacingSmall);
                p.primariesFloatEdits[i].first->setParent(hLayout);
                p.primariesFloatEdits[i].second->setParent(hLayout);
                gridLayout->setGridPos(hLayout, 1 + i, 1);
            }
            label =
                ui::Label::create("Mastering luminance:", context, gridLayout);
            gridLayout->setGridPos(label, 6, 0);
            auto hLayout = ui::HorizontalLayout::create(context, gridLayout);
            hLayout->setSpacingRole(ui::SizeRole::SpacingSmall);
            p.masteringLuminanceFloatEdits.first->setParent(hLayout);
            p.masteringLuminanceFloatEdits.second->setParent(hLayout);
            gridLayout->setGridPos(hLayout, 6, 1);
            label = ui::Label::create("Maximum CLL:", context, gridLayout);
            gridLayout->setGridPos(label, 7, 0);
            p.maxCLLSlider->setParent(gridLayout);
            gridLayout->setGridPos(p.maxCLLSlider, 7, 1);
            label = ui::Label::create("Maximum FALL:", context, gridLayout);
            gridLayout->setGridPos(label, 8, 0);
            p.maxFALLSlider->setParent(gridLayout);
            gridLayout->setGridPos(p.maxFALLSlider, 8, 1);
            bellows->setWidget(gridLayout);

            auto scrollWidget = ui::ScrollWidget::create(context);
            scrollWidget->setWidget(layout);
            _setWidget(scrollWidget);

            auto appWeak = std::weak_ptr<App>(app);
            p.enabledCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getBMDDevicesModel()->setDeviceEnabled(value);
                    }
                });

            p.deviceComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getBMDDevicesModel()->setDeviceIndex(value);
                    }
                });
            p.displayModeComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getBMDDevicesModel()->setDisplayModeIndex(value);
                    }
                });
            p.pixelTypeComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getBMDDevicesModel()->setPixelTypeIndex(value);
                    }
                });

            p._444SDIVideoOutputCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getBMDDevicesModel()
                                           ->observeData()
                                           ->get()
                                           .boolOptions;
                        options[bmd::Option::_444SDIVideoOutput] = value;
                        app->getBMDDevicesModel()->setBoolOptions(options);
                    }
                });

            p.videoLevelsComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getBMDDevicesModel()->setVideoLevels(
                            static_cast<image::VideoLevels>(value));
                    }
                });

            p.hdrModeComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getBMDDevicesModel()->setHDRMode(
                            static_cast<bmd::HDRMode>(value));
                    }
                });

            for (size_t i = 0; i < image::HDRPrimaries::Count; ++i)
            {
                p.primariesFloatEdits[i].first->setCallback(
                    [appWeak, i](float value)
                    {
                        if (auto app = appWeak.lock())
                        {
                            auto hdrData = app->getBMDDevicesModel()
                                               ->observeData()
                                               ->get()
                                               .hdrData;
                            hdrData.primaries[i].x = value;
                            app->getBMDDevicesModel()->setHDRData(hdrData);
                        }
                    });
                p.primariesFloatEdits[i].second->setCallback(
                    [appWeak, i](float value)
                    {
                        if (auto app = appWeak.lock())
                        {
                            auto hdrData = app->getBMDDevicesModel()
                                               ->observeData()
                                               ->get()
                                               .hdrData;
                            hdrData.primaries[i].y = value;
                            app->getBMDDevicesModel()->setHDRData(hdrData);
                        }
                    });
            }

            p.masteringLuminanceFloatEdits.first->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto hdrData = app->getBMDDevicesModel()
                                           ->observeData()
                                           ->get()
                                           .hdrData;
                        hdrData.displayMasteringLuminance = math::FloatRange(
                            value, hdrData.displayMasteringLuminance.getMax());
                        app->getBMDDevicesModel()->setHDRData(hdrData);
                    }
                });
            p.masteringLuminanceFloatEdits.second->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto hdrData = app->getBMDDevicesModel()
                                           ->observeData()
                                           ->get()
                                           .hdrData;
                        hdrData.displayMasteringLuminance = math::FloatRange(
                            hdrData.displayMasteringLuminance.getMin(), value);
                        app->getBMDDevicesModel()->setHDRData(hdrData);
                    }
                });

            p.maxCLLSlider->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto hdrData = app->getBMDDevicesModel()
                                           ->observeData()
                                           ->get()
                                           .hdrData;
                        hdrData.maxCLL = value;
                        app->getBMDDevicesModel()->setHDRData(hdrData);
                    }
                });
            p.maxFALLSlider->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto hdrData = app->getBMDDevicesModel()
                                           ->observeData()
                                           ->get()
                                           .hdrData;
                        hdrData.maxFALL = value;
                        app->getBMDDevicesModel()->setHDRData(hdrData);
                    }
                });

            p.dataObserver =
                observer::ValueObserver<bmd::DevicesModelData>::create(
                    app->getBMDDevicesModel()->observeData(),
                    [this](const bmd::DevicesModelData& value)
                    {
                        TLRENDER_P();
                        p.enabledCheckBox->setChecked(value.deviceEnabled);

                        p.deviceComboBox->setItems(value.devices);
                        p.deviceComboBox->setCurrentIndex(value.deviceIndex);
                        p.displayModeComboBox->setItems(value.displayModes);
                        p.displayModeComboBox->setCurrentIndex(
                            value.displayModeIndex);
                        std::vector<std::string> pixelTypes;
                        for (const auto& pixelType : value.pixelTypes)
                        {
                            std::stringstream ss;
                            ss << pixelType;
                            pixelTypes.push_back(ss.str());
                        }
                        p.pixelTypeComboBox->setItems(pixelTypes);
                        p.pixelTypeComboBox->setCurrentIndex(
                            value.pixelTypeIndex);

                        const auto i = value.boolOptions.find(
                            bmd::Option::_444SDIVideoOutput);
                        p._444SDIVideoOutputCheckBox->setChecked(
                            i != value.boolOptions.end() ? i->second : false);

                        p.videoLevelsComboBox->setCurrentIndex(
                            static_cast<int>(value.videoLevels));

                        p.hdrModeComboBox->setCurrentIndex(
                            static_cast<int>(value.hdrMode));

                        for (size_t i = 0; i < image::HDRPrimaries::Count; ++i)
                        {
                            p.primariesFloatEdits[i].first->setValue(
                                value.hdrData.primaries[i].x);
                            p.primariesFloatEdits[i].first->setEnabled(
                                bmd::HDRMode::Custom == value.hdrMode);
                            p.primariesFloatEdits[i].second->setValue(
                                value.hdrData.primaries[i].y);
                            p.primariesFloatEdits[i].second->setEnabled(
                                bmd::HDRMode::Custom == value.hdrMode);
                        }

                        p.masteringLuminanceFloatEdits.first->setValue(
                            value.hdrData.displayMasteringLuminance.getMin());
                        p.masteringLuminanceFloatEdits.first->setEnabled(
                            bmd::HDRMode::Custom == value.hdrMode);
                        p.masteringLuminanceFloatEdits.second->setValue(
                            value.hdrData.displayMasteringLuminance.getMax());
                        p.masteringLuminanceFloatEdits.second->setEnabled(
                            bmd::HDRMode::Custom == value.hdrMode);

                        p.maxCLLSlider->setValue(value.hdrData.maxCLL);
                        p.maxCLLSlider->setEnabled(
                            bmd::HDRMode::Custom == value.hdrMode);
                        p.maxFALLSlider->setValue(value.hdrData.maxFALL);
                        p.maxFALLSlider->setEnabled(
                            bmd::HDRMode::Custom == value.hdrMode);
                    });
#endif // TLRENDER_BMD
        }

        DevicesTool::DevicesTool() :
            _p(new Private)
        {
        }

        DevicesTool::~DevicesTool() {}

        std::shared_ptr<DevicesTool> DevicesTool::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<DevicesTool>(new DevicesTool);
            out->_init(app, context, parent);
            return out;
        }
    } // namespace play_app
} // namespace tl
