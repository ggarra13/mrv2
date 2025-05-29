// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlayApp/AudioPopup.h>

#include <tlPlayApp/App.h>

#include <tlPlay/AudioModel.h>

#include <tlUI/IntEditSlider.h>
#include <tlUI/RowLayout.h>
#include <tlUI/ToolButton.h>

#include <tlCore/StringFormat.h>

namespace tl
{
    namespace play_app
    {
        struct AudioPopup::Private
        {
            std::shared_ptr<ui::IntEditSlider> volumeSlider;
            std::shared_ptr<ui::HorizontalLayout> layout;

            std::shared_ptr<observer::ValueObserver<float> > volumeObserver;
        };

        void AudioPopup::_init(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidgetPopup::_init("tl::play_app::AudioPopup", context, parent);
            TLRENDER_P();

            p.volumeSlider = ui::IntEditSlider::create(context);
            p.volumeSlider->setRange(math::IntRange(0, 100));
            p.volumeSlider->setStep(1);
            p.volumeSlider->setLargeStep(10);
            p.volumeSlider->setToolTip("Audio volume");

            p.layout = ui::HorizontalLayout::create(context);
            p.layout->setMarginRole(ui::SizeRole::MarginInside);
            p.layout->setSpacingRole(ui::SizeRole::SpacingTool);
            p.volumeSlider->setParent(p.layout);
            setWidget(p.layout);

            auto appWeak = std::weak_ptr<App>(app);
            p.volumeSlider->setCallback(
                [appWeak](int value)
                {
                    if (auto app = appWeak.lock())
                    {
                        app->getAudioModel()->setVolume(value / 100.F);
                    }
                });

            p.volumeObserver = observer::ValueObserver<float>::create(
                app->getAudioModel()->observeVolume(), [this](float value)
                { _p->volumeSlider->setValue(std::roundf(value * 100.F)); });
        }

        AudioPopup::AudioPopup() :
            _p(new Private)
        {
        }

        AudioPopup::~AudioPopup() {}

        std::shared_ptr<AudioPopup> AudioPopup::create(
            const std::shared_ptr<App>& app,
            const std::shared_ptr<system::Context>& context,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<AudioPopup>(new AudioPopup);
            out->_init(app, context, parent);
            return out;
        }
    } // namespace play_app
} // namespace tl
