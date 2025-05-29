// SPDX-License-Identifier: BSD-3-Clause
// Copyright (c) 2021-2025 Darby Johnston and Gonzalo Garramuño
// All rights reserved.

#include <tlTimeline/PlayerPrivate.h>

#include <tlTimeline/Util.h>

#include <tlCore/StringFormat.h>

namespace
{
    inline float fadeValue(double sample, double in, double out)
    {
        return (sample - in) / (out - in);
    }
} // namespace

namespace tl
{
    namespace timeline
    {
        const std::vector<int>& Player::getChannelMute() const
        {
            return _p->channelMute->get();
        }

        std::shared_ptr<observer::IList<int> >
        Player::observeChannelMute() const
        {
            return _p->channelMute;
        }

        void Player::setChannelMute(const std::vector<int>& value)
        {
            TLRENDER_P();
            if (p.channelMute->setIfChanged(value))
            {
                std::unique_lock<std::mutex> lock(p.audioMutex.mutex);
                p.audioMutex.channelMute = value;
            }
        }

        void Player::Private::resetAudioTime()
        {
            {
                std::unique_lock<std::mutex> lock(audioMutex.mutex);
                audioMutex.reset = true;
            }
#if defined(TLRENDER_AUDIO)
            if (thread.rtAudio && thread.rtAudio->isStreamRunning())
            {
                try
                {
                    thread.rtAudio->setStreamTime(0.0);
                }
                catch (const std::exception&)
                {
                    //! \todo How should this be handled?
                }
            }
#endif // TLRENDER_AUDIO
        }

#if defined(TLRENDER_AUDIO)
        int Player::Private::rtAudioCallback(
            void* outputBuffer, void* inputBuffer, unsigned int outSamples,
            double streamTime, RtAudioStreamStatus status, void* userData)
        {
            auto p = reinterpret_cast<Player::Private*>(userData);

            // Get mutex protected values.
            Playback playback = Playback::Stop;
            otime::RationalTime playbackStartTime = time::invalidTime;
            double audioOffset = 0.0;
            {
                std::unique_lock<std::mutex> lock(p->mutex.mutex);
                playback = p->mutex.playback;
                playbackStartTime = p->mutex.playbackStartTime;
                audioOffset = p->mutex.audioOffset;
            }
            double speed = 0.0;
            double defaultSpeed = 0.0;
            double speedMultiplier = 1.0F;
            float volume = 1.F;
            bool mute = false;
            std::vector<int> channelMute;
            std::chrono::steady_clock::time_point muteTimeout;
            bool reset = false;
            {
                std::unique_lock<std::mutex> lock(p->audioMutex.mutex);
                speed = p->audioMutex.speed;
                defaultSpeed = p->timeline->getTimeRange().duration().rate();
                speedMultiplier = defaultSpeed / speed;
                volume = p->audioMutex.volume;
                mute = p->audioMutex.mute;
                channelMute = p->audioMutex.channelMute;
                muteTimeout = p->audioMutex.muteTimeout;
                reset = p->audioMutex.reset;
                p->audioMutex.reset = false;
            }
            // std::cout << "playback: " << playback << std::endl;
            // std::cout << "playbackStartTime: " << playbackStartTime <<
            // std::endl;

            auto& thread = p->audioThread;

            // Zero output audio data.
            std::memset(
                outputBuffer, 0, outSamples * thread.info.getByteCount());

            switch (playback)
            {
            case Playback::Forward:
            case Playback::Reverse:
            {
                // Flush the audio resampler and buffer when the RtAudio
                // playback is reset.
                if (reset)
                {
                    if (thread.resample)
                    {
                        thread.resample->flush();
                    }
                    thread.silence.reset();
                    thread.buffer.clear();
                    thread.rtAudioCurrentFrame = 0;
                    thread.backwardsSamples =
                        std::numeric_limits<size_t>::max();
                }

                const auto& inputInfo = p->ioInfo.audio;
                const size_t inSampleRate = inputInfo.sampleRate;
                const size_t outSampleRate =
                    thread.info.sampleRate * speedMultiplier;

                auto outputInfo = thread.info;
                outputInfo.sampleRate = outSampleRate;
                // Create the audio resampler.
                if (!thread.resample ||
                    (thread.resample &&
                     thread.resample->getInputInfo() != inputInfo) ||
                    (thread.resample &&
                     thread.resample->getOutputInfo() != outputInfo))
                {
                    thread.resample =
                        audio::AudioResample::create(inputInfo, outputInfo);
                }

                // Fill the audio buffer.
                if (inputInfo.sampleRate <= 0 ||
                    playbackStartTime == time::invalidTime)
                    return 0;

                const bool backwards = playback == Playback::Reverse;
                if (!thread.silence)
                {
                    thread.silence =
                        audio::Audio::create(inputInfo, inSampleRate);
                    thread.silence->zero();
                }

                const int64_t playbackStartFrame =
                    playbackStartTime.rescaled_to(inSampleRate).value() -
                    p->timeline->getTimeRange()
                        .start_time()
                        .rescaled_to(inSampleRate)
                        .value() -
                    otime::RationalTime(audioOffset, 1.0)
                        .rescaled_to(inSampleRate)
                        .value();
                const auto bufferSampleCount =
                    audio::getSampleCount(thread.buffer);
                const auto& timeOffset =
                    otime::RationalTime(
                        thread.rtAudioCurrentFrame + bufferSampleCount,
                        outSampleRate)
                        .rescaled_to(inSampleRate);

                const int64_t frameOffset = timeOffset.value();
                int64_t frame = playbackStartFrame;

                if (backwards)
                {
                    frame -= frameOffset;
                }
                else
                {
                    frame += frameOffset;
                }

                int64_t seconds = inSampleRate > 0 ? (frame / inSampleRate) : 0;
                int64_t inOffsetSamples = frame - seconds * inSampleRate;

                // std::cerr << "TIM seconds:     " << seconds << std::endl;
                // std::cerr << "TIM rtAudioCurrentFrame: " <<
                // thread.rtAudioCurrentFrame << std::endl; std::cerr << "TIM
                // playbackStartTime: " << playbackStartTime << std::endl;
                // std::cerr << "TIM playbackStartFrame: " << playbackStartFrame
                // << std::endl; std::cerr << "TIM bufferSampleCount: " <<
                // bufferSampleCount << std::endl; std::cerr << "TIM
                // inSampleRate: " << inSampleRate << std::endl; std::cerr <<
                // "TIM outSampleRate: " << outSampleRate << std::endl;
                // std::cerr << "TIM timeOffset: " <<
                // timeOffset.rescaled_to(1.0) << std::endl; std::cerr << "TIM
                // frameOffset: " << frameOffset << std::endl; std::cerr << "TIM
                // frame:       " << frame   << std::endl;
                while (audio::getSampleCount(thread.buffer) < outSamples)
                {
                    ;
                    // std::cerr << "\toffset  = " << offset  << std::endl;
                    // std::cout << "\tseconds: " << seconds << std::endl;
                    // std::cerr << "\tinOffsetSamples:      " <<
                    // inOffsetSamples  << std::endl;
                    AudioData audioData;
                    {
                        std::unique_lock<std::mutex> lock(p->audioMutex.mutex);
                        const auto j =
                            p->audioMutex.audioDataCache.find(seconds);
                        if (j != p->audioMutex.audioDataCache.end())
                        {
                            audioData = j->second;
                        }
                    }

                    std::vector<float> volumeScale;
                    volumeScale.reserve(audioData.layers.size());
                    std::vector<std::shared_ptr<audio::Audio> > audios;
                    std::vector<const uint8_t*> audioDataP;
                    const size_t dataByteOffset =
                        inOffsetSamples * inputInfo.getByteCount();
                    const auto sample =
                        seconds * inSampleRate + inOffsetSamples;
                    int audioIndex = 0;
                    for (const auto& layer : audioData.layers)
                    {
                        float volumeMultiplier = 1.F;
                        if (layer.audio && layer.audio->getInfo() == inputInfo)
                        {
                            auto audio = layer.audio;

                            if (layer.inTransition)
                            {
                                const auto& clipTimeRange = layer.clipTimeRange;
                                const auto& range = otime::TimeRange(
                                    clipTimeRange.start_time().rescaled_to(
                                        inSampleRate),
                                    clipTimeRange.duration().rescaled_to(
                                        inSampleRate));
                                const auto inOffset =
                                    layer.inTransition->in_offset().value();
                                const auto outOffset =
                                    layer.inTransition->out_offset().value();
                                const auto fadeInBegin =
                                    range.start_time().value() - inOffset;
                                const auto fadeInEnd =
                                    range.start_time().value() + outOffset;
                                if (sample > fadeInBegin)
                                {
                                    if (sample < fadeInEnd)
                                    {
                                        volumeMultiplier = fadeValue(
                                            sample,
                                            range.start_time().value() -
                                                inOffset - 1.0,
                                            range.start_time().value() +
                                                outOffset);
                                        volumeMultiplier =
                                            std::min(1.F, volumeMultiplier);
                                    }
                                }
                                else
                                {
                                    volumeMultiplier = 0.F;
                                }
                            }

                            if (layer.outTransition)
                            {
                                const auto& clipTimeRange = layer.clipTimeRange;
                                const auto& range = otime::TimeRange(
                                    clipTimeRange.start_time().rescaled_to(
                                        inSampleRate),
                                    clipTimeRange.duration().rescaled_to(
                                        inSampleRate));

                                const auto inOffset =
                                    layer.outTransition->in_offset().value();
                                const auto outOffset =
                                    layer.outTransition->out_offset().value();
                                const auto fadeOutBegin =
                                    range.end_time_inclusive().value() -
                                    inOffset;
                                const auto fadeOutEnd =
                                    range.end_time_inclusive().value() +
                                    outOffset;
                                if (sample > fadeOutBegin)
                                {
                                    if (sample < fadeOutEnd)
                                    {
                                        volumeMultiplier =
                                            1.F - fadeValue(
                                                      sample,
                                                      range.end_time_inclusive()
                                                              .value() -
                                                          inOffset,
                                                      range.end_time_inclusive()
                                                              .value() +
                                                          outOffset + 1.0);
                                    }
                                    else
                                    {
                                        volumeMultiplier = 0.F;
                                    }
                                }
                            }

                            if (audioIndex < channelMute.size() &&
                                channelMute[audioIndex])
                            {
                                volumeMultiplier = 0.F;
                            }

                            if (backwards)
                            {
                                auto tmp = audio::Audio::create(
                                    inputInfo, inSampleRate);
                                tmp->zero();

                                std::memcpy(
                                    tmp->getData(), audio->getData(),
                                    audio->getByteCount());
                                audio = tmp;
                                audios.push_back(audio);
                            }
                            audioDataP.push_back(
                                audio->getData() + dataByteOffset);
                            volumeScale.push_back(volumeMultiplier);
                            ++audioIndex;
                        }
                    }

                    if (audioDataP.empty())
                    {
                        volumeScale.push_back(0.F);
                        audioDataP.push_back(thread.silence->getData());
                    }

                    size_t inSamples = std::min(
                        p->playerOptions.audioBufferFrameCount,
                        static_cast<size_t>(inSampleRate - inOffsetSamples));

                    if (backwards)
                    {
                        if (thread.backwardsSamples < inSamples)
                        {
                            inSamples = thread.backwardsSamples;
                        }

                        audio::reverse(
                            const_cast<uint8_t**>(audioDataP.data()),
                            audioDataP.size(), inSamples,
                            inputInfo.channelCount, inputInfo.dataType);
                    }

                    auto tmp = audio::Audio::create(inputInfo, inSamples);
                    tmp->zero();
                    audio::mix(
                        audioDataP.data(), audioDataP.size(), tmp->getData(),
                        volume, volumeScale, inSamples, inputInfo.channelCount,
                        inputInfo.dataType);

                    if (thread.resample)
                    {
                        thread.buffer.push_back(thread.resample->process(tmp));
                    }

                    if (backwards)
                    {
                        inOffsetSamples -= inSamples;
                        if (inOffsetSamples < 0)
                        {
                            seconds -= 1;
                            if (speedMultiplier < 1.0)
                                inOffsetSamples +=
                                    (inSampleRate * speedMultiplier);
                            else
                                inOffsetSamples += inSampleRate;
                            thread.backwardsSamples = static_cast<size_t>(
                                inSampleRate - inOffsetSamples);
                        }
                        else
                        {
                            thread.backwardsSamples = inSamples;
                        }
                    }
                    else
                    {
                        inOffsetSamples += inSamples;
                        if (inOffsetSamples >= inSampleRate)
                        {
                            inOffsetSamples -= inSampleRate;
                            seconds += 1;
                        }
                    }
                }

                // Send audio data to RtAudio.
                const auto now = std::chrono::steady_clock::now();
                if (defaultSpeed ==
                        p->timeline->getTimeRange().duration().rate() &&
                    !mute && now >= muteTimeout &&
                    outSamples <= getSampleCount(thread.buffer))
                {
                    audio::move(
                        thread.buffer, reinterpret_cast<uint8_t*>(outputBuffer),
                        outSamples);
                }

                // Update the audio frame.
                thread.rtAudioCurrentFrame += outSamples;

#    ifdef CHECK_AUDIO
                std::cerr << "\t\t\tSEND seconds=" << seconds << std::endl
                          << "\t\t\tSEND inOccsetSamples " << inOffsetSamples
                          << " outSamples=" << outSamples << std::endl;
#    endif

                break;
            }
            default:
                break;
            }

            return 0;
        }

        void Player::Private::rtAudioErrorCallback(
            RtAudioError::Type type, const std::string& errorText)
        {
            // std::cout << "RtAudio ERROR: " << errorText << std::endl;
        }
#endif // TLRENDER_AUDIO

    } // namespace timeline
} // namespace tl
