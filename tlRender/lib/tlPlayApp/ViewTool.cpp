// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/ViewToolPrivate.h>

#include <tlPlayApp/App.h>

#include <tlPlay/ViewportModel.h>

#include <tlUI/Bellows.h>
#include <tlUI/ColorSwatch.h>
#include <tlUI/ComboBox.h>
#include <tlUI/GridLayout.h>
#include <tlUI/GroupBox.h>
#include <tlUI/IntEditSlider.h>
#include <tlUI/Label.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollWidget.h>

namespace tl
{
    namespace play_app
    {
        struct BackgroundWidget::Private
        {
            std::shared_ptr<ui::ComboBox> typeComboBox;
            std::shared_ptr<ui::ColorSwatch> color0Swatch;
            std::shared_ptr<ui::ColorSwatch> color1Swatch;
            std::shared_ptr<ui::IntEditSlider> checkersSizeSlider;
            std::shared_ptr<ui::GridLayout> layout;

            std::shared_ptr<
                observer::ValueObserver<timeline::BackgroundOptions> >
                optionsObservers;
        };

        void BackgroundWidget::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<ui::IWidget>& parent)
        {
            ui::IWidget::_init(
                "tl::play_app::BackgroundWidget", context, parent);
            TLRENDER_P();

            p.typeComboBox =
                ui::ComboBox::create(timeline::getBackgroundLabels(), context);

            p.color0Swatch = ui::ColorSwatch::create(context);
            p.color0Swatch->setEditable(true);
            p.color1Swatch = ui::ColorSwatch::create(context);
            p.color1Swatch->setEditable(true);
            p.checkersSizeSlider = ui::IntEditSlider::create(context);
            p.checkersSizeSlider->setRange(math::IntRange(10, 100));

            p.layout = ui::GridLayout::create(context, shared_from_this());
            p.layout->setMarginRole(ui::SizeRole::MarginSmall);
            p.layout->setSpacingRole(ui::SizeRole::SpacingSmall);
            auto label = ui::Label::create("Type:", context, p.layout);
            p.layout->setGridPos(label, 0, 0);
            p.typeComboBox->setParent(p.layout);
            p.layout->setGridPos(p.typeComboBox, 0, 1);
            label = ui::Label::create("Color 0:", context, p.layout);
            p.layout->setGridPos(label, 1, 0);
            p.color0Swatch->setParent(p.layout);
            p.layout->setGridPos(p.color0Swatch, 1, 1);
            label = ui::Label::create("Color 1:", context, p.layout);
            p.layout->setGridPos(label, 2, 0);
            p.color1Swatch->setParent(p.layout);
            p.layout->setGridPos(p.color1Swatch, 2, 1);
            label = ui::Label::create("Checkers size:", context, p.layout);
            p.layout->setGridPos(label, 3, 0);
            p.checkersSizeSlider->setParent(p.layout);
            p.layout->setGridPos(p.checkersSizeSlider, 3, 1);

            p.optionsObservers =
                observer::ValueObserver<timeline::BackgroundOptions>::create(
                    app->getViewportModel()->observeBackgroundOptions(),
                    [this](const timeline::BackgroundOptions& value)
                    { _optionsUpdate(value); });

            auto appWeak = std::weak_ptr<App>(app);
            p.typeComboBox->setIndexCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getViewportModel()->getBackgroundOptions();
                        options.type = static_cast<timeline::Background>(value);
                        app->getViewportModel()->setBackgroundOptions(options);
                    }
                });

            p.color0Swatch->setCallback(
                [appWeak](const image::Color4f& value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getViewportModel()->getBackgroundOptions();
                        options.color0 = value;
                        app->getViewportModel()->setBackgroundOptions(options);
                    }
                });

            p.color1Swatch->setCallback(
                [appWeak](const image::Color4f& value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getViewportModel()->getBackgroundOptions();
                        options.color1 = value;
                        app->getViewportModel()->setBackgroundOptions(options);
                    }
                });

            p.checkersSizeSlider->setCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        auto options =
                            app->getViewportModel()->getBackgroundOptions();
                        options.checkersSize.w = value;
                        options.checkersSize.h = value;
                        app->getViewportModel()->setBackgroundOptions(options);
                    }
                });
        }

        BackgroundWidget::BackgroundWidget() :
            _p(new Private)
        {
        }

        BackgroundWidget::~BackgroundWidget() {}

        std::shared_ptr<BackgroundWidget> BackgroundWidget::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<BackgroundWidget>(new BackgroundWidget);
            out->_init(app, context, parent);
            return out;
        }

        void BackgroundWidget::setGeometry(const math::Box2i& value)
        {
            IWidget::setGeometry(value);
            _p->layout->setGeometry(value);
        }

        void BackgroundWidget::sizeHintEvent(const ui::SizeHintEvent& value)
        {
            IWidget::sizeHintEvent(value);
            _sizeHint = _p->layout->getSizeHint();
        }

        void BackgroundWidget::_optionsUpdate(
            const timeline::BackgroundOptions& value)
        {
            TLRENDER_P();
            p.typeComboBox->setCurrentIndex(static_cast<int>(value.type));
            p.color0Swatch->setColor(value.color0);
            p.color1Swatch->setColor(value.color1);
            p.checkersSizeSlider->setValue(value.checkersSize.w);
        }

        struct ViewTool::Private
        {
            std::shared_ptr<BackgroundWidget> backgroundWidget;
        };

        void ViewTool::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::View, "tl::play_app::ViewTool", app, context, parent);
            TLRENDER_P();

            p.backgroundWidget = BackgroundWidget::create(app, context);

            auto layout = ui::VerticalLayout::create(context);
            auto bellows = ui::Bellows::create("Background", context, layout);
            bellows->setWidget(p.backgroundWidget);
            auto scrollWidget = ui::ScrollWidget::create(context);
            scrollWidget->setWidget(layout);
            _setWidget(scrollWidget);
        }

        ViewTool::ViewTool() :
            _p(new Private)
        {
        }

        ViewTool::~ViewTool() {}

        std::shared_ptr<ViewTool> ViewTool::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<ViewTool>(new ViewTool);
            out->_init(app, context, parent);
            return out;
        }
    } // namespace play_app
} // namespace tl
