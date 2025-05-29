// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2024 Darby Johnston
// All rights reserved.

#include <tlPlay/AudioModel.h>

#include <tlPlay/Settings.h>

#include <tlCore/Math.h>

namespace tl
{
    namespace play
    {
        struct AudioModel::Private
        {
            std::shared_ptr<Settings> settings;
            std::shared_ptr<observer::Value<float> > volume;
            std::shared_ptr<observer::Value<bool> > mute;
            std::shared_ptr<observer::Value<double> > syncOffset;
        };

        void AudioModel::_init(
            const std::shared_ptr<Settings>& settings,
            const std::shared_ptr<system::Context>& context)
        {
            TLRENDER_P();

            p.settings = settings;

            p.settings->setDefaultValue("Audio/Volume", 1.F);
            p.volume = observer::Value<float>::create(
                p.settings->getValue<float>("Audio/Volume"));

            p.settings->setDefaultValue("Audio/Mute", false);
            p.mute = observer::Value<bool>::create(
                p.settings->getValue<bool>("Audio/Mute"));

            double offset = 0.0;
            p.syncOffset = observer::Value<double>::create(0.0);
        }

        AudioModel::AudioModel() :
            _p(new Private)
        {
        }

        AudioModel::~AudioModel() {}

        std::shared_ptr<AudioModel> AudioModel::create(
            const std::shared_ptr<Settings>& settings,
            const std::shared_ptr<system::Context>& context)
        {
            auto out = std::shared_ptr<AudioModel>(new AudioModel);
            out->_init(settings, context);
            return out;
        }

        float AudioModel::getVolume() const
        {
            return _p->volume->get();
        }

        std::shared_ptr<observer::IValue<float> >
        AudioModel::observeVolume() const
        {
            return _p->volume;
        }

        void AudioModel::setVolume(float value)
        {
            const float tmp = math::clamp(value, 0.F, 1.F);
            _p->settings->setValue("Audio/Volume", tmp);
            _p->volume->setIfChanged(tmp);
        }

        void AudioModel::volumeUp()
        {
            setVolume(_p->volume->get() + .1F);
        }

        void AudioModel::volumeDown()
        {
            setVolume(_p->volume->get() - .1F);
        }

        bool AudioModel::isMuted() const
        {
            return _p->mute->get();
        }

        std::shared_ptr<observer::IValue<bool> > AudioModel::observeMute() const
        {
            return _p->mute;
        }

        void AudioModel::setMute(bool value)
        {
            _p->settings->setValue("Audio/Mute", value);
            _p->mute->setIfChanged(value);
        }

        double AudioModel::getSyncOffset() const
        {
            return _p->syncOffset->get();
        }

        std::shared_ptr<observer::IValue<double> >
        AudioModel::observeSyncOffset() const
        {
            return _p->syncOffset;
        }

        void AudioModel::setSyncOffset(double value)
        {
            _p->syncOffset->setIfChanged(value);
        }
    } // namespace play
} // namespace tl
