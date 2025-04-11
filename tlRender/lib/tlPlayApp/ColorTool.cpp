// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/ColorToolPrivate.h>

#include <tlPlayApp/App.h>

#include <tlPlay/ColorModel.h>
#include <tlPlay/ViewportModel.h>

#include <tlUI/Bellows.h>
#include <tlUI/ButtonGroup.h>
#include <tlUI/CheckBox.h>
#include <tlUI/ComboBox.h>
#include <tlUI/FileEdit.h>
#include <tlUI/FloatEditSlider.h>
#include <tlUI/GridLayout.h>
#include <tlUI/Label.h>
#include <tlUI/ListWidget.h>
#include <tlUI/RowLayout.h>
#include <tlUI/SearchBox.h>
#include <tlUI/TabBar.h>
#include <tlUI/ScrollWidget.h>
#include <tlUI/StackLayout.h>

namespace tl
{
    namespace play_app
    {
        struct OCIOWidget::Private
        {
            std::shared_ptr<play::OCIOModel> ocioModel;

            std::shared_ptr<ui::CheckBox> enabledCheckBox;
            std::shared_ptr<ui::FileEdit> fileEdit;
            std::shared_ptr<ui::TabBar> tabBar;
            std::map<std::string, std::shared_ptr<ui::ListWidget> > listWidgets;
            std::map<std::string, std::shared_ptr<ui::SearchBox> > searchBoxes;
            std::shared_ptr<ui::StackLayout> tabLayout;
            std::shared_ptr<ui::VerticalLayout> layout;

            std::shared_ptr<observer::ValueObserver<timeline::OCIOOptions> >
                optionsObserver;
            std::shared_ptr<observer::ValueObserver<timeline::OCIOOptions> >
                optionsObserver2;
            std::shared_ptr<observer::ValueObserver<play::OCIOModelData> >
                dataObserver;
        };

