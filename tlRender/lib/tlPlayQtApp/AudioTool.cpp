// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayQtApp/AudioTool.h>

#include <tlPlayQtApp/App.h>
#include <tlPlayQtApp/DockTitleBar.h>

#include <tlPlay/AudioModel.h>

#include <tlQtWidget/FloatEditSlider.h>

#include <QAction>
#include <QBoxLayout>

namespace tl
{
    namespace play_qt
    {
        struct AudioOffsetWidget::Private
        {
            qtwidget::FloatEditSlider* slider = nullptr;

            std::shared_ptr<observer::ValueObserver<double> >
                syncOffsetObserver;
        };

        AudioOffsetWidget::AudioOffsetWidget(App* app, QWidget* parent) :
            QWidget(parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.slider = new qtwidget::FloatEditSlider;
            p.slider->setRange(math::FloatRange(-1.F, 1.F));
            p.slider->setDefaultValue(0.F);

            auto layout = new QVBoxLayout;
            layout->addWidget(p.slider);
            layout->addStretch();
            setLayout(layout);

            connect(
                p.slider, &qtwidget::FloatEditSlider::valueChanged,
                [app](float value)
                { app->audioModel()->setSyncOffset(value); });

            p.syncOffsetObserver = observer::ValueObserver<double>::create(
                app->audioModel()->observeSyncOffset(),
                [this](double value)
                {
                    QSignalBlocker signalBlocker(_p->slider);
                    _p->slider->setValue(value);
                });
        }

        AudioOffsetWidget::~AudioOffsetWidget() {}

        struct AudioTool::Private
        {
            AudioOffsetWidget* offsetWidget = nullptr;
        };

        AudioTool::AudioTool(App* app, QWidget* parent) :
            IToolWidget(app, parent),
            _p(new Private)
        {
            TLRENDER_P();

            p.offsetWidget = new AudioOffsetWidget(app);

            addBellows(tr("Sync Offset"), p.offsetWidget);
            addStretch();
        }

        AudioTool::~AudioTool() {}

        AudioDockWidget::AudioDockWidget(AudioTool* audioTool, QWidget* parent)
        {
            setObjectName("AudioTool");
            setWindowTitle(tr("Audio"));
            setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

            auto dockTitleBar = new DockTitleBar;
            dockTitleBar->setText(tr("Audio"));
            dockTitleBar->setIcon(QIcon(":/Icons/Audio.svg"));
            auto dockWidget = new QDockWidget;
            setTitleBarWidget(dockTitleBar);

            setWidget(audioTool);

            toggleViewAction()->setIcon(QIcon(":/Icons/Audio.svg"));
            toggleViewAction()->setShortcut(QKeySequence(Qt::Key_F5));
            toggleViewAction()->setToolTip(tr("Show audio controls"));
        }
    } // namespace play_qt
} // namespace tl
