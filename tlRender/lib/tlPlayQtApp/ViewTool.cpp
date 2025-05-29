// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/ViewTool.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/DockTitleBar.h>

#include <tlQtWidget/ColorSwatch.h>
#include <tlQtWidget/ColorWidget.h>
#include <tlQtWidget/IntEditSlider.h>

#include <QAction>
#include <QBoxLayout>
#include <QComboBox>
#include <QFormLayout>
#include <QGroupBox>

namespace tl
{
    namespace play_qt
    {
        struct BackgroundWidget::Private
        {
            QComboBox* typeComboBox = nullptr;
            qtwidget::ColorSwatch* color0Swatch = nullptr;
            qtwidget::ColorSwatch* color1Swatch = nullptr;
            qtwidget::IntEditSlider* checkersSizeSlider = nullptr;

            std::shared_ptr<
                observer::ValueObserver<timeline::BackgroundOptions> >
                optionsObservers;
        };

        BackgroundWidget::BackgroundWidget(App* app, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.typeComboBox = new QComboBox;
            for (const auto& i : timeline::getBackgroundLabels())
            {
                p.typeComboBox->addItem(QString::fromUtf8(i.c_str()));
            }

            p.color0Swatch = new qtwidget::ColorSwatch;
            p.color0Swatch->setEditable(true);

            p.color1Swatch = new qtwidget::ColorSwatch;
            p.color1Swatch->setEditable(true);

            p.checkersSizeSlider = new qtwidget::IntEditSlider;
            p.checkersSizeSlider->setRange(math::IntRange(10, 100));

            auto layout = new QFormLayout;
            layout->addRow(tr("Type:"), p.typeComboBox);
            layout->addRow(tr("Color 0:"), p.color0Swatch);
            layout->addRow(tr("Color 1:"), p.color1Swatch);
            layout->addRow(tr("Checkers size:"), p.checkersSizeSlider);
            setLayout(layout);

            connect(
                p.typeComboBox,
                QOverload<int>::of(&QComboBox::currentIndexChanged),
                [app](int value)
                {
                    auto options = app->viewportModel()->getBackgroundOptions();
                    options.type = static_cast<timeline::Background>(value);
                    app->viewportModel()->setBackgroundOptions(options);
                });

            connect(
                p.color0Swatch, &qtwidget::ColorSwatch::colorChanged,
                [app](const image::Color4f& value)
                {
                    auto options = app->viewportModel()->getBackgroundOptions();
                    options.color0 = value;
                    app->viewportModel()->setBackgroundOptions(options);
                });
            connect(
                p.color1Swatch, &qtwidget::ColorSwatch::colorChanged,
                [app](const image::Color4f& value)
                {
                    auto options = app->viewportModel()->getBackgroundOptions();
                    options.color1 = value;
                    app->viewportModel()->setBackgroundOptions(options);
                });

            connect(
                p.checkersSizeSlider, &qtwidget::IntEditSlider::valueChanged,
                [app](int value)
                {
                    auto options = app->viewportModel()->getBackgroundOptions();
                    options.checkersSize.w = value;
                    options.checkersSize.h = value;
                    app->viewportModel()->setBackgroundOptions(options);
                });

            p.optionsObservers =
                observer::ValueObserver<timeline::BackgroundOptions>::create(
                    app->viewportModel()->observeBackgroundOptions(),
                    [this](const timeline::BackgroundOptions& value)
                    { _optionsUpdate(value); });
        }

        BackgroundWidget::~BackgroundWidget() {}

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
            BackgroundWidget* backgroundWidget = nullptr;
        };

        ViewTool::ViewTool(App* app, QWidget* parent) :
            IToolWidget(app, parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.backgroundWidget = new BackgroundWidget(app);

            addBellows(tr("Background"), p.backgroundWidget);
            addStretch();
        }

        ViewTool::~ViewTool() {}

        ViewDockWidget::ViewDockWidget(ViewTool* viewTool, QWidget* parent)
        {
            setObjectName("ViewTool");
            setWindowTitle(tr("View"));
            setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

            auto dockTitleBar = new DockTitleBar;
            dockTitleBar->setText(tr("View"));
            dockTitleBar->setIcon(QIcon(":/Icons/View.svg"));
            auto dockWidget = new QDockWidget;
            setTitleBarWidget(dockTitleBar);

            setWidget(viewTool);

            toggleViewAction()->setIcon(QIcon(":/Icons/View.svg"));
            toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F5));
            toggleViewAction()->setToolTip(tr("Show view controls"));
        }
    } // namespace play_qt
} // namespace tl