        void OCIOWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<ui::IWidget>& parent)
        {
            ui::IWidget::_init("tl::play_app::OCIOWidget", context, parent);
            TLRENDER_P();

#if !defined(TLRENDER_OCIO)
            setEnabled(false);
#endif // TLRENDER_OCIO

            p.ocioModel = play::OCIOModel::create(context);

            p.enabledCheckBox = ui::CheckBox::create("Enabled", context);

            p.fileEdit = ui::FileEdit::create(context);

            p.tabBar = ui::TabBar::create(context);
            const std::vector<std::string> tabs = {
                "Input", "Display", "View", "Look"};
            p.tabBar->setTabs(tabs);
            for (const auto& tab : tabs)
            {
                p.listWidgets[tab] =
                    ui::ListWidget::create(ui::ButtonGroupType::Radio, context);
                p.searchBoxes[tab] = ui::SearchBox::create(context);
                p.searchBoxes[tab]->setHStretch(ui::Stretch::Expanding);
            }

            p.layout = ui::VerticalLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            auto gridLayout = ui::GridLayout::create(context, p.layout);
            gridLayout->setSpacingRole(ui::SizeRole::SpacingSmall);
            p.enabledCheckBox->setParent(gridLayout);
            gridLayout->setGridPos(p.enabledCheckBox, 0, 0);
            auto label = ui::Label::create("File name:", context, gridLayout);
            gridLayout->setGridPos(label, 1, 0);
            p.fileEdit->setParent(gridLayout);
            gridLayout->setGridPos(p.fileEdit, 1, 1);
            auto vLayout = ui::VerticalLayout::create(context, p.layout);
            vLayout->setSpacingRole(ui::SizeRole::None);
            p.tabBar->setParent(vLayout);
            p.tabLayout = ui::StackLayout::create(context, vLayout);
            for (const auto& tab : tabs)
            {
                vLayout = ui::VerticalLayout::create(context, p.tabLayout);
                vLayout->setSpacingRole(ui::SizeRole::SpacingSmall);
                p.listWidgets[tab]->setParent(vLayout);
                p.searchBoxes[tab]->setParent(vLayout);
            }

            p.optionsObserver =
                observer::ValueObserver<timeline::OCIOOptions>::create(
                    app->getColorModel()->observeOCIOOptions(),
                    [this](const timeline::OCIOOptions& value)
                    {
                        _p->enabledCheckBox->setChecked(value.enabled);
                        _p->fileEdit->setPath(file::Path(value.fileName));
                        _p->ocioModel->setOptions(value);
                    });

            auto appWeak = std::weak_ptr<App>(app);
            p.optionsObserver2 =
                observer::ValueObserver<timeline::OCIOOptions>::create(
                    p.ocioModel->observeOptions(),
                    [appWeak](const timeline::OCIOOptions& value)
                    {
                        if (auto app = appWeak.lock())
                        {
                            app->getColorModel()->setOCIOOptions(value);
                        }
                    });

            p.dataObserver =
                observer::ValueObserver<play::OCIOModelData>::create(
                    p.ocioModel->observeData(),
                    [this](const play::OCIOModelData& value)
                    {
                        _p->enabledCheckBox->setChecked(value.enabled);
                        _p->fileEdit->setPath(file::Path(value.fileName));
                        _p->listWidgets["Input"]->setItems(value.inputs);
                        _p->listWidgets["Input"]->setCurrentItem(
                            value.inputIndex);
                        _p->listWidgets["Display"]->setItems(value.displays);
                        _p->listWidgets["Display"]->setCurrentItem(
                            value.displayIndex);
                        _p->listWidgets["View"]->setItems(value.views);
                        _p->listWidgets["View"]->setCurrentItem(
                            value.viewIndex);
                        _p->listWidgets["Look"]->setItems(value.looks);
                        _p->listWidgets["Look"]->setCurrentItem(
                            value.lookIndex);
                    });

            p.tabBar->setCallback([this](int index)
                                  { _p->tabLayout->setCurrentIndex(index); });

            p.enabledCheckBox->setCheckedCallback(
                [this](bool value) { _p->ocioModel->setEnabled(value); });

            p.fileEdit->setCallback([this](const file::Path& value)
                                    { _p->ocioModel->setConfig(value.get()); });

            p.listWidgets["Input"]->setCallback(
                [this](int index) { _p->ocioModel->setInputIndex(index); });
            p.listWidgets["Display"]->setCallback(
                [this](int index) { _p->ocioModel->setDisplayIndex(index); });
            p.listWidgets["View"]->setCallback(
                [this](int index) { _p->ocioModel->setViewIndex(index); });
            p.listWidgets["Look"]->setCallback(
                [this](int index) { _p->ocioModel->setLookIndex(index); });

            p.searchBoxes["Input"]->setCallback(
                [this](const std::string& value)
                { _p->listWidgets["Input"]->setSearch(value); });
            p.searchBoxes["Display"]->setCallback(
                [this](const std::string& value)
                { _p->listWidgets["Display"]->setSearch(value); });
            p.searchBoxes["View"]->setCallback(
                [this](const std::string& value)
                { _p->listWidgets["View"]->setSearch(value); });
            p.searchBoxes["Look"]->setCallback(
                [this](const std::string& value)
                { _p->listWidgets["Look"]->setSearch(value); });
        }

        OCIOWidget::OCIOWidget() :
            _p(new Private)
        {
        }

        OCIOWidget::~OCIOWidget() {}

