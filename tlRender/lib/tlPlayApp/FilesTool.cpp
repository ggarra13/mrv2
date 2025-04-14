// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/FilesToolPrivate.h>

#include <tlPlayApp/App.h>

#include <tlUI/Bellows.h>
#include <tlUI/ButtonGroup.h>
#include <tlUI/ComboBox.h>
#include <tlUI/FloatEditSlider.h>
#include <tlUI/GridLayout.h>
#include <tlUI/Label.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollWidget.h>
#include <tlUI/ToolButton.h>

namespace tl
{
    namespace play_app
    {
        struct FilesTool::Private
        {
            std::shared_ptr<ui::ButtonGroup> aButtonGroup;
            std::shared_ptr<ui::ButtonGroup> bButtonGroup;
            std::map<
                std::shared_ptr<play::FilesModelItem>,
                std::shared_ptr<FileButton> >
                aButtons;
            std::map<
                std::shared_ptr<play::FilesModelItem>,
                std::shared_ptr<ui::ToolButton> >
                bButtons;
            std::vector<std::shared_ptr<ui::ComboBox> > layerComboBoxes;
            std::shared_ptr<ui::FloatEditSlider> wipeXSlider;
            std::shared_ptr<ui::FloatEditSlider> wipeYSlider;
            std::shared_ptr<ui::FloatEditSlider> wipeRotationSlider;
            std::shared_ptr<ui::FloatEditSlider> overlaySlider;
            std::shared_ptr<ui::GridLayout> widgetLayout;

            std::shared_ptr<
                observer::ListObserver<std::shared_ptr<play::FilesModelItem> > >
                filesObserver;
            std::shared_ptr<observer::ValueObserver<
                std::shared_ptr<play::FilesModelItem> > >
                aObserver;
            std::shared_ptr<
                observer::ListObserver<std::shared_ptr<play::FilesModelItem> > >
                bObserver;
            std::shared_ptr<observer::ListObserver<int> > layersObserver;
            std::shared_ptr<observer::ValueObserver<timeline::CompareOptions> >
                compareObserver;
        };

        void FilesTool::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::Files, "tl::play_app::FilesTool", app, context, parent);
            TLRENDER_P();

            p.aButtonGroup =
                ui::ButtonGroup::create(ui::ButtonGroupType::Radio, context);
            p.bButtonGroup =
                ui::ButtonGroup::create(ui::ButtonGroupType::Check, context);

            p.wipeXSlider = ui::FloatEditSlider::create(context);
            p.wipeXSlider->setDefaultValue(.5F);
            p.wipeYSlider = ui::FloatEditSlider::create(context);
            p.wipeYSlider->setDefaultValue(.5F);
            p.wipeRotationSlider = ui::FloatEditSlider::create(context);
            p.wipeRotationSlider->setRange(math::FloatRange(0.F, 360.F));
            p.wipeRotationSlider->setStep(1.F);
            p.wipeRotationSlider->setLargeStep(10.F);
            p.wipeRotationSlider->setDefaultValue(0.F);

            p.overlaySlider = ui::FloatEditSlider::create(context);
            p.overlaySlider->setDefaultValue(.5F);

            auto layout = ui::VerticalLayout::create(context);
            layout->setSpacingRole(ui::SizeRole::kNone);

            p.widgetLayout = ui::GridLayout::create(context, layout);
            p.widgetLayout->setMarginRole(ui::SizeRole::MarginSmall);
            p.widgetLayout->setSpacingRole(ui::SizeRole::kNone);

            auto vLayout = ui::VerticalLayout::create(context, layout);
            vLayout->setSpacingRole(ui::SizeRole::kNone);
            auto bellows = ui::Bellows::create("Wipe", context, vLayout);
            auto gridLayout = ui::GridLayout::create(context);
            gridLayout->setMarginRole(ui::SizeRole::MarginSmall);
            auto label = ui::Label::create("X:", context, gridLayout);
            gridLayout->setGridPos(label, 0, 0);
            p.wipeXSlider->setParent(gridLayout);
            gridLayout->setGridPos(p.wipeXSlider, 0, 1);
            label = ui::Label::create("Y:", context, gridLayout);
            gridLayout->setGridPos(label, 1, 0);
            p.wipeYSlider->setParent(gridLayout);
            gridLayout->setGridPos(p.wipeYSlider, 1, 1);
            label = ui::Label::create("Rotation:", context, gridLayout);
            gridLayout->setGridPos(label, 2, 0);
            p.wipeRotationSlider->setParent(gridLayout);
            gridLayout->setGridPos(p.wipeRotationSlider, 2, 1);
            bellows->setWidget(gridLayout);

            bellows = ui::Bellows::create("Overlay", context, vLayout);
            gridLayout = ui::GridLayout::create(context);
            gridLayout->setMarginRole(ui::SizeRole::MarginSmall);
            p.overlaySlider->setParent(gridLayout);
            gridLayout->setGridPos(p.overlaySlider, 0, 0);
            bellows->setWidget(gridLayout);

            auto scrollWidget =
                ui::ScrollWidget::create(context, ui::ScrollType::Both);
            scrollWidget->setWidget(layout);
            _setWidget(scrollWidget);

