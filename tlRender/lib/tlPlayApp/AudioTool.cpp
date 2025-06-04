// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/AudioTool.h>

#include <tlPlayApp/App.h>

#include <tlPlay/AudioModel.h>

#include <tlUI/Bellows.h>
#include <tlUI/DoubleEditSlider.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ScrollWidget.h>

namespace tl
{
    namespace play_app
    {
        struct AudioTool::Private
        {
            std::shared_ptr<ui::DoubleEditSlider> syncOffsetSlider;

            std::shared_ptr<observer::ValueObserver<double> >
                syncOffsetObserver;
        };

        void AudioTool::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IToolWidget::_init(
                Tool::Audio, "tl::play_app::AudioTool", app, context, parent);
            TLRENDER_P();

            p.syncOffsetSlider = ui::DoubleEditSlider::create(context);
            p.syncOffsetSlider->setRange(math::DoubleRange(-1.0, 1.0));
            p.syncOffsetSlider->setDefaultValue(0.0);

            auto layout = ui::VerticalLayout::create(context);
            auto vLayout = ui::VerticalLayout::create(context);
            vLayout->setMarginRole(ui::SizeRole::MarginSmall);
            vLayout->setSpacingRole(ui::SizeRole::SpacingSmall);
            p.syncOffsetSlider->setParent(vLayout);
            auto bellows = ui::Bellows::create("Sync Offset", context, layout);
            bellows->setWidget(vLayout);
            auto scrollWidget = ui::ScrollWidget::create(context);
            scrollWidget->setWidget(layout);
            _setWidget(scrollWidget);

            auto appWeak = std::weak_ptr<App>(app);
            p.syncOffsetSlider->setCallback(
                [appWeak](double value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getAudioModel()->setSyncOffset(value);
                    }
                });

            p.syncOffsetObserver = observer::ValueObserver<double>::create(
                app->getAudioModel()->observeSyncOffset(), [this](double value)
                { _p->syncOffsetSlider->setValue(value); });
        }

        AudioTool::AudioTool() :
            _p(new Private)
        {
        }

        AudioTool::~AudioTool() {}

        std::shared_ptr<AudioTool> AudioTool::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<AudioTool>(new AudioTool);
            out->_init(app, context, parent);
            return out;
        }
    } // namespace play_app
} // namespace tl