        std::shared_ptr<OCIOWidget> OCIOWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<OCIOWidget>(new OCIOWidget);
            out->_init(app, context, parent);
            return out;
        }

        void OCIOWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void OCIOWidget::sizeHintEvent(const ui::SizeHintEvent& value)
        {
            IWidget::sizeHintEvent(value);
            _sizeHint = _p->layout->getSizeHint();
        }

        struct LUTWidget::Private
        {
            std::shared_ptr<ui::CheckBox> enabledCheckBox;
            std::shared_ptr<ui::FileEdit> fileEdit;
            std::shared_ptr<ui::ComboBox> orderComboBox;
            std::shared_ptr<ui::GridLayout> layout;

            std::shared_ptr<observer::ValueObserver<timeline::LUTOptions> >
                optionsObservers;
        };

        void LUTWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<ui::IWidget>& parent)
        {
            ui::IWidget::_init("tl::play_app::LUTWidget", context, parent);
            TLRENDER_P();

#if !defined(TLRENDER_OCIO)
            setEnabled(false);
#endif // TLRENDER_OCIO

            p.enabledCheckBox = ui::CheckBox::create("Enabled", context);

            p.fileEdit = ui::FileEdit::create(context);

            p.orderComboBox =
                ui::ComboBox::create(timeline::getLUTOrderLabels(), context);

            p.layout = ui::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            p.enabledCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.enabledCheckBox, 0, 0);
            auto label = ui::Label::create("File name:", context, p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.fileEdit->setParent(p.layout);
            p.layout->setGridPos(p.fileEdit, 1, 1);
            label = ui::Label::create("Order:", context, p.layout);
            p.layout->setGridPos(label, 2, 0);
            p.orderComboBox->setParent(p.layout);
            p.layout->setGridPos(p.orderComboBox, 2, 1);

            p.optionsObservers =
                observer::ValueObserver<timeline::LUTOptions>::create(
                    app->getColorModel()->observeLUTOptions(),
                    [this](const timeline::LUTOptions& value)
                    {
                        _p->enabledCheckBox->setChecked(value.enabled);
                        _p->fileEdit->setPath(file::Path(value.fileName));
                        _p->orderComboBox->setCurrentIndex(
                            static_cast<size_t>(value.order));
                    });

            auto appWeak = std::weak_ptr<App>(app);
            p.enabledCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getColorModel()->getLUTOptions();
                        options.enabled = value;
                        app->getColorModel()->setLUTOptions(options);
                    }
                });

            p.fileEdit->setCallback(
                [appWeak](const file::Path& value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getColorModel()->getLUTOptions();
                        options.enabled = true;
                        options.fileName = value.get();
                        app->getColorModel()->setLUTOptions(options);
                    }
                });

            p.orderComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options = app->getColorModel()->getLUTOptions();
                        options.enabled = true;
                        options.order = static_cast<timeline::LUTOrder>(value);
                        app->getColorModel()->setLUTOptions(options);
                    }
                });
        }

        LUTWidget::LUTWidget() :
            _p(new Private)
        {
        }

        LUTWidget::~LUTWidget() {}

        std::shared_ptr<LUTWidget> LUTWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<LUTWidget>(new LUTWidget);
            out->_init(app, context, parent);
            return out;
        }

        void LUTWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void LUTWidget::sizeHintEvent(const ui::SizeHintEvent& value)
        {
            IWidget::sizeHintEvent(value);
            _sizeHint = _p->layout->getSizeHint();
        }

        struct ColorWidget::Private
        {
            std::shared_ptr<ui::CheckBox> enabledCheckBox;
            std::map<std::string, std::shared_ptr<ui::FloatEditSlider> >
                sliders;
            std::shared_ptr<ui::CheckBox> invertCheckBox;
            std::shared_ptr<ui::GridLayout> layout;

            std::shared_ptr<observer::ValueObserver<timeline::DisplayOptions> >
                optionsObservers;
        };

        void ColorWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<ui::IWidget>& parent)
        {
            ui::IWidget::_init("tl::play_app::ColorWidget", context, parent);
            TLRENDER_P();

            p.enabledCheckBox = ui::CheckBox::create("Enabled", context);

            p.sliders["Add"] = ui::FloatEditSlider::create(context);
            p.sliders["Add"]->setRange(math::FloatRange(-1.F, 1.F));
            p.sliders["Add"]->setDefaultValue(0.F);
            p.sliders["Brightness"] = ui::FloatEditSlider::create(context);
            p.sliders["Brightness"]->setRange(math::FloatRange(0.F, 4.F));
            p.sliders["Brightness"]->setDefaultValue(1.F);
            p.sliders["Contrast"] = ui::FloatEditSlider::create(context);
            p.sliders["Contrast"]->setRange(math::FloatRange(0.F, 4.F));
            p.sliders["Contrast"]->setDefaultValue(1.F);
            p.sliders["Saturation"] = ui::FloatEditSlider::create(context);
            p.sliders["Saturation"]->setRange(math::FloatRange(0.F, 4.F));
            p.sliders["Saturation"]->setDefaultValue(1.F);
            p.sliders["Tint"] = ui::FloatEditSlider::create(context);
            p.sliders["Tint"]->setDefaultValue(1.F);

            p.invertCheckBox = ui::CheckBox::create("Invert", context);

            p.layout = ui::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            p.enabledCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.enabledCheckBox, 0, 0);
            auto label = ui::Label::create("Add:", context, p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.sliders["Add"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["Add"], 1, 1);
            label = ui::Label::create("Brightness:", context, p.layout);
            p.layout->setGridPos(label, 2, 0);
            p.sliders["Brightness"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["Brightness"], 2, 1);
            label = ui::Label::create("Contrast:", context, p.layout);
            p.layout->setGridPos(label, 3, 0);
            p.sliders["Contrast"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["Contrast"], 3, 1);
            label = ui::Label::create("Saturation:", context, p.layout);
            p.layout->setGridPos(label, 4, 0);
            p.sliders["Saturation"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["Saturation"], 4, 1);
            label = ui::Label::create("Tint:", context, p.layout);
            p.layout->setGridPos(label, 5, 0);
            p.sliders["Tint"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["Tint"], 5, 1);
            p.invertCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.invertCheckBox, 6, 0);

            p.optionsObservers =
                observer::ValueObserver<timeline::DisplayOptions>::create(
                    app->getViewportModel()->observeDisplayOptions(),
                    [this](const timeline::DisplayOptions& value)
                    {
                        _p->enabledCheckBox->setChecked(value.color.enabled);
                        _p->sliders["Add"]->setValue(value.color.add.x);
                        _p->sliders["Brightness"]->setValue(
                            value.color.brightness.x);
                        _p->sliders["Contrast"]->setValue(
                            value.color.contrast.x);
                        _p->sliders["Saturation"]->setValue(
                            value.color.saturation.x);
                        _p->sliders["Tint"]->setValue(value.color.tint);
                        _p->invertCheckBox->setChecked(value.color.invert);
                    });

            auto appWeak = std::weak_ptr<App>(app);
            p.enabledCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getViewportModel()->getDisplayOptions();
                        options.color.enabled = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["Add"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getViewportModel()->getDisplayOptions();
                        options.color.enabled = true;
                        options.color.add.x = value;
                        options.color.add.y = value;
                        options.color.add.z = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["Brightness"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getViewportModel()->getDisplayOptions();
                        options.color.enabled = true;
                        options.color.brightness.x = value;
                        options.color.brightness.y = value;
                        options.color.brightness.z = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["Contrast"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getViewportModel()->getDisplayOptions();
                        options.color.enabled = true;
                        options.color.contrast.x = value;
                        options.color.contrast.y = value;
                        options.color.contrast.z = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["Saturation"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getViewportModel()->getDisplayOptions();
                        options.color.enabled = true;
                        options.color.saturation.x = value;
                        options.color.saturation.y = value;
                        options.color.saturation.z = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["Tint"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getViewportModel()->getDisplayOptions();
                        options.color.enabled = true;
                        options.color.tint = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.invertCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getViewportModel()->getDisplayOptions();
                        options.color.enabled = true;
                        options.color.invert = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });
        }

        ColorWidget::ColorWidget() :
            _p(new Private)
        {
        }

        ColorWidget::~ColorWidget() {}

        std::shared_ptr<ColorWidget> ColorWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ColorWidget>(new ColorWidget);
            out->_init(app, context, parent);
            return out;
        }

        void ColorWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void ColorWidget::sizeHintEvent(const ui::SizeHintEvent& value)
        {
            IWidget::sizeHintEvent(value);
            _sizeHint = _p->layout->getSizeHint();
        }

        struct LevelsWidget::Private
        {
            std::shared_ptr<ui::CheckBox> enabledCheckBox;
            std::map<std::string, std::shared_ptr<ui::FloatEditSlider> >
                sliders;
            std::shared_ptr<ui::GridLayout> layout;

            std::shared_ptr<observer::ValueObserver<timeline::DisplayOptions> >
                optionsObservers;
        };

        void LevelsWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<ui::IWidget>& parent)
        {
            ui::IWidget::_init("tl::play_app::LevelsWidget", context, parent);
            TLRENDER_P();

            p.enabledCheckBox = ui::CheckBox::create("Enabled", context);

            p.sliders["InLow"] = ui::FloatEditSlider::create(context);
            p.sliders["InLow"]->setDefaultValue(0.F);
            p.sliders["InHigh"] = ui::FloatEditSlider::create(context);
            p.sliders["InHigh"]->setDefaultValue(1.F);
            p.sliders["Gamma"] = ui::FloatEditSlider::create(context);
            p.sliders["Gamma"]->setRange(math::FloatRange(.1F, 4.F));
            p.sliders["Gamma"]->setDefaultValue(1.F);
            p.sliders["OutLow"] = ui::FloatEditSlider::create(context);
            p.sliders["OutLow"]->setDefaultValue(0.F);
            p.sliders["OutHigh"] = ui::FloatEditSlider::create(context);
            p.sliders["OutHigh"]->setDefaultValue(1.F);

            p.layout = ui::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            p.enabledCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.enabledCheckBox, 0, 0);
            auto label = ui::Label::create("In low:", context, p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.sliders["InLow"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["InLow"], 1, 1);
            label = ui::Label::create("In high:", context, p.layout);
            p.layout->setGridPos(label, 2, 0);
            p.sliders["InHigh"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["InHigh"], 2, 1);
            label = ui::Label::create("Gamma:", context, p.layout);
            p.layout->setGridPos(label, 3, 0);
            p.sliders["Gamma"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["Gamma"], 3, 1);
            label = ui::Label::create("Out low:", context, p.layout);
            p.layout->setGridPos(label, 4, 0);
            p.sliders["OutLow"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["OutLow"], 4, 1);
            label = ui::Label::create("Out high:", context, p.layout);
            p.layout->setGridPos(label, 5, 0);
            p.sliders["OutHigh"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["OutHigh"], 5, 1);

            p.optionsObservers =
                observer::ValueObserver<timeline::DisplayOptions>::create(
                    app->getViewportModel()->observeDisplayOptions(),
                    [this](const timeline::DisplayOptions& value)
                    {
                        _p->enabledCheckBox->setChecked(value.levels.enabled);
                        _p->sliders["InLow"]->setValue(value.levels.inLow);
                        _p->sliders["InHigh"]->setValue(value.levels.inHigh);
                        _p->sliders["Gamma"]->setValue(value.levels.gamma);
                        _p->sliders["OutLow"]->setValue(value.levels.outLow);
                        _p->sliders["OutHigh"]->setValue(value.levels.outHigh);
                    });

            auto appWeak = std::weak_ptr<App>(app);
            p.enabledCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getViewportModel()->getDisplayOptions();
                        options.levels.enabled = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["InLow"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getViewportModel()->getDisplayOptions();
                        options.levels.enabled = true;
                        options.levels.inLow = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["InHigh"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getViewportModel()->getDisplayOptions();
                        options.levels.enabled = true;
                        options.levels.inHigh = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["Gamma"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getViewportModel()->getDisplayOptions();
                        options.levels.enabled = true;
                        options.levels.gamma = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["OutLow"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getViewportModel()->getDisplayOptions();
                        options.levels.enabled = true;
                        options.levels.outLow = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["OutHigh"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getViewportModel()->getDisplayOptions();
                        options.levels.enabled = true;
                        options.levels.outHigh = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });
        }

        LevelsWidget::LevelsWidget() :
            _p(new Private)
        {
        }

        LevelsWidget::~LevelsWidget() {}

        std::shared_ptr<LevelsWidget> LevelsWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<LevelsWidget>(new LevelsWidget);
            out->_init(app, context, parent);
            return out;
        }

        void LevelsWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void LevelsWidget::sizeHintEvent(const ui::SizeHintEvent& value)
        {
            IWidget::sizeHintEvent(value);
            _sizeHint = _p->layout->getSizeHint();
        }

        struct EXRDisplayWidget::Private
        {
            std::shared_ptr<ui::CheckBox> enabledCheckBox;
            std::map<std::string, std::shared_ptr<ui::FloatEditSlider> >
                sliders;
            std::shared_ptr<ui::GridLayout> layout;

            std::shared_ptr<observer::ValueObserver<timeline::DisplayOptions> >
                optionsObservers;
        };

        void EXRDisplayWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<ui::IWidget>& parent)
        {
            ui::IWidget::_init(
                "tl::play_app::EXRDisplayWidget", context, parent);
            TLRENDER_P();

            p.enabledCheckBox = ui::CheckBox::create("Enabled", context);

            p.sliders["Exposure"] = ui::FloatEditSlider::create(context);
            p.sliders["Exposure"]->setRange(math::FloatRange(-10.F, 10.F));
            p.sliders["Exposure"]->setDefaultValue(0.F);
            p.sliders["Defog"] = ui::FloatEditSlider::create(context);
            p.sliders["Defog"]->setDefaultValue(0.F);
            p.sliders["KneeLow"] = ui::FloatEditSlider::create(context);
            p.sliders["KneeLow"]->setRange(math::FloatRange(-3.F, 3.F));
            p.sliders["KneeLow"]->setDefaultValue(0.F);
            p.sliders["KneeHigh"] = ui::FloatEditSlider::create(context);
            p.sliders["KneeHigh"]->setRange(math::FloatRange(3.5F, 7.5F));
            p.sliders["KneeHigh"]->setDefaultValue(5.F);

            p.layout = ui::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            p.enabledCheckBox->setParent(p.layout);
            p.layout->setGridPos(p.enabledCheckBox, 0, 0);
            auto label = ui::Label::create("Exposure:", context, p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.sliders["Exposure"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["Exposure"], 1, 1);
            label = ui::Label::create("Defog:", context, p.layout);
            p.layout->setGridPos(label, 2, 0);
            p.sliders["Defog"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["Defog"], 2, 1);
            label = ui::Label::create("Knee low:", context, p.layout);
            p.layout->setGridPos(label, 3, 0);
            p.sliders["KneeLow"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["KneeLow"], 3, 1);
            label = ui::Label::create("Knee high:", context, p.layout);
            p.layout->setGridPos(label, 4, 0);
            p.sliders["KneeHigh"]->setParent(p.layout);
            p.layout->setGridPos(p.sliders["KneeHigh"], 4, 1);

            p.optionsObservers =
                observer::ValueObserver<timeline::DisplayOptions>::create(
                    app->getViewportModel()->observeDisplayOptions(),
                    [this](const timeline::DisplayOptions& value)
                    {
                        _p->enabledCheckBox->setChecked(
                            value.exrDisplay.enabled);
                        _p->sliders["Exposure"]->setValue(
                            value.exrDisplay.exposure);
                        _p->sliders["Defog"]->setValue(value.exrDisplay.defog);
                        _p->sliders["KneeLow"]->setValue(
                            value.exrDisplay.kneeLow);
                        _p->sliders["KneeHigh"]->setValue(
                            value.exrDisplay.kneeHigh);
                    });

            auto appWeak = std::weak_ptr<App>(app);
            p.enabledCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getViewportModel()->getDisplayOptions();
                        options.exrDisplay.enabled = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["Exposure"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getViewportModel()->getDisplayOptions();
                        options.exrDisplay.enabled = true;
                        options.exrDisplay.exposure = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["Defog"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getViewportModel()->getDisplayOptions();
                        options.exrDisplay.enabled = true;
                        options.exrDisplay.defog = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["KneeLow"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getViewportModel()->getDisplayOptions();
                        options.exrDisplay.enabled = true;
                        options.exrDisplay.kneeLow = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["KneeHigh"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getViewportModel()->getDisplayOptions();
                        options.exrDisplay.enabled = true;
                        options.exrDisplay.kneeHigh = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });
        }

        EXRDisplayWidget::EXRDisplayWidget() :
            _p(new Private)
        {
        }

        EXRDisplayWidget::~EXRDisplayWidget() {}

        std::shared_ptr<EXRDisplayWidget> EXRDisplayWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<EXRDisplayWidget>(new EXRDisplayWidget);
            out->_init(app, context, parent);
            return out;
        }

        void EXRDisplayWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void EXRDisplayWidget::sizeHintEvent(const ui::SizeHintEvent& value)
        {
            IWidget::sizeHintEvent(value);
            _sizeHint = _p->layout->getSizeHint();
        }

        struct SoftClipWidget::Private
        {
            std::shared_ptr<ui::CheckBox> enabledCheckBox;
            std::map<std::string, std::shared_ptr<ui::FloatEditSlider> >
                sliders;
            std::shared_ptr<ui::VerticalLayout> layout;

            std::shared_ptr<observer::ValueObserver<timeline::DisplayOptions> >
                optionsObservers;
        };

        void SoftClipWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<ui::IWidget>& parent)
        {
            ui::IWidget::_init("tl::play_app::SoftClipWidget", context, parent);
            TLRENDER_P();

            p.enabledCheckBox = ui::CheckBox::create("Enabled", context);

            p.sliders["SoftClip"] = ui::FloatEditSlider::create(context);
            p.sliders["SoftClip"]->setDefaultValue(0.F);

            p.layout = ui::VerticalLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            p.enabledCheckBox->setParent(p.layout);
            p.sliders["SoftClip"]->setParent(p.layout);

            p.optionsObservers =
                observer::ValueObserver<timeline::DisplayOptions>::create(
                    app->getViewportModel()->observeDisplayOptions(),
                    [this](const timeline::DisplayOptions& value)
                    {
                        _p->enabledCheckBox->setChecked(value.softClip.enabled);
                        _p->sliders["SoftClip"]->setValue(value.softClip.value);
                    });

            auto appWeak = std::weak_ptr<App>(app);
            p.enabledCheckBox->setCheckedCallback(
                [appWeak](bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getViewportModel()->getDisplayOptions();
                        options.softClip.enabled = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });

            p.sliders["SoftClip"]->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getViewportModel()->getDisplayOptions();
                        options.softClip.enabled = true;
                        options.softClip.value = value;
                        app->getViewportModel()->setDisplayOptions(options);
                    }
                });
        }

        SoftClipWidget::SoftClipWidget() :
            _p(new Private)
        {
        }

        SoftClipWidget::~SoftClipWidget() {}

        std::shared_ptr<SoftClipWidget> SoftClipWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<SoftClipWidget>(new SoftClipWidget);
            out->_init(app, context, parent);
            return out;
        }

        void SoftClipWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void SoftClipWidget::sizeHintEvent(const ui::SizeHintEvent& value)
        {
            IWidget::sizeHintEvent(value);
            _sizeHint = _p->layout->getSizeHint();
        }

        struct ColorTool::Private
        {
            std::shared_ptr<OCIOWidget> ocioWidget;
            std::shared_ptr<LUTWidget> lutWidget;
            std::shared_ptr<ColorWidget> colorWidget;
            std::shared_ptr<LevelsWidget> levelsWidget;
            std::shared_ptr<EXRDisplayWidget> exrDisplayWidget;
            std::shared_ptr<SoftClipWidget> softClipWidget;
            std::map<std::string, std::shared_ptr<ui::Bellows> > bellows;
        };

        void ColorTool::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::Color, "tl::play_app::ColorTool", app, context, parent);
            TLRENDER_P();

            p.ocioWidget = OCIOWidget::create(app, context);
            p.lutWidget = LUTWidget::create(app, context);
            p.colorWidget = ColorWidget::create(app, context);
            p.levelsWidget = LevelsWidget::create(app, context);
            p.exrDisplayWidget = EXRDisplayWidget::create(app, context);
            p.softClipWidget = SoftClipWidget::create(app, context);

            auto layout = ui::VerticalLayout::create(context);
            layout->setSpacingRole(ui::SizeRole::None);
            p.bellows["OCIO"] = ui::Bellows::create("OCIO", context);
            p.bellows["OCIO"]->setParent(layout);
            p.bellows["OCIO"]->setWidget(p.ocioWidget);
            p.bellows["LUT"] = ui::Bellows::create("LUT", context);
            p.bellows["LUT"]->setParent(layout);
            p.bellows["LUT"]->setWidget(p.lutWidget);
            p.bellows["Color"] = ui::Bellows::create("Color", context);
            p.bellows["Color"]->setParent(layout);
            p.bellows["Color"]->setWidget(p.colorWidget);
            p.bellows["Levels"] = ui::Bellows::create("Levels", context);
            p.bellows["Levels"]->setParent(layout);
            p.bellows["Levels"]->setWidget(p.levelsWidget);
            p.bellows["EXRDisplay"] =
                ui::Bellows::create("EXR Display", context);
            p.bellows["EXRDisplay"]->setParent(layout);
            p.bellows["EXRDisplay"]->setWidget(p.exrDisplayWidget);
            p.bellows["SoftClip"] = ui::Bellows::create("Soft Clip", context);
            p.bellows["SoftClip"]->setParent(layout);
            p.bellows["SoftClip"]->setWidget(p.softClipWidget);
            auto scrollWidget = ui::ScrollWidget::create(context);
            scrollWidget->setWidget(layout);
            _setWidget(scrollWidget);
        }

        ColorTool::ColorTool() :
            _p(new Private)
        {
        }

        ColorTool::~ColorTool() {}

        std::shared_ptr<ColorTool> ColorTool::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ColorTool>(new ColorTool);
            out->_init(app, context, parent);
            return out;
        }
    } // namespace play_app
} // namespace tl