            auto appWeak = std::weak_ptr<App>(app);
            p.aButtonGroup->setCheckedCallback(
                [appWeak](int index, bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->setA(index);
                    }
                });

            p.bButtonGroup->setCheckedCallback(
                [appWeak](int index, bool value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getFilesModel()->setB(index, value);
                    }
                });

            p.wipeXSlider->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getFilesModel()->getCompareOptions();
                        options.wipeCenter.x = value;
                        app->getFilesModel()->setCompareOptions(options);
                    }
                });

            p.wipeYSlider->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getFilesModel()->getCompareOptions();
                        options.wipeCenter.y = value;
                        app->getFilesModel()->setCompareOptions(options);
                    }
                });

            p.wipeRotationSlider->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getFilesModel()->getCompareOptions();
                        options.wipeRotation = value;
                        app->getFilesModel()->setCompareOptions(options);
                    }
                });

            p.overlaySlider->setCallback(
                [appWeak](float value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getFilesModel()->getCompareOptions();
                        options.overlay = value;
                        app->getFilesModel()->setCompareOptions(options);
                    }
                });

            p.filesObserver = observer::
                ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                    app->getFilesModel()->observeFiles(),
                    [this](const std::vector<
                           std::shared_ptr<play::FilesModelItem> >& value)
                    { _filesUpdate(value); });

            p.aObserver = observer::
                ValueObserver<std::shared_ptr<play::FilesModelItem> >::create(
                    app->getFilesModel()->observeA(),
                    [this](const std::shared_ptr<play::FilesModelItem>& value)
                    { _aUpdate(value); });

            p.bObserver = observer::
                ListObserver<std::shared_ptr<play::FilesModelItem> >::create(
                    app->getFilesModel()->observeB(),
                    [this](const std::vector<
                           std::shared_ptr<play::FilesModelItem> >& value)
                    { _bUpdate(value); });

            p.layersObserver = observer::ListObserver<int>::create(
                app->getFilesModel()->observeLayers(),
                [this](const std::vector<int>& value)
                { _layersUpdate(value); });

            p.compareObserver =
                observer::ValueObserver<timeline::CompareOptions>::create(
                    app->getFilesModel()->observeCompareOptions(),
                    [this](const timeline::CompareOptions& value)
                    { _compareUpdate(value); });
        }

        FilesTool::FilesTool() :
            _p(new Private)
        {
        }

        FilesTool::~FilesTool() {}

        std::shared_ptr<FilesTool> FilesTool::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<FilesTool>(new FilesTool);
            out->_init(app, context, parent);
            return out;
        }

        void FilesTool::_filesUpdate(
            const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
        {
            TLRENDER_P();
            p.aButtonGroup->clearButtons();
            p.bButtonGroup->clearButtons();
            p.layerComboBoxes.clear();
            auto children = p.widgetLayout->getChildren();
            for (const auto& widget : children)
            {
                widget->setParent(nullptr);
            }
            children.clear();
            auto appWeak = _app;
            if (auto app = appWeak.lock())
            {
                const auto& a = app->getFilesModel()->getA();
                const auto& b = app->getFilesModel()->getB();
                if (auto context = _context.lock())
                {
                    size_t row = 0;
                    for (const auto& item : value)
                    {
                        auto aButton = FileButton::create(item, context);
                        aButton->setChecked(item == a);
                        aButton->setToolTip(item->path.get());
                        p.aButtons[item] = aButton;
                        p.aButtonGroup->addButton(aButton);
                        aButton->setParent(p.widgetLayout);
                        p.widgetLayout->setGridPos(aButton, row, 0);

                        auto bButton = ui::ToolButton::create(context);
                        bButton->setText("B");
                        const auto i = std::find(b.begin(), b.end(), item);
                        bButton->setChecked(i != b.end());
                        bButton->setToolTip("Set the B file(s)");
                        p.bButtons[item] = bButton;
                        p.bButtonGroup->addButton(bButton);
                        bButton->setParent(p.widgetLayout);
                        p.widgetLayout->setGridPos(bButton, row, 1);

                        auto layerComboBox = ui::ComboBox::create(context);
                        layerComboBox->setItems(item->videoLayers);
                        layerComboBox->setCurrentIndex(item->videoLayer);
                        layerComboBox->setHAlign(ui::HAlign::Left);
                        layerComboBox->setToolTip("Set the current layer");
                        p.layerComboBoxes.push_back(layerComboBox);
                        layerComboBox->setParent(p.widgetLayout);
                        p.widgetLayout->setGridPos(layerComboBox, row, 2);

                        layerComboBox->setIndexCallback(
                            [appWeak, item](int value)
                            {
                                if (auto app = appWeak.lock())
                                {
                                    app->getFilesModel()->setLayer(item, value);
                                }
                            });

                        ++row;
                    }
                    if (value.empty())
                    {
                        auto label = ui::Label::create(
                            "No files open", context, p.widgetLayout);
                        p.widgetLayout->setGridPos(label, 0, 0);
                    }
                }
            }
        }

        void
        FilesTool::_aUpdate(const std::shared_ptr<play::FilesModelItem>& value)
        {
            TLRENDER_P();
            for (const auto& button : p.aButtons)
            {
                button.second->setChecked(button.first == value);
            }
        }

        void FilesTool::_bUpdate(
            const std::vector<std::shared_ptr<play::FilesModelItem> >& value)
        {
            TLRENDER_P();
            for (const auto& button : p.bButtons)
            {
                const auto i =
                    std::find(value.begin(), value.end(), button.first);
                button.second->setChecked(i != value.end());
            }
        }

        void FilesTool::_layersUpdate(const std::vector<int>& value)
        {
            TLRENDER_P();
            for (size_t i = 0; i < value.size() && i < p.layerComboBoxes.size();
                 ++i)
            {
                p.layerComboBoxes[i]->setCurrentIndex(value[i]);
            }
        }

        void FilesTool::_compareUpdate(const timeline::CompareOptions& value)
        {
            TLRENDER_P();
            p.wipeXSlider->setValue(value.wipeCenter.x);
            p.wipeYSlider->setValue(value.wipeCenter.y);
            p.wipeRotationSlider->setValue(value.wipeRotation);
            p.overlaySlider->setValue(value.overlay);
        }
    } // namespace play_app
} // namespace tl
